#include "serversocket.h"
#include <vector>
#include <queue>
#include <semaphore.h>
#include <pthread.h>

using namespace std;

int allocate_port(const string& student_id);

sem_t active_requests, buffer;
pthread_mutex_t lock;
queue<int> pending_requests;

//void* assign_request(void* arg);
void* do_request(void* arg);

int main(int argc, char* argv[]) {

    int total_threads = stoi(argv[2]);
    vector<pthread_t> thread_pool;
    for (int i = 0; i < total_threads; i++) {
        pthread_t new_thread;
        if (pthread_create(&new_thread, NULL, &do_request, (void*)i) != 0) {
            cerr << "Failed to create thread " + to_string(i) << endl;
            exit(1);
        }
	
        thread_pool.push_back(new_thread);
    }
    pthread_mutex_init(&lock, NULL);
 
    int buffer_size = stoi(argv[3]);
    sem_init(&active_requests, 0, 0);
    sem_init(&buffer, 0, buffer_size);

    string id = string(argv[1]);
    string port = to_string(allocate_port(id));
    serverSocket server(port);
    int client_sock;

    while (1) {
        sem_wait(&buffer);
        client_sock = server.accept_connection();
	cout << "Accepted connection from " << client_sock << endl;
        pthread_mutex_lock(&lock);
        pending_requests.push(client_sock);
        pthread_mutex_unlock(&lock);
        sem_post(&active_requests);
        
    }


    for (int i = 0; i < total_threads; i++) {
        if (pthread_join(thread_pool[i], NULL) != 0) {
            cerr << "Failed to join thread " + to_string(i) << endl;
            exit(1);
        }
    }

    pthread_mutex_destroy(&lock);

    sem_destroy(&active_requests);
    sem_destroy(&buffer);
}


void* do_request(void* arg) {
    while (1) {
        int client_sock;
        sem_wait(&active_requests);
        pthread_mutex_lock(&lock);
        client_sock = pending_requests.front();
        pending_requests.pop();
        pthread_mutex_unlock(&lock);
	      sem_post(&buffer);
        cout << to_string(client_sock) << endl;
        close(client_sock);
    }
}

int allocate_port(const string& student_id) {
    // Use std::hash function to hash the student ID
    size_t hash_value = hash<string>{}(student_id);
    // Map the hash value to the range of 0-99
    int port_number = static_cast<int>(hash_value % 100);
    // Map the port number to the range of 10401-10500
    port_number += 10401;
    return port_number;
}
