#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sti/sti.h"

#include <ctype.h>

#include <errno.h>
#include <sys/epoll.h>
#include <sys/resource.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define fatal(fmt, ...) do { \
	fprintf(stderr, "%s:%d " fmt, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__); \
	exit(1); \
} while(0);



struct server;

typedef struct connection {
	struct server* srv;
	int peerfd;
	struct sockaddr_storage peeraddr;
	
	char** buf;
	size_t* buf_remain;
	
	int new_data;
	
	void (*buffer_full)(struct connection*);
	void (*got_data)(struct connection*);
	
	void* user_data;
	
} connection_t;


typedef struct server {
	int epollfd;
	
	int listen_socket;
	
	VEC(connection_t*) cons;
	
	void (*on_accept)(connection_t*);
	
} server_t;



void on_data(connection_t* con);



void add_epoll_watch(int epollfd, int fd, void* data, int events) {
	struct epoll_event ee = {0};
	
	ee.events = events;
	ee.data.ptr = data;

	int ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ee);
	if(ret < 0) {
		printf("epoll error: %s\n", strerror(errno));
	}
}


server_t* server_init(int epollfd, int port) {
//	int epollfd = epoll_create(16);
	server_t* srv = calloc(1, sizeof(*srv));
	srv->epollfd = epollfd;
	
	srv->listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	fcntl(srv->listen_socket, F_SETFL, O_NONBLOCK);
	
	int on = 1;
	setsockopt(srv->listen_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	
	struct sockaddr_in bindaddr, peeraddr;
	
	bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	bindaddr.sin_family = AF_INET;
	bindaddr.sin_port = htons(port);
	
	if(bind(srv->listen_socket, (struct sockaddr*)&bindaddr, sizeof(bindaddr)) < 0) {
		fatal("Failed to bind socket");
	}
	
	listen(srv->listen_socket, SOMAXCONN);
	
	add_epoll_watch(epollfd, srv->listen_socket, srv->listen_socket, EPOLLIN);
	
	return srv;
}


void server_tick(server_t* srv, int wait) {

	struct epoll_event ee = {0};
//	printf("epoll waiting\n");
	
	int ret = epoll_wait(srv->epollfd, &ee, 1, wait); 
	
	if(ret == -1) {
		fatal("epoll error\n");
	}
	
	if(ret == 0) return;
	
	// new connections
	if(ee.data.fd == srv->listen_socket) {
		connection_t* con = calloc(1, sizeof(*con));
	
		int addrlen;
		con->peerfd = accept(srv->listen_socket, (struct sockaddr*)&con->peeraddr, &addrlen);
		fcntl(con->peerfd, F_SETFL, O_NONBLOCK);
	
		if(con->peerfd < 0) {
			printf("socket error: %s\n", strerror(errno));
	        exit(5);
	    }
		
		con->srv = srv;
		srv->on_accept(con);
			
		add_epoll_watch(srv->epollfd, con->peerfd, con, EPOLLIN);
		
		VEC_PUSH(&srv->cons, con);
		
		return;
	}
	
	
	if(ee.events & EPOLLIN) { // data to read
//		printf("got data\n\n");
		
		connection_t* con = ee.data.ptr;
		
		int bread = 0;
		do {
			if(*con->buf_remain == 0) {
				con->buffer_full(con);
			}
			
			bread = recv(con->peerfd, *con->buf, *con->buf_remain, 0);
			
			if(bread > 0) {
				*con->buf_remain -= bread;
				con->new_data = 1;	
			}
			
	        if(errno == EAGAIN) {
				break;
			}
		} while(bread > 0);
		
		if(con->new_data) {
			printf("got new data\n");
			if(con->got_data) con->got_data(con);
			con->new_data = 0;
		}
	}
	
}


void connection_close(connection_t* con) {
	struct epoll_event ee = {0};
	epoll_ctl(con->srv->epollfd, EPOLL_CTL_DEL, con->peerfd, &ee);
				
	close(con->peerfd);
	
	VEC_RM_VAL(&con->srv->cons, con);
	
	free(con);
}


enum {
	REQST_START,
	REQST_GOT_LEN,
	REQST_PARSE_HEADERS,
	REQST_GET_CONTENT,
	REQST_RESPOND,
};


typedef struct {
	char* key, *value;
	long numval;
} req_header;

typedef struct {
	char* buf;
	size_t buf_alloc;
	size_t buf_remain;
	
	int state;
	
	int header_len;
	int header_netstring_len;
	int header_offset; // first char after the colon
	char* content_start;
	
	long content_len;
	long total_len;
	
	VEC(req_header) headers;
	
} request;



void check_buffer(connection_t* con) {
	request* req = con->user_data;
	
	if(!req->buf) {
		req->buf = malloc(4096);
		req->buf_alloc = 4096;
		req->buf_remain = 4096;
		return;
	}
	
	req->buf_remain = req->buf_alloc;
	req->buf_alloc *= 2;
	req->buf = realloc(req->buf, req->buf_alloc);
}

void accepted(connection_t* con) {
	
	printf("accepted connection\n");
	
	request* req = calloc(1, sizeof(*req));
	
	con->buffer_full = check_buffer;
	con->got_data = on_data;
	con->user_data = req;
	con->buf = &req->buf;
	con->buf_remain = &req->buf_remain;
	
}

void on_data(connection_t* con) {
	request* req = con->user_data;
	int len;
	
	while(1) {
		switch(req->state) {
			case REQST_START:
				// look for the netstring encoding of the SCGI header length
				len = req->buf_alloc - req->buf_remain;
				
				char* colon = strnchr(req->buf, ':', len);
				if(!colon) {
					return; // not enough data, somehow...
				}
				
				req->header_netstring_len = strtol(req->buf, NULL, 10);
				req->header_len = req->header_netstring_len - (colon - req->buf) - 1; // minus 1 for the trailing netstring comma
				req->header_offset = (colon - req->buf) + 1;
				
				req->state = REQST_GOT_LEN;
				break;
			
			case REQST_GOT_LEN:
				// wait for all of the headers to arrive
				len = req->buf_alloc - req->buf_remain;
				
				if(len >= req->header_netstring_len) {
					req->state = REQST_PARSE_HEADERS;
					break;
				}
				return;
			
			case REQST_PARSE_HEADERS: {
				
				char* s = req->buf + req->header_offset;
				char* header_end = req->buf + req->header_netstring_len - 1; // minus one for the comma
				req->content_start = header_end + 1;
				
				while(s < header_end) {
					VEC_INC(&req->headers);
					req_header* header = &VEC_TAIL(&req->headers);
					header->key = strdup(s);
					s += strlen(s) + 1; // skip the null byte
					header->value = strdup(s);
					s += strlen(s) + 1; // skip the null byte
					
					if(isdigit(header->value[0])) {
						header->numval = strtol(header->value, NULL, 10);
					}
					
					if(0 == strcasecmp(header->key, "CONTENT_LENGTH")) {
						req->content_len = header->numval;
						req->total_len = req->content_len + req->header_netstring_len;
					}
					
					printf("Header: %s = %s\n", header->key, header->value);
				}
				
				req->state = REQST_RESPOND;
				break;
			}
			
			case REQST_GET_CONTENT:
				len = req->buf_alloc - req->buf_remain;
				if(len >= req->total_len) {
					req->state = REQST_RESPOND;
					break;
				}
				return;
			
			case REQST_RESPOND: {
				char* resp = "Status: 200 OK\r\nContent-Type: text/plain\r\n\r\nFoobar\r\n";
				
				send(con->peerfd, resp, strlen(resp), 0);
				
				connection_close(con);
				
				return;
			}
		}
	}
}




int main(int argc, char* argv[]) {

	
	int epollfd = epoll_create(16);
	
	server_t* srv = server_init(epollfd, 4999);
	srv->on_accept = accepted;
	
	
	while(1) {
		
		server_tick(srv, 100);
		
//		sleep(1);
	}


	close(epollfd);

	return 0;
}



