#include <stdio.h>
#include <stdlib.h>
#include "aesdsocket.h"
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <signal.h> 
#include <sys/wait.h>
void getaddrinfoex() {

#if 0
    struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr,"usage: showip hostname\n");
        return 1;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
    }

    printf("IP addresses for %s:\n\n", argv[1]);

    for(p = res;p != NULL; p = p->ai_next) {
        void *addr;
        char *ipver;

        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        if (p->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        // convert the IP to a string and print it:
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        printf("  %s: %s\n", ipver, ipstr);
    }

    freeaddrinfo(res); // free the linked list

#endif
}

int server_skt = 0;
int rcv_data_file = 0;
bool continue_to_listen = true;
pid_t daemon_id = 0;
void cleanup() {
	close(server_skt);
	close(rcv_data_file);
	system("rm /var/tmp/aesdsocketdata");
	continue_to_listen = false;
}
void signal_handler() {
	if (daemon_id == 0) {
		printf("SIG handler");
		syslog(LOG_INFO, "Caught signal, exiting");
		cleanup();
		exit(0);
	}
}
int main(int argc, char *argv[])
{
	struct addrinfo hints, *res;
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);

	char buf[1024];
	
	struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = signal_handler;
    sigaction(SIGTERM, &action, NULL);
    
    memset(&action, 0, sizeof(action));
    action.sa_handler = signal_handler;
    sigaction(SIGINT, &action, NULL);
	// first, load up address structs with getaddrinfo():

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;  // use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

	getaddrinfo(NULL, "9000", &hints, &res);

	// make a socket:

	server_skt = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (server_skt ==-1) {
		printf("Server socket creation error");
		return -1;
	}
	

	// bind it to the port we passed in to getaddrinfo():

	if (bind(server_skt, res->ai_addr, res->ai_addrlen) == -1) {
		printf("Server socket bind error");
		return -1;
	}
	if (argc > 1 && strcmp(argv[1],"-d") == 0) {
		printf("Daemon mode\n");
		daemon_id = fork();
	}
	if (daemon_id == 0) {
		rcv_data_file = open("/var/tmp/aesdsocketdata", O_WRONLY | O_CREAT | O_TRUNC, 0777);
		int read_data_file = 0;
		while (continue_to_listen) {
			if (listen(server_skt, 1) == -1) {
				printf("server skt could not listen");
				return -1;
			}
			int client_skt = accept(server_skt, (struct sockaddr *)&client_addr,
								&client_addr_len);
			printf("accepted the client conn %d\n", client_skt);
			syslog(LOG_INFO, "Accepted connection from %u", client_addr.sin_addr.s_addr);
			memset(buf, 0, 1024);
			int rcv_cnt = 0;
			int retry = 1;
			while ( (rcv_cnt = recv(client_skt, buf, sizeof(buf), MSG_DONTWAIT)) > 0 || retry--) {
				printf("receied buf cnt =%d cont=%s", rcv_cnt, buf);
				write(rcv_data_file, buf, rcv_cnt);
				//send(client_skt, buf, sizeof(buf), MSG_DONTWAIT);
				memset(buf, 0, 1024);
			}
			memset(buf, 0, 1024);
			read_data_file = open("/var/tmp/aesdsocketdata", O_RDONLY, 0777);
			while ((rcv_cnt = read(read_data_file, buf, sizeof(buf))) > 0) {
				printf("read and sending cnt=%d content =%s ", rcv_cnt, buf);
				send(client_skt, buf, rcv_cnt, MSG_DONTWAIT);
				memset(buf, 0, 1024);
			}
			printf("Finished reading %d error %d\n", rcv_cnt, errno);
			close(read_data_file);
			//close(client_skt);
			syslog(LOG_INFO, "Closed connection from %u", client_addr.sin_addr.s_addr);
		}
		close(rcv_data_file);
		freeaddrinfo(res);
	} /*else {
		int status = 0;
		waitpid(daemon_id, &status, 0);
	}*/
	return 0;
}
