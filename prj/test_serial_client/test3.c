// http://tldp.org/HOWTO/Serial-Programming-HOWTO/x115.html
// https://www.cmrr.umn.edu/~strupp/serial.html

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#define BAUDRATE B115200
#define MODEMDEVICE "/dev/ttyACM0"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE; 

main()
{
	int fd, c, res, n;
	struct termios oldtio,newtio;
	char buf[255];

	fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY ); 
	if (fd <0) {perror(MODEMDEVICE); exit(-1); }

	tcgetattr(fd,&oldtio); /* save current port settings */

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;

	newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
	newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd,TCSANOW,&newtio);
	
	// Do stuff...
	
	// Send command to read key
	n = write(fd, "read0\n", 6);
	if (n < 0)
		fputs("write() of 6 bytes failed!\n", stderr);
	
	// Read back key
	char keyStr[255];
	int  keyStrIdx = 0;
	
	while (STOP==FALSE) {
		/*res = read(fd, buf, 255);
		buf[res]=0; // So we can printf
		printf("> %s | %d\n", buf, res);
		if (buf[0]=='z') STOP=TRUE;*/
		
		res = read(fd, buf, 1);
		if (buf[0] == '\n') {
			STOP=TRUE;
			continue;
		}
		
		keyStr[keyStrIdx] = buf[0];
		keyStrIdx += res;
	}
	
	printf("Returned key string: %s\n", keyStr);
	
	char key[32];
	int i = 0;
	while (i<32) {
		key[i] = keyStr[i+4];
		i++;
	}
	key[32] = 0; // So we can printf
	
	printf("Key: %s\n", key);
	
	
	// Finished doing stuff
	
	tcsetattr(fd,TCSANOW,&oldtio);
	
	close(fd);
}
