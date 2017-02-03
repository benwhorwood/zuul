// http://tldp.org/HOWTO/Serial-Programming-HOWTO/x115.html
// https://www.cmrr.umn.edu/~strupp/serial.html

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>

#define BAUDRATE B115200
#define MODEMDEVICE "/dev/ttyACM0"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE; 

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("usage: %s filename\n", argv[0]);
		return 1;
	}
	
	char *keyFile = argv[1];
	
	char key[255];
	getKey(key);
	//printf("%s\n", key);

	// Moved to take key filename as argument
	/*char randFile[16];
	randStr(randFile, 16);
	//printf("%s\n", randFile);
	strcat(keyFile, "/tmp/");
	strcat(keyFile, randFile);*/
	
	FILE *f = fopen(keyFile, "w");
	if (f == NULL)
	{
		printf("Error opening %s\n", keyFile);
		exit(1);
	}
	
	fprintf(f, "%s", key);
	fclose(f);
	
	// TODO chmod key file as current user
	
	return 0;
}

void randStr(char *string, size_t length, size_t offset) 
{
	/* Seed number for rand() */
	srand((unsigned int) time(0));  

	/* ASCII characters 33 to 126 */
	/*unsigned int num_chars = length - 1;
	unsigned int i;
	for (i = 0; i < num_chars; ++i)
	{
	  string[i] = rand() % (126 - 33 + 1) + 33;
	}*/

	/* ASCII characters 97 to 122 */
	unsigned int num_chars = length - 1;
	unsigned int i;
	for (i = 0; i < num_chars; ++i)
	{
	  string[i] = rand() % (122 - 97 + 1) + 97;
	}

	string[num_chars] = '\0';  
}

void getKey(char *key)
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
	
	int i = 0;
	while (i<32) {
		key[i] = keyStr[i+4];
		i++;
	}
	key[32] = 0; // So we can printf
	
	//printf("Key: %s\n", key);
	
	// Finished doing stuff
	
	tcsetattr(fd,TCSANOW,&oldtio);
	
	close(fd);
}
