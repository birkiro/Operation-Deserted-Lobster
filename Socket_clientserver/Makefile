
all:
	#--- COMPILING [Controller] FOR x86 ---#
	#gcc -Wall Socket_client.c -o Socket_client -lm 
	#gcc -Wall Socket_server.c -o ./bin/Socket_server -lm 
	#--- COMPILING [Controller] FOR ARM ---#
	arm-linux-gnueabi-gcc -Wall Socket_server.c -o ../bin/Socket_server_arm -lm 
	#scp ./bin/Socket_server_arm root@192.168.1.33:/home/root
