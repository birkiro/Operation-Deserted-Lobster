
all:
	#--- COMPILING [Controller] FOR x86 ---#
	gcc -Wall -DTFM_DESC Tomcrypt_client.c -ltomcrypt -ltfm -o ./bin/SSL_client_x86
	gcc -Wall -DTFM_DESC Tomcrypt_server.c -ltomcrypt -ltfm -o ./bin/SSL_server_x86 
	#--- COMPILING [Controller] FOR ARM ---#
	arm-linux-gnueabi-gcc -DTFM_DESC -Wall Tomcrypt_client.c -ltomcrypt -ltfm -o ./bin/SSL_client_arm
	arm-linux-gnueabi-gcc -DTFM_DESC -Wall Tomcrypt_server.c -ltomcrypt -ltfm -o ./bin/SSL_server_arm  
	#scp ./bin/tomcryptbeaglearm root@192.168.1.33:/home/root
