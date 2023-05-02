#ifndef SOCKETCONFIG_H
#define SOCKETCONFIG_H

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <string>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAXBUF 8192
using namespace std;

class Socket {
public:
	Socket(string port);
	virtual int connect_socket(int sock, struct addrinfo* servinfo) = 0;
	void read_data(int sock, char* buffer, int buf_len, char* output_str);
	void write_data(int sock, char* buffer, int buf_len);
	void set_sock(int new_sock);
	int get_sock();
	struct addrinfo* get_servinfo();
	char* convert_to_char_arr(char* output, string input);
private:
	int sock, rv, connection;
	struct addrinfo hints, *servinfo;
	
};

#endif
