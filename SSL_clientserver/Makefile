
all:
	# COMPILING [Controller] FOR x86
	#gcc -Wall Socket_client.c -o ../bin/client -lm 
	gcc -Wall SSL_client.c -lssl -lcrypto -o ../bin/ssl_client -lm
	#gcc -Wall Socket_server.c -o ./bin/Socket_server -lm 
	
	# COMPILING [Controller] FOR ARM
	#arm-linux-gnueabi-gcc -Wall Socket_server.c -o ../bin/server_arm -lm
	#arm-linux-gnueabi-gcc -Wall ssl_server.c -lssl -lcrypto -o ../bin/ssl_server -lm
	
	# Copy to Beaglebone
	#scp ../bin/server_arm root@192.168.7.2:/home/root
	#scp ../bin/ssl_server root@192.168.7.2:/home/root
	scp SSL_server.c root@192.168.7.2:/home/root