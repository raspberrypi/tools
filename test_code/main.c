#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_SEARCH 100
#define SAVED_SIZE 4096

// Block waiting for input from fd
void block(int fd)
{
	fd_set fds;
	struct timeval tv;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	tv.tv_sec = 100;

	select(fd+1, &fds, NULL, NULL, &tv);
}

// Receive characters from fd waiting for search_str, return string
// containing total output until that point.  The returned string
// is statically allocated so does not need to be free'd
//
char * wait(int fd, char * search_str)
{
	char buf[MAX_SEARCH * 2];
	int bytes, offs = 0;
	int i, found = 0;
	static char *saved = NULL;
	static long saved_size;
	int buf_pos = 0;

	if(saved == NULL)
	{
		saved = malloc(SAVED_SIZE);
		saved_size = SAVED_SIZE;
	}

	do
	{
		do
		{
			bytes = read(fd, buf + offs, MAX_SEARCH * 2 - offs);
			if(bytes == -1)
				block(fd);
		}
		while(bytes == -1);

		for(i = 0; i < bytes; i++)
		{
			fputc(*(buf+offs+i), stdout);
			if(buf_pos == saved_size)
			{
				// If we run out of space in our saved buffer then double
				// the buffer size, simple solution
				saved = realloc(saved, saved_size * 2);
				saved_size *= 2;
			}
			// get rid of \r\n to match the written data
			if(*(buf+offs+i) != '\r')
				saved[buf_pos++] = *(buf+offs+i);
		}
		saved[buf_pos] = '\0';

		*(buf + offs + bytes) = '\0';
		// Read in up to 2 x max search size, search for the string if it's not
		// there then copy down the top half to the bottom.  This handles the case
		// where you have the search string crossing the buffer length...
		if(strstr(buf, search_str))
			found = 1;
		offs += bytes;
		if(offs > MAX_SEARCH)
		{
			memcpy(buf, buf + offs - MAX_SEARCH, MAX_SEARCH);
			offs=0;
		}
	}
	while(!found);

	return saved;
}

int writes(int fd, char *str)
{
	return write(fd, str, strlen(str));
}

// Switch into standard serial mode
// stdin -> serial out 
// serial in -> stdout
void serial_mode(int fd)
{
	int bytes;
	char buf[256];
	
	while(1)
	{
		fd_set fds;

		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		FD_SET(fileno(stdin), &fds);

		// Wait for input from either serial or stdin
		if(select(fd+1, &fds, NULL, NULL, NULL) < 0)
		{
			return;
		}
		
		if(FD_ISSET(fd, &fds))
		{
			bytes = read(fd, buf, sizeof(buf));
			if(bytes > 0)
				write(fileno(stdout), buf, bytes);
		}
		
		if(FD_ISSET(fileno(stdin), &fds))
		{
			bytes = read(fileno(stdin), buf, sizeof(buf));
			if(bytes > 0)
				write(fd, buf, bytes);
		}
	}
	
}

// The lsusb test just checks for the correct number of devices, for
// model A it should just be the root hub (since the host is plugged)
// into it...
int lsusb_test(char *str)
{
	int device_count = 0;
	char *p;
	
	for(p = str; (p = strstr(p, "Device")) != NULL; p++)
		device_count++;
		
	if(device_count == 1)
		return 0;
		
	return -1;
}

typedef int (* test_check) (char *reponse);

struct test_set_s {
	char *instr;
	char *response;
	test_check check_result;
} test_set[] = 
	{
			{ "lsusb", NULL, lsusb_test },
	};

int main(int argc, char * argv[])
{
	int fd;
	char *str;
	int i;
	int device = 0;

	fd = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY | O_NONBLOCK);
	if(fd == -1)
	{
		printf("Failed to open /dev/ttyAMA0\n");
		exit(-1);
	}

	// Wait until there is something to read from the serial port
	block(fd);

	// Wait for the string login: skipping everything else
	wait(fd, "login:");

	// Output the string root with a carriage return
	writes(fd, "root\n");

	wait(fd, "Password:");

	writes(fd, "raspberry\n");

	wait(fd, "$");

	for(i = 0; i < sizeof(test_set) / sizeof(*test_set); i++)
	{
		int result;
		
		// Send the test command to the UUT
		writes(fd, test_set[i].instr);
		
		// Wait for the $ (meaning the end of the command) and return all the
		// string output, this can be huge if needs be
		str = wait(fd, "$") + strlen(test_set[i].instr);
		if(test_set[i].response)
			if(strcmp(test_set[i].response, str))
				result = 0;
			else
				result = 1;
		else
			result = test_set[i].check_result(str);
		
		printf("TEST_RESULT: %s\n", result ? "FAILED" : "PASSED");
	}

	serial_mode(fd);
}
