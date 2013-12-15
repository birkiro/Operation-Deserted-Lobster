#include <tomcrypt.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <resolv.h>
#include <netdb.h>

#define FAIL    -1
#define MAXDATASIZE 	100 	// max number of bytes we can get at once
#define PLAINTEXT_SIZE	30		// lenght of pt
#define CIPHER_SIZE		1020	// lenght of cipher
#define SOCKETBUFFERSIZE 1024

#define REQ_FOR_SESSION 0xaa
#define NEW_SESSION_KEY 0xdd
#define ACK_NEW_SESSION_KEY	0xee


unsigned char buf[MAXDATASIZE];

int err, hash_idx, prng_idx, res;
rsa_key private_key, public_key;
FILE* file;
unsigned char private_key_file[200], public_key_file[200];
unsigned char pt_in[PLAINTEXT_SIZE], pt_out[PLAINTEXT_SIZE]; //for plain text both ways
unsigned long pt_lenght = PLAINTEXT_SIZE;
unsigned char cipher_in[CIPHER_SIZE], cipher_out[CIPHER_SIZE]; //for encrypted msg's both ways
unsigned long cipher_lenght = CIPHER_SIZE;

unsigned long nonceA, key_copy;
unsigned char key[16], IV[16], buffer[512];
symmetric_CTR ctr;

int SetupAESCrypt(void) {
	/* register aes_desc first */
	if (register_cipher(&aes_desc) == -1) {
		printf("Error registering cipher.\n");
		return -1;
	}

	/* start up CTR mode */
	if ((err = ctr_start(find_cipher("aes"), /* index of desired cipher */
							IV, /* the initial vector */
							key, /* the secret key */
							16, /* length of secret key (16 bytes) */
							0, /* 0 == default # of rounds */
							CTR_COUNTER_LITTLE_ENDIAN, /* Little endian counter */
							&ctr) /* where to store the CTR state */
	) != CRYPT_OK) {
		printf("ctr_start error: %s\n", error_to_string(err));
		return -1;
	}

	return 0;
}

int AESEncrypt(void) {
	/* encrypt buffer */

	if ((err = ctr_encrypt(buffer, /* plaintext */
							buffer, /* ciphertext */
							sizeof(buffer), /* length of plaintext pt */
							&ctr) /* CTR state */
	) != CRYPT_OK) {
		printf("ctr_encrypt error: %s\n", error_to_string(err));
		return -1;
	}

	return 0;
}

int AESDecrypt(void) {
	/* now we want to decrypt so let’s use ctr_setiv */
	if ((err = ctr_setiv(IV, /* the initial IV we gave to ctr_start */
							16, /* the IV is 16 bytes long */
							&ctr) /* the ctr state we wish to modify */
	) != CRYPT_OK) {
		printf("ctr_setiv error: %s\n", error_to_string(err));
		return -1;
	}
	if ((err = ctr_decrypt(buffer,/* ciphertext */
							buffer,/* plaintext */
							sizeof(buffer),/* length of plaintext */
							&ctr) /* CTR state */

	) != CRYPT_OK) {
		printf("ctr_decrypt error: %s\n", error_to_string(err));
		return -1;
	}
	/* terminate the stream */
	if ((err = ctr_done(&ctr)) != CRYPT_OK) {
		printf("ctr_done error: %s\n", error_to_string(err));
		return -1;
	}

	return 0;
}

