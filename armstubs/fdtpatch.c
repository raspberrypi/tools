/*-
 * Copyright (c) 2016 Oleksandr Tymoshenko <gonzo@freebsd.org>
 * Copyright (c) 2021 Ivan T. Ivanov <iivanov@suse.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* FDT bits */
#define	FDT_MAGIC       0xd00dfeed      /* 4: version, 4: total size */
#define	FDT_TAGSIZE     sizeof(uint32_t)

#define	FDT_BEGIN_NODE	0x1             /* Start node: full name */
#define	FDT_END_NODE	0x2             /* End node */
#define	FDT_PROP	0x3             /* Property: name off,
                                           size, content */
#define	FDT_NOP		0x4             /* nop */
#define	FDT_END		0x9

/*
 * Data below when applied to main blob produces following device tree node
 *   psci {
 *       compatible      = "arm,psci-1.0";
 *       method          = "smc";
 *   };
 */

#define	PSCI_NODE_LEN			0x3C
#define	PSCI_NODE_COMAPTIBLE_OFF	0x14
#define	PSCI_NODE_METHOD_OFF		0x30
const unsigned char psci_node[] = {
	/* FDT_BEGIN_NODE, "psci", FDT_PROP */
	0, 0, 0, 1, 'p', 's', 'c', 'i', 0, 0, 0, 0, 0, 0, 0, 3,
	/* len, @"compatible", "arm,psci-1.0 */
	0, 0, 0, 13, 0, 0, 0, 0, 'a', 'r', 'm', ',', 'p', 's', 'c', 'i',
	                                /* FDT_PROP, len */
	'-', '1', '.', '0', 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 4,
        /* @"method", "smc", FDT_END_NODE */
        0, 0, 0, 0, 's', 'm', 'c', 0, 0, 0, 0, 2
};

#define	STRINGS_SIZE			0x12
#define	STRINGS_COMPATIBLE_OFF		0
#define	STRINGS_METHOD_OFF		11
const unsigned char strings[] = {
	'c', 'o', 'm', 'p', 'a', 't', 'i', 'b',
	'l', 'e',   0, 'm', 'e', 't', 'h', 'o',
	'd', 0
};

typedef	unsigned int uint32_t;
typedef	unsigned long long uint64_t;

struct fdt_reserve_entry {
	uint64_t address;
	uint64_t size;
};

__attribute__((always_inline))
static inline uint32_t
bswap32(uint32_t v)
{
	uint32_t ret;

	__asm __volatile("rev32 %x0, %x1\n"
                         : "=&r" (ret), "+r" (v));

	return (ret);
}

__attribute__((always_inline))
static inline uint64_t
bswap64(uint64_t v)
{
	uint64_t ret;

	__asm __volatile("rev %x0, %x1\n"
                         : "=&r" (ret), "+r" (v));

	return (ret);
}

/*
 * We always assume that dst is la
 */
static void memmove(const void *src, void *dst, int len)
{
	const char *s;
	char *d;

	s = src;
	d = dst;
	for (int i = len - 1; i >= 0; i--)
		d[i] = s[i];
}

static int is_enable_method(const char *s1)
{
	const char *s2 = "enable-method";
	unsigned n = 13;

	while(n--) {
		if (*s1++ != *s2++)
			return 0;
	}

	return 1;
}

