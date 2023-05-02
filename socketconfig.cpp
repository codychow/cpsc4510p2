#include "socketconfig.h"

Socket::Socket(string port) {
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
    	hints.ai_flags = AI_PASSIVE;


	rv = getaddrinfo(NULL, port.c_str(), &hints, &servinfo);
    	if (rv != 0) {
        	cerr << "error with getaddrinfo\n";
        	exit(-1);
    	}
}

void Socket::read_data(int sock, char* buffer, int buf_len, char* output_str) {
    memset(buffer, 0, MAXBUF);
    int bytes_recv;
    string output, temp;
    int lines_read = 0;

    while (bytes_recv = recv(sock, buffer, buf_len, 0) > 0) {
        temp = buffer;
        output += temp;
        if (lines_read > 1) {
            break;
        }

        lines_read++;
        memset(buffer, 0, MAXBUF);
    }
    if (bytes_recv == -1) {
        cerr << "Recv() error\n";
        close(sock);
    }
    convert_to_char_arr(output_str, output);
    
}

void Socket::write_data(int sock, char* buffer, int buf_len) {
    int bytes_sent;

    while (buf_len > 0) { 
        bytes_sent = send(sock, buffer, buf_len, 0);
        buf_len -= bytes_sent;
        buffer += bytes_sent;
    }
    if (bytes_sent == -1) {
        cerr << "Recv() error\n";
        close(sock);
    }
}

void Socket::set_sock(int new_sock) {
    sock = new_sock;
}

int Socket::get_sock() {
    return sock;
}

struct addrinfo* Socket::get_servinfo() {
    return servinfo;
}

char* Socket::convert_to_char_arr(char* output, string input) {
    strcpy(output, input.c_str());
    return output;
}
