#include <dlfcn.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/evp.h>
#include <openssl/aes.h>

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

unsigned char *aes_encrypt(EVP_CIPHER_CTX *e, unsigned char *plaintext, int *len)
{
	int c_len = *len + AES_BLOCK_SIZE, f_len = 0;
	unsigned char *ciphertext = malloc(c_len);

	EVP_EncryptInit_ex(e, NULL, NULL, NULL, NULL);

	EVP_EncryptUpdate(e, ciphertext, &c_len, plaintext, *len);

	EVP_EncryptFinal_ex(e, ciphertext+c_len, &f_len);
	*len = c_len + f_len;
	return ciphertext;
}

int main(int argc, char **argv)
{
	char* error;
	EVP_CIPHER_CTX en, de;
	unsigned int salt[] = {12345, 54321};
	unsigned char *key_data;
	FILE *fp;
	int key_data_len, i;
	char *input[] = {"Welcome Back Master! Unlocking.",NULL};
	char *plaintext;
	unsigned char *ciphertext;
	int olen, len;

	/*if (argc != 2) 
	{
		printf("Please enter a 32-byte key string as parameter\n");
		return -1;
	}
	if (strlen(argv[1]) != 32)
	{
		printf("Please enter a 32-byte key string as parameter\n");
		return -1;
	}
	
	key_data = (unsigned char *)argv[1];
	key_data_len = strlen(argv[1]);*/
	key_data = "1-_-whatabeautifuldayinapril-_-1";
	key_data_len = 32;
  
	if (aes_init(key_data, key_data_len, (unsigned char *)&salt, &en, &de)) 
	{
		printf("Couldn't initialize AES cipher\n");
		return -1;
	}

	olen = len = strlen(input[0])+1;
   	ciphertext = aes_encrypt(&en, (unsigned char *)input[0], &len);

    	fp = fopen("./login_check_file", "w");
	if (!fp)
	{
		printf("Failed to open login_check_file file. Do you have the necessary perms?\n");
		return -1;
	}

	fwrite(ciphertext, 1, 48, fp);
	fclose(fp);
	
	printf("login_check_file file has been created successfully\n");
	
	free(ciphertext);

	EVP_CIPHER_CTX_cleanup(&en);
	EVP_CIPHER_CTX_cleanup(&de);
	return 0;
}
  
