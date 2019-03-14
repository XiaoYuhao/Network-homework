server_all: OnlineClient.cpp tools.cpp Database.cpp Server.cpp main.cpp md5.cpp
	g++ -c OnlineClient.cpp -std=gnu++11 -I/usr/include/mysql -o OnlineClient.o
	g++ -c tools.cpp -std=gnu++11 -I/usr/include/mysql -o tools.o
	g++ -c Server.cpp -std=gnu++11 -I/usr/include/mysql -o Server.o
	g++ -c Database.cpp -std=gnu++11 -I/usr/include/mysql -o Database.o
	g++ -c md5.cpp -std=gnu++11 -I/usr/include/mysql -o md5.o
	g++ -c main.cpp -std=gnu++11 -I/usr/include/mysql -o main.o
	g++ -o server -std=gnu++11 -I/usr/include/mysql -L/usr/lib64/mysql -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -ldl -lssl -lcrypto Database.o tools.o OnlineClient.o Server.o md5.o main.o

clean:
	rm -rf *.o server *.log