#include <tomcrypt.h>

#define MAXDATASIZE 	100 	// max number of bytes we can get at once
#define PLAINTEXT_SIZE	50		// lenght of pt
#define CIPHER_SIZE		1024	// lenght of cipher
char buf[MAXDATASIZE];

int err, hash_idx, prng_idx, res;
rsa_key private_key, public_key;
FILE* file;
char private_key_file[200], public_key_file[200];
unsigned char pt_in[PLAINTEXT_SIZE], pt_out[PLAINTEXT_SIZE]; //for plain text both ways
unsigned long pt_lenght = PLAINTEXT_SIZE;
unsigned char cipher_in[CIPHER_SIZE], cipher_out[CIPHER_SIZE]; //for encrypted msg's both ways
unsigned long cipher_lenght = CIPHER_SIZE;

unsigned long int generated_random_number;


int key_generator(void) {
	/* make an RSA-1024 key */
	rsa_key key;
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

int key_import() {
	/* read keys
	 *	This function first, imports the raw keys from binary files
	 *	Then converts them to respectably private_key and public_key
	 *	Where Private are used to decrypt received msg, and public to encrypt msg to server
	 */

	file = fopen("public_key", "r");

	fseek(file, 0, SEEK_END); // seek to end of file
	int key_size = ftell(file); // get current file pointer
	fseek(file, 0, SEEK_SET); // seek back to beginning of file
							  // proceed with allocating memory and reading the file

	unsigned char import_key_array[key_size];

	unsigned long i = 0;
	for (i = 0; i < key_size; ++i) {
		fscanf(file, "%c", &import_key_array[i]);
//		printf("%c", import_key_array[i]);
	}
	fclose(file);
	printf("\n\n");

	file = fopen("private_key", "r");

	fseek(file, 0, SEEK_END); // seek to end of file
	key_size = ftell(file); // get current file pointer
	fseek(file, 0, SEEK_SET); // seek back to beginning of file
							  // proceed with allocating memory and reading the file

	unsigned char import_key_array2[key_size];

	for (i = 0; i < key_size; ++i) {
		fscanf(file, "%c", &import_key_array2[i]);
//		printf("%c", import_key_array2[i]);
	}
	fclose(file);
	printf("\n\n");

	unsigned long inlen = sizeof(import_key_array2);

	if ((err = rsa_import(import_key_array2, inlen, &private_key))
			!= CRYPT_OK) {
		printf("Export error: %s\n", error_to_string(err));
		return -1;
	}
	inlen = sizeof(import_key_array);

	if ((err = rsa_import(import_key_array, inlen, &public_key)) != CRYPT_OK) {
		printf("Export error: %s\n", error_to_string(err));
		return -1;
	}

	printf("Keys imported.\n\n");
	return 0;
}

int encrypt_msg() {

	//cipher_lenght = sizeof(cipher_out);

	if ((err = rsa_encrypt_key(pt_out, /* data we wish to encrypt */

			pt_lenght, /* data is 16 bytes long */

			cipher_out, /* where to store ciphertext */

			&cipher_lenght, /* length of ciphertext */

			"TestApp", /* our lparam for this program */

			7, /* lparam is 7 bytes long */

			NULL, /* PRNG state */

			prng_idx, /* prng idx */

			hash_idx, /* hash idx */

			&public_key) /* our RSA key */

	) != CRYPT_OK) {

		printf("rsa_encrypt_key %s", error_to_string(err));

		return EXIT_FAILURE;

	}
	return 0;
}

int decrypt_msg() {
	//pt_lenght = sizeof(pt2);

	if ((err = rsa_decrypt_key(cipher_in, /* encrypted data */

			cipher_lenght, /* length of ciphertext */

			pt_in, /* where to put plaintext */

			&pt_lenght, /* plaintext length */

			"TestApp", /* lparam for this program */

			7, /* lparam is 7 bytes long */

			hash_idx, /* hash idx */

			&res, /* validity of data */

			&private_key) /* our RSA key */

	) != CRYPT_OK) {

		printf("rsa_decrypt_key %s", error_to_string(err));

		return EXIT_FAILURE;

	}
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
			key_import();
			break;
		case '3':
			printf("3: Initiate communication\n");
			generated_random_number = random();
			sprintf((char*)pt_out, "%lu",generated_random_number);
			//memcpy(pt_out, (char*)generated_random_number, 10);
			//strcpy((char*) pt_out, "test input string here!");
			encrypt_msg();
			unsigned long i = 0;
			for (; i < CIPHER_SIZE; i++) {
				cipher_in[i] = cipher_out[i];
			}
			decrypt_msg();
			printf("pt_out = %s\n", pt_out);
			printf("pt_in = %s\n", pt_in);

			unsigned long generated_random_number2=0;

			sscanf((char*)pt_in, "%lu",&generated_random_number2 );
			//memcpy(generated_random_number2, &pt_out, 10);
			if(generated_random_number==generated_random_number2){
				printf("keys are correct !!!\n");
			}
			else{
				printf("keys not correct !!!\n");
			}

			break;
		default:
			printf("Try again\n");
			break;
		}

	}
	return 0;
}
