
all:
	#--- COMPILING [Controller] FOR x86 ---#
	gcc -Wall SSL_client.c -o ./bin/SSL_client_x86 -lm
	gcc -Wall SSL_server.c -o ./bin/SSL_server_x86 -lm 
	#--- COMPILING [Controller] FOR ARM ---#
	arm-linux-gnueabi-gcc -Wall SSL_client.c -o ./bin/SSL_client_arm -lm
	arm-linux-gnueabi-gcc -Wall SSL_server.c -o ./bin/SSL_server_arm -lm  
	#scp ./bin/tomcryptbeaglearm root@192.168.1.33:/home/root

