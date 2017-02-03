
#include <dlfcn.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>

typedef unsigned char BYTE;
typedef BYTE *        PBYTE;

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

int main(int argc, char **argv)
{
	char printable[32] = "1-_-whatabeautifuldayinapril-_-1";
	printable[32] = 0x00;
	FILE *fp;
	char secrettext[]= "Welcome Back Master! Unlocking.";
	EVP_CIPHER_CTX en,de;
	unsigned int salt[] = {12345, 54321};
	unsigned char ciphertext[48];
	char *plaintext;
	int len;
	
	if (aes_init(printable, strlen(printable), (unsigned char *)&salt, &en, &de))
	{
		printf("Couldn't initialize AES cipher\n");
		return -1;
	}
	
	fp = fopen("./login_check_file", "r");
	if (!fp) 
	{
		printf("Failed to open login_check_file file. Does it exist?\n");
		EVP_CIPHER_CTX_cleanup(&en);
		EVP_CIPHER_CTX_cleanup(&de);
		return -1;
	}
	fread(ciphertext, 1, 48, fp);
	fclose(fp);
	len =48;
	plaintext = (char *)aes_decrypt(&de, ciphertext, &len);

	if (strcmp(plaintext, secrettext) == 0)
	{
		printf("Strings do match!\n");
	}
	else
	{
		printf("Strings don't match!\n");
	}
	
	printf("%s | %s\n", plaintext, secrettext);
	
	free(plaintext);
	EVP_CIPHER_CTX_cleanup(&en);
	EVP_CIPHER_CTX_cleanup(&de);
	
	return 0;
}