int OpenServerSocket(int port)
{   int my_socket;
    struct sockaddr_in addr;

    my_socket = socket(PF_INET, SOCK_STREAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if ( bind(my_socket, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
    {
        perror("can't bind port");
        abort();
    }
    if ( listen(my_socket, 10) != 0 )
    {
        perror("Can't configure listening port");
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
	long numbytes;
	unsigned char socketbuf[SOCKETBUFFERSIZE];
	unsigned char payload[1024];

	char portnum[]="5000";
	struct sockaddr_in their_addr;
	int len = sizeof(struct sockaddr_in);
	unsigned long i = 0;
	unsigned char header=0;

	/* fill out IV */
	strcpy((char*) IV, "0123456789012345");		//not that safe ;)


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
			server = OpenServerSocket(atoi(portnum));    /* create server socket */
			printf("Socket connection on server Open\n");

			printf("Waits on client connection\n");
			struct sockaddr_in addr;
			        socklen_t len = sizeof(addr);
			while(1)
			{
				/* Client trying to get connected */
				int client = accept(server, (struct sockaddr*)&addr, &len);
				/* Client connected */
				printf("Connection: %s:%d\n",inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

				// The socket read() function
				bzero(socketbuf, SOCKETBUFFERSIZE); // Fill buffer with zeros
				if ((numbytes = recv(client, socketbuf, SOCKETBUFFERSIZE, 0)) == -1)
				{
					perror("Failed to receive");
					exit(1);
				}
				buf[numbytes] = '\0';  // NULL

				/* Analyze received data */
				bzero(payload, SOCKETBUFFERSIZE); // Fill buffer with zeros
				header=socketbuf[0];

				for(i=0;i < numbytes;i++)
				{
					payload[i]=socketbuf[i+1];
				}

				if(header == REQ_FOR_SESSION){
					printf("REQ_FOR_SESSION\n");
				}
				else
				{
					printf("Not REQ_FOR_SESSION %x\n",header);
					break; //exit
				}

				bzero(cipher_in, CIPHER_SIZE); // Fill buffer with zeros

				for(i=0;i < CIPHER_SIZE;i++)
				{
					cipher_in[i]=payload[i];
				}
				cipher_lenght=(numbytes-1);
				pt_lenght=sizeof(pt_in);
				/* Decrypt payload using RSA */
				decrypt_msg();

				/* Isolate nonceA test number */
				sscanf((char*)pt_in, "%lu",&nonceA );

				/* generate new key for AES crypt */
				key_copy = random();	//twice as it else is the same ?
				key_copy = random();	//generating random number key

				bzero(cipher_out, CIPHER_SIZE); 	// Fill buffer with zeros
				bzero(pt_out, PLAINTEXT_SIZE); 		// Fill buffer with zeros
				bzero(cipher_in, CIPHER_SIZE); 		// Fill buffer with zeros
				bzero(pt_in, PLAINTEXT_SIZE); 		// Fill buffer with zeros
				bzero(socketbuf, SOCKETBUFFERSIZE);	// Fill buffer with zeros
				/* generate string and encrypt using RSA */
				pt_lenght=sprintf((char*)pt_out, "%lu, %lu",nonceA+1, key_copy);
				cipher_lenght=CIPHER_SIZE;
				encrypt_msg();

				/* Send package to client */
				socketbuf[0]=NEW_SESSION_KEY;			//NEW_SESSION_KEY as start byte
				printf("Message NEW_SESSION_KEY\n");
				for(i=0;i < CIPHER_SIZE;i++)
				{
					socketbuf[i+1]=cipher_out[i];
				}
				printf("Message writing to client\n");
				numbytes = write(client, socketbuf, cipher_lenght+1); // send buffer content through socket
				if(numbytes < 0) { perror("Error in write()"); exit(1);}

				/* copy AES key to key variable. */
				sprintf((char*) key, "%lu", key_copy);	//using key

				/* Assuming server passed RSA handshake, its time to set up AES now */
				SetupAESCrypt();

				bzero(buffer, 512); 				// Fill buffer with zeros
				bzero(socketbuf, SOCKETBUFFERSIZE);	// Fill buffer with zeros
				/* Receive new message */
				if ((numbytes = recv(client, socketbuf, SOCKETBUFFERSIZE, 0)) == -1)
				{
					perror("Failed to receive");
					exit(1);
				}
				bzero(payload, SOCKETBUFFERSIZE); // Fill buffer with zeros
				header=socketbuf[0];
				for(i=0;i < numbytes;i++)
				{
					payload[i]=socketbuf[i+1];
				}
				/* Test header */
				if(header == ACK_NEW_SESSION_KEY){
					printf("ACK_NEW_SESSION_KEY\n");
				}
				else
				{
					printf("Not ACK_NEW_SESSION_KEY %x\n",header);
					break;
				}

				memcpy(buffer, payload,numbytes-1);
				/* Decrypt using AES */
				AESDecrypt();
				unsigned long temp_nonceA;
				sscanf((char*)buffer, "%lu",&temp_nonceA);

				if (nonceA + 1 == temp_nonceA) {
					printf("client passed handshake test! nonceA==nonceA_server\n");
				} else {
					printf("client didn't pass handshake test! nonceA!=nonceA_server!!!!\n");
					break;
				}
				/* At this point the SEEP handshake and key sharing is over,
				 * 	from now on communicating can continue with AES encryption */
				close(client);
			}

			break;
		default:
			printf("Try again\n");
			break;
		}

	}
	return 0;
}