void fixup_dt_blob(void *dtb)
{
	uint32_t magic;
	uint32_t total_size;
	uint32_t off_memrsrv;
	uint32_t off_struct;
	uint32_t off_strings;
	uint32_t size_struct;
	uint32_t size_strings;
	uint32_t *dtb_hdr;
	uint32_t *dtb_struct;
	struct fdt_reserve_entry *reserve;
	int tag_ptr, tag, done, len, level, node_ptr;
	int psci_node_ptr;
	int strings_end;
	char *dtb_strings;
	char *s;

	dtb_hdr = dtb;

	magic = bswap32(dtb_hdr[0]);
	if (magic != FDT_MAGIC) {
		return;
	}
	total_size = bswap32(dtb_hdr[1]);
	off_struct = bswap32(dtb_hdr[2]);
	off_strings = bswap32(dtb_hdr[3]);
	off_memrsrv = bswap32(dtb_hdr[4]);
	size_strings = bswap32(dtb_hdr[8]);
	size_struct = bswap32(dtb_hdr[9]);

	/* Get one more page */
	reserve = (struct fdt_reserve_entry *)((char*)dtb + off_memrsrv);
	reserve->size = bswap64(bswap64(reserve->size) + 0x1000);

	dtb_struct = dtb_hdr + off_struct/sizeof(uint32_t);
	dtb_strings = (char*)dtb + off_strings;

	/*
	 * Find end first non-root node
	 */
	done = 0;
	tag_ptr = 0;
	level = 0;
	node_ptr = -1;
	/* Find first sub-node of root node */
	while (!done) {
		tag = bswap32(dtb_struct[tag_ptr]);
		switch (tag) {
		case FDT_BEGIN_NODE:
			level++;
			if (level == 2) {
				node_ptr = tag_ptr;
				break;
			}
			tag_ptr++;
			s = (char *)(dtb_struct + tag_ptr);
			len = 0;
			while (s[len] != 0)
				len++;
			len++; /* include zero byte */
			len = (len + 3) & ~0x3;
			tag_ptr += len/sizeof(uint32_t);
			break;

		case FDT_END_NODE:
			tag_ptr++;
			break;

		case FDT_NOP:
			tag_ptr++;
			break;
		case FDT_PROP:
			tag_ptr++; /* skip tag */
			len = bswap32(dtb_struct[tag_ptr]);
			/* Goto 'nameoff'  */
			tag_ptr += 1;
			s = (char *)(dtb_strings + bswap32(dtb_struct[tag_ptr]));
			/* Align up to the next 32 bit */
			len = (len + 3) & ~0x3;
			/* Goto property value  */
			tag_ptr += 1;
			if (is_enable_method(s)) {
				s = (char *)&dtb_struct[tag_ptr];
				memmove("psci\0", s, 5);
			}
			tag_ptr += len/sizeof(uint32_t);
			break;
		case FDT_END:
			done = 1;
			break;
		default:
			done = 1;
			break;
		}
	}

	tag_ptr = node_ptr;

	/* Either invalid tag or reached end */
	if (tag_ptr < 0)
		return;

	/*
	 * Insert free space for psci node before
	 * first non-root node
	 */
	psci_node_ptr = off_struct + tag_ptr * sizeof(uint32_t);
	memmove((char *)dtb + psci_node_ptr,
		(char *)dtb + psci_node_ptr + PSCI_NODE_LEN,
		total_size - psci_node_ptr);
	memmove(psci_node, (char *)dtb + psci_node_ptr, PSCI_NODE_LEN);

	/* Fixup lengths/offsets */
	total_size += PSCI_NODE_LEN;
	size_struct += PSCI_NODE_LEN;
	if (off_strings > psci_node_ptr)
		off_strings += PSCI_NODE_LEN;
	if (off_memrsrv > psci_node_ptr)
		off_memrsrv += PSCI_NODE_LEN;

	/*
	 * Append some free space at the end of strings section
	 */
	strings_end = off_strings + size_strings;
	memmove((char *)dtb + strings_end,
		(char *)dtb + strings_end + STRINGS_SIZE,
		total_size - strings_end);
	memmove(strings, (char *)dtb + strings_end, STRINGS_SIZE);

	/* set property names offsets in psci node */
	dtb_struct[tag_ptr + PSCI_NODE_COMAPTIBLE_OFF/sizeof(uint32_t)] =
		bswap32(size_strings + STRINGS_COMPATIBLE_OFF);
	dtb_struct[tag_ptr + PSCI_NODE_METHOD_OFF/sizeof(uint32_t)] =
		bswap32(size_strings + STRINGS_METHOD_OFF);

	/* Fixup lengths/offsets */
	total_size += STRINGS_SIZE;
	size_strings += STRINGS_SIZE;
	if (off_struct > strings_end)
		off_struct += STRINGS_SIZE;
	if (off_memrsrv > psci_node_ptr)
		off_memrsrv += STRINGS_SIZE;

	/* Update header */
	dtb_hdr[1] = bswap32(total_size);
	dtb_hdr[2] = bswap32(off_struct);
	dtb_hdr[3] = bswap32(off_strings);
	dtb_hdr[4] = bswap32(off_memrsrv);
	dtb_hdr[8] = bswap32(size_strings);
	dtb_hdr[9] = bswap32(size_struct);
}
