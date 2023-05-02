#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include <fstream>
#include "socketconfig.h"
#define MAXBUF 8192

class serverSocket : public Socket {
public:
	serverSocket(string port);
	~serverSocket();
	int connect_socket(int sock, struct addrinfo* servinfo);
	int accept_connection();
	string get_time();

	struct request {
		string method, filename, version;
		bool is_static;
		string get_request_line();
	};
private:
	static const int CAUSE_SIZE = 64;
	static const int TIME_SIZE = 128;
	static const int ERRNUM_SIZE = 3;
	static const int SHORTMSG_SIZE = 128;
	static const int FILENAME_SIZE = 128;

	struct addrinfo* p;
	struct sockaddr_storage client_addr;
	socklen_t addr_len;
	int client_sock;
	char buf[MAXBUF];
	char output[MAXBUF];
	request req;

	void parse_request(string request);
	void request_error(int sock, char* cause, char* errnum, char* shortmsg, char* longmsg);
	void handle_request(int sock, request _request);
};

#endif
