#include <dlfcn.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>

#define BAUDRATE B115200
#define MODEMDEVICE "/dev/ttyACM0"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE; 

int aes_init(unsigned char *key_data, int key_data_len, unsigned char *salt, EVP_CIPHER_CTX *e_ctx, EVP_CIPHER_CTX *d_ctx)
{
	int i, nrounds = 5;
	unsigned char key[32], iv[32];
  	i = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha1(), salt, key_data, key_data_len, nrounds, key, iv);
	if (i != 32) return -1;

	EVP_CIPHER_CTX_init(e_ctx);
	EVP_EncryptInit_ex(e_ctx, EVP_aes_256_cbc(), NULL, key, iv);
	EVP_CIPHER_CTX_init(d_ctx);
	EVP_DecryptInit_ex(d_ctx, EVP_aes_256_cbc(), NULL, key, iv);
	
	return 0;
}

unsigned char *aes_decrypt(EVP_CIPHER_CTX *e, unsigned char *ciphertext, int *len)
{
	int p_len = *len, f_len = 0;
	unsigned char *plaintext = malloc(p_len + AES_BLOCK_SIZE);
  
	EVP_DecryptInit_ex(e, NULL, NULL, NULL, NULL);
	EVP_DecryptUpdate(e, plaintext, &p_len, ciphertext, *len);
	EVP_DecryptFinal_ex(e, plaintext+p_len, &f_len);

	*len = p_len + f_len;
	return plaintext;
}


int check_nfc()
{
	long i;
	char printable[32];
	FILE *fp;
	char secrettext[]= "Welcome Back Master! Unlocking.";
	EVP_CIPHER_CTX en,de;
	unsigned int salt[] = {12345, 54321};
	unsigned char ciphertext[48];
	char *plaintext;
	int len;
	
	
	// Initialize serial (Non-Canonical Input Processing)
	int fd, c, res, n;
	struct termios oldtio,newtio;
	char buf[255];

	fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY ); 
	if (fd <0) {perror(MODEMDEVICE); return 1; }

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
	
	
	// Send command to read key
	n = write(fd, "read0\n", 6);
	if (n < 0) return 2;
	
	
	// Read key back
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
	
	
	// Close serial
	tcsetattr(fd,TCSANOW,&oldtio); // Set last attributes on port
	close(fd);
	
	
	// Extract key
	for (i=0;i<32;i++)
		printable[i] = keyStr[i+4];
	printable[32]=0x00;

	if (aes_init(printable, strlen(printable), (unsigned char *)&salt, &en, &de)) return 7;
	
	fp = fopen("/etc/zuul_login_check", "r");
	if (!fp) 
	{
		EVP_CIPHER_CTX_cleanup(&en);
		EVP_CIPHER_CTX_cleanup(&de);
		return 8;
	}
	fread(ciphertext, 1, 48, fp);
	fclose(fp);
	len =48;
	plaintext = (char *)aes_decrypt(&de, ciphertext, &len);

	if (strcmp(plaintext, secrettext) == 0)
		return 0;
	else
	{
		
		free(plaintext);
		EVP_CIPHER_CTX_cleanup(&en);
		EVP_CIPHER_CTX_cleanup(&de);
		return 9;
	}
  

}


PAM_EXTERN int pam_sm_setcred( pam_handle_t *pamh, int flags, int argc, const char **argv ) {
	return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv) {
	return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_authenticate( pam_handle_t *pamh, int flags,int argc, const char **argv ) {
	
	int retval = check_nfc();
	if (retval != 0)
	{
		printf("\nNFC Auth Err: %d", retval);
		return PAM_AUTH_ERR;
	}
	else
		return PAM_SUCCESS;
}
