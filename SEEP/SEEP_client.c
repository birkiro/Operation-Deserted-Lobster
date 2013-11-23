#include <tomcrypt.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>

#define FAIL    -1
#define MAXDATASIZE 	100 	// max number of bytes we can get at once
#define PLAINTEXT_SIZE	30		// lenght of pt
#define CIPHER_SIZE		1020	// lenght of cipher
#define SOCKETBUFFERSIZE 1024

#define REQ_FOR_SESSION 0xaa


unsigned char buf[MAXDATASIZE];

int err, hash_idx, prng_idx, res;
rsa_key private_key, public_key;
FILE* file;
unsigned char private_key_file[200], public_key_file[200];
unsigned char pt_in[PLAINTEXT_SIZE], pt_out[PLAINTEXT_SIZE]; //for plain text both ways
unsigned long pt_lenght = PLAINTEXT_SIZE;
unsigned char cipher_in[CIPHER_SIZE], cipher_out[CIPHER_SIZE]; //for encrypted msg's both ways
unsigned long cipher_lenght = CIPHER_SIZE;

unsigned long nonceA;

int OpenClientSocket(const char *hostname, int port)
{   int my_socket;
    struct hostent *host;
    struct sockaddr_in addr;

    if ( (host = gethostbyname(hostname)) == NULL )
    {
    	perror("hostname");
    	exit(1);
    }
    // socket()
    if((my_socket = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
    	perror("socket");
    	exit(1);
    }
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = *(long*)(host->h_addr);

    // connect()
    if ( connect(my_socket, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
    {
        close(my_socket);
        perror(hostname);
        abort();
    }
    return my_socket;
}



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

			NULL, /* our lparam for this program */

			0, /* lparam is 7 bytes long */

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

			NULL, /* lparam for this program */

			0, /* lparam is 7 bytes long */

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
	int server;
	long sockfd, numbytes;
	unsigned char socketbuf[SOCKETBUFFERSIZE];
	int bytes;
	char hostname[]="127.0.0.1";
	char portnum[]="5000";
	unsigned long i = 0;

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
			server = OpenClientSocket(hostname, atoi(portnum));
			printf("Socket connection to server established\n");

			nonceA = random();											//generating random number nonceA

			sprintf((char*)pt_out, "%lu",nonceA);	//generate string and encrypt
			pt_lenght=sizeof(pt_out);
			cipher_lenght=sizeof(cipher_out);
			encrypt_msg();
			printf("NonceA encrypted, was: %s\n", pt_out);
			printf("Sending message to server\n");
			//bzero(socketbuf, MAXDATASIZE);							// Fill buffer with zeros
			//sprintf((char*)socketbuf, "%x,%s",REQ_FOR_SESSION,cipher_out);

			socketbuf[0]=REQ_FOR_SESSION;			//REQ_FOR_SESSION as start byte
			for(i=0;i < CIPHER_SIZE;i++)
			{
				socketbuf[i+1]=cipher_out[i];
			}

			/*for(i=0;i < CIPHER_SIZE;i++)
			{
				printf("%x",cipher_out[i]);
			}*/

			//strcpy(socketbuf, cipher_out);
			//fgets(socketbuf, MAXDATASIZE, stdin);					// Read from stream
			numbytes = write(server, socketbuf, strlen(socketbuf)); // send buffer content through socket
			if(numbytes < 0) { perror("Error in write()"); exit(1);}
			printf("Message send: %s\n\n",socketbuf);
			printf("Message length: %ld \n",numbytes);

			printf("\n\n\n\n\n");
					for(i=0;i < numbytes;i++)
					{
						printf("%x",cipher_out[i]);
					}
					printf("\n\n\n\n\n");

////Do rest here !!! 7A: receive message and decrypt wih Aprivate and store K.
			for(i=0;i < numbytes;i++)
			{
									cipher_in[i]=socketbuf[i+1];
			}


			for(i=0;i < CIPHER_SIZE;i++)
						{
							printf("%x",cipher_in[i]);
						}

			printf("\n\n\n\n\n");
			cipher_lenght=(numbytes-1);
			pt_lenght=sizeof(pt_in);
			decrypt_msg();
			printf("pt_out = %s\n", pt_out);
			printf("pt_in = %s\n", pt_in);

			unsigned long generated_random_number2=0;

			sscanf((char*)pt_in, "%lu",&generated_random_number2 );
			//memcpy(generated_random_number2, &pt_out, 10);
			if(nonceA==generated_random_number2){
				printf("keys are correct !!!\n");
			}
			else{
				printf("keys not correct !!!\n");
			}



			printf("cipher length: %d\n",cipher_lenght);

			break;
		default:
			printf("Try again\n");
			break;
		}

	}
	return 0;
}


