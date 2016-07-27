all:
	gcc -Wall MyServer1.c -o server1 -lpthread -lrt
	gcc -Wall MyServer2.c -o server2 -lpthread -lrt
	gcc -Wall MyClient1.c -o client1 -lpthread -lrt
	gcc -Wall MyClient2.c -o client2 -lpthread -lrt
	chmod +x run_tests.sh
clean:
	rm client1
	rm server1
	rm client2
	rm server2
