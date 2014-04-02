/* Copyright (C) 2005, 2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _FEATURES_H
# error "Never use <bits/predefs.h> directly; include <features.h> instead."
#endif

#ifndef _PREDEFS_H
#define _PREDEFS_H

/* We do support the IEC 559 math functionality, real and complex, but only
   if a VFP coprocessor is present. If we don't have one, we fall back to
   software emulation and the functions won't work properly. So in general,
   we don't claim to support this functionality.  */
#if defined (__VFP_FP__) && !defined(__SOFTFP__)
#define __STDC_IEC_559__		1
#define __STDC_IEC_559_COMPLEX__	1
#endif

#endif /* predefs.h */
