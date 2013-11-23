all:
	#--- COMPILING [Controller] FOR x86 ---#
	gcc -Wall -DLTM_DESC SEEP_client.c -ltommath -ltomcrypt -o ./bin/SEEP_client_x86 -lm
	gcc -Wall -DLTM_DESC SEEP_server.c -ltommath -ltomcrypt -o ./bin/testserverloc/SEEP_server_x86 -lm
	#--- COMPILING [Controller] FOR ARM ---#
	#arm-linux-gnueabi-gcc -DTFM_DESC -Wall SEEP_client.c -ltomcrypt -ltfm -o ./bin/SEEP_client_arm
	#arm-linux-gnueabi-gcc -DTFM_DESC -Wall SEEP_server.c -ltomcrypt -ltfm -o ./bin/SEEP_server_arm
