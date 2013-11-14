
all:
	#--- COMPILING [Controller] FOR x86 ---#
	gcc -Wall Socket_client.c -o ../bin/client -lm 
	#gcc -Wall Socket_server.c -o ./bin/Socket_server -lm 
	#--- COMPILING [Controller] FOR ARM ---#
	arm-linux-gnueabi-gcc -Wall Socket_server.c -o ../bin/server_arm -lm 
	scp ../bin/server_arm root@192.168.7.2:/home/root
