#include <tomcrypt.h>

#define MAXDATASIZE 100 // max number of bytes we can get at once
char buf[MAXDATASIZE];

int err, hash_idx, prng_idx, res;
rsa_key key;
FILE* file;
char private_key_file[200], public_key_file[200];

int key_generator(void) {
	/* make an RSA-1024 key */

	if ((err = rsa_make_key(NULL, /* PRNG state */
	prng_idx, /* PRNG idx */
	1024 / 8, /* 1024-bit key */
	65537, /* we like e=65537 */
	&key) /* where to store the key */
	) != CRYPT_OK) {
		printf("rsa_make_key %s", error_to_string(err));
		return EXIT_FAILURE;
	}

	/* Export keys */
	system("rm private_key");
	system("rm public_key");

	unsigned char out[1024];
	unsigned long outlen;
	outlen = sizeof(out);

	if ((err = rsa_export(out, &outlen, PK_PUBLIC, &key)) != CRYPT_OK) {
		printf("Export error: %s\n", error_to_string(err));
		return -1;
	}

	printf("The public key is: %lu\n", outlen);

	file = fopen("public_key", "a");
	unsigned long i = 0;
	for (i = 0; i < outlen; ++i) {
		printf("%c", out[i]);
		fprintf(file, "%c", out[i]);
	}
	fclose(file);
	printf("\n\n");


	outlen = sizeof(out);
	 if ((err = rsa_export(out, &outlen, PK_PRIVATE, &key)) != CRYPT_OK) {
	 printf("Export error: %s\n", error_to_string(err));
	 return -1;
	 }
	 printf("The private key is: %lu\n", outlen);

	 file = fopen("private_key", "a");
	 	for (i = 0; i < outlen; ++i) {
	 		printf("%c", out[i]);
	 		fprintf(file, "%c", out[i]);
	 	}
	 	fclose(file);
	 	printf("\n\n");

	return 0;
}

int main(int argc, char *argv[]) {

	/* register prng/hash */
	if (register_prng(&sprng_desc) == -1) {
		printf("Error registering sprng");
		return EXIT_FAILURE;
	}
	/* register a math library (in this case TomsFastMath) */
	//ltc_mp = tfm_desc;
	ltc_mp = ltm_desc; // normal math library
	if (register_hash(&sha1_desc) == -1) {
		printf("Error registering sha1");
		return EXIT_FAILURE;
	}

	hash_idx = find_hash("sha1");
	prng_idx = find_prng("sprng");

	while (1) {
		printf(
				"Choose an option:\n 1: Create keys\n 2: Import keys\n 3: Initiate communication\n ");
		bzero(buf, MAXDATASIZE); // Fill buffer with zeros
		fgets(buf, MAXDATASIZE, stdin); // Read from stream
		printf("You choose option: %s\n", buf);

		switch (buf[0]) {
		case '1':
			printf("1: Create keys\n");
			key_generator();
			printf("Keys generated\n");
			break;
		case '2':
			printf("2: Import keys\n");

			break;
		case '3':
			printf("3: Initiate communication\n");

			break;
		default:
			printf("Try again\n");
			break;
		}

	}
	return 0;
}
