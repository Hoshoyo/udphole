all:
	mkdir -p bin
	gcc -g udpserver_p2p.c -o bin/p2pudpserver
	gcc -g udpclient_p2p.c -o bin/p2pudpclient