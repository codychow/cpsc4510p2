#include "serversocket.h"

serverSocket::serverSocket(string port) : Socket(port) {
    if (connect_socket(get_sock(), get_servinfo()) != 0) {
        cerr << "Could not connect socket\n";
        exit(-1);
    }
    cout << "Socket binded...\n";
    freeaddrinfo(get_servinfo());

    if (listen(get_sock(), 1) != 0) {
        cerr << "Listen() error\n";
        exit(-1);
    }

    cout << "Listening on port " << port << "...\n";
/*
    client_sock = accept_connection();
    cout << "Accepted connection!\n";

    while (1) {
	
        client_sock = accept_connection();
        cout << "Accepted connection from " << client_sock << "!\n";
        //close(client_sock);
    }
*/
}

serverSocket::~serverSocket() {
    freeaddrinfo(get_servinfo());
    close(get_sock());
}

int serverSocket::connect_socket(int sock, struct addrinfo* servinfo) {
    int rv;
    for (p = servinfo; p != NULL; p = p->ai_next) {
        // create socket
        sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        set_sock(sock);

        if (sock == -1) {
            cerr << "Socket() error\n";
            continue;
        }
        rv = bind(sock, p->ai_addr, p->ai_addrlen);
        // bind
        if (rv == -1) {
            close(sock);
            cerr << "Bind() error\n";
            continue;
        }
        break;
    }

    if (p == NULL) {
        return -1;
    }

    return rv;
}

int serverSocket::accept_connection() {
    addr_len = sizeof(client_addr);
    return accept(get_sock(), (struct sockaddr*)&client_addr, &addr_len);
}

string serverSocket::get_time() {
    time_t timetoday;
    time (&timetoday);
    return asctime(localtime(&timetoday));
}

string serverSocket::request::get_request_line() {
    return method + " " + filename + " " + version;
}

void serverSocket::parse_request(string request) {
    string substring = request.substr(request.find(" ") + 1);
    req.method = request.substr(0, request.find(" "));
    req.filename = substring.substr(0, substring.find(" "));
    req.version = substring.substr(substring.find(" ") + 1);

    string file_type = req.filename.substr(req.filename.find_last_of("."));
    if (file_type == ".html")
        req.is_static = true;
    else
        req.is_static = false;
}

void serverSocket::request_error(int sock, char* cause, char* errnum, char* shortmsg, char* longmsg) {
    char buf[MAXBUF], body[MAXBUF];

    sprintf(body, "%s\r\n", longmsg);

    sprintf(buf, "HTTP/1.1 %s %s\r\n", errnum, shortmsg);
    write_data(sock, buf, strlen(buf));

    sprintf(buf, "Connection: close\r\n");
    write_data(sock, buf, strlen(buf));

    char _time[TIME_SIZE];
    convert_to_char_arr(_time, get_time());
    sprintf(buf, "Date: %s", _time);
    write_data(sock, buf, strlen(buf));

    sprintf(buf, "Content-Length: %lu\r\n", strlen(body));
    write_data(sock, buf, strlen(buf));

    sprintf(buf, "Content-Type: text/html\r\n");
    write_data(sock, buf, strlen(buf));

    sprintf(buf, "Server: cpsc4510 web server 1.0\r\n");
    write_data(sock, buf, strlen(buf));
    
    sprintf(buf, "\r\ndata...\r\n");
    write_data(sock, buf, strlen(buf));

    write_data(sock, body, strlen(body));
}

void serverSocket::handle_request(int sock, request _request) {
    char cause[CAUSE_SIZE], errnum[ERRNUM_SIZE],
        shortmsg[SHORTMSG_SIZE], longmsg[MAXBUF];
    
    if (_request.method != "GET") {
        convert_to_char_arr(cause, _request.filename);
        convert_to_char_arr(errnum, "501");
        convert_to_char_arr(shortmsg, "Not Implemented");
        convert_to_char_arr(longmsg, "Server does not implement this method");
        request_error(sock, cause, errnum, shortmsg, longmsg);
        return;
    }

    if (_request.version != "HTTP/1.1") {
        convert_to_char_arr(cause, _request.filename);
        convert_to_char_arr(errnum, "502");
        convert_to_char_arr(shortmsg, "Not Supported");
        convert_to_char_arr(longmsg, "Server does not support this version");
        request_error(sock, cause, errnum, shortmsg, longmsg);
        return;
    }

    if (_request.filename.find("..") != string::npos) {
        convert_to_char_arr(cause, _request.filename);
        convert_to_char_arr(errnum, "403");
        convert_to_char_arr(shortmsg, "Forbidden");
        convert_to_char_arr(longmsg, "Server could not read this file");
        request_error(sock, cause, errnum, shortmsg, longmsg);
        return;
    }

    // handle static requests
    if (_request.is_static) {
        ifstream inFile;
        string finalmsg, out;
        inFile.open(_request.filename);

        if (inFile.fail()) {
            convert_to_char_arr(cause, _request.filename);
            convert_to_char_arr(errnum, "404");
            convert_to_char_arr(shortmsg, "Not Found");
            convert_to_char_arr(longmsg, "Server could not find this file");
            request_error(sock, cause, errnum, shortmsg, longmsg);
            return;
        }

        while (inFile.good()) {
            getline(inFile, out);
            if (inFile.good())
                out += '\n';
            finalmsg += out;
        }
        inFile.close();

        convert_to_char_arr(cause, _request.filename);
        convert_to_char_arr(errnum, "200");
        convert_to_char_arr(shortmsg, "OK");
        convert_to_char_arr(longmsg, finalmsg);
        request_error(sock, cause, errnum, shortmsg, longmsg);
    } else { // handle dynamic requests
	char file_name[FILENAME_SIZE];
        extern char** environ;
        string query_string = _request.filename.substr(_request.filename.find("?"));
        _request.filename = _request.filename.substr(0, _request.filename.find("?"));
        char* args[] = { convert_to_char_arr(file_name, _request.filename), NULL };
        string su_account = "SU_ACCOUNT=";
        string nint = "NINT=";

        setenv("QUERY_STRING", query_string.c_str(), 1);
        string query = getenv("QUERY_STRING");
        string substring = query.substr(0, query.find("&"));
        su_account += substring.substr(substring.find("=") + 1, substring.find("&"));
        substring = query.substr(query.find("&"));
        nint += substring.substr(substring.find("=") + 1);

        // check if n is valid input
	string num = substring.substr(substring.find("=") + 1);
        bool valid_input = true;
        for (int i = 0; i < num.length(); i++) {
            if (!isdigit(num[i]))
                valid_input = false;
        }
        if (valid_input) {
            if (stoi(num) < 0)
                valid_input = false;
        }
        if (!valid_input) {
            convert_to_char_arr(cause, _request.filename);
            convert_to_char_arr(errnum, "500");
            convert_to_char_arr(shortmsg, "Internal Server Error");
            convert_to_char_arr(longmsg, "Server could not complete this request");
            request_error(sock, cause, errnum, shortmsg, longmsg);
            return;
        }

        // set environment variables
        char su_acc[30];
        convert_to_char_arr(su_acc, su_account);
        char n_int[30];
        convert_to_char_arr(n_int, nint);
        putenv(su_acc);
        putenv(n_int);

        convert_to_char_arr(cause, _request.filename);
        convert_to_char_arr(errnum, "200");
        convert_to_char_arr(shortmsg, "OK");
        convert_to_char_arr(longmsg, "");
        request_error(sock, cause, errnum, shortmsg, longmsg);

        execve(args[0], args, environ);
    }
}

