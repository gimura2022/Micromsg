#define _GNU_SOURCE
#include <stdio.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUF_SIZE 1024 * 2

static void* read_thread(void* data)
{
	char* buf = malloc(BUF_SIZE);
	int fd    = *((int*) data);
	size_t recv_size;

	while ((recv_size = recv(fd, buf, BUF_SIZE, 0)) >= 0) {
		printf("%s", buf);
		memset(buf, '\0', BUF_SIZE);
	}

	free(buf);

	return NULL;
}

static void* write_thread(void* data)
{
	char* buf = malloc(BUF_SIZE);
	int fd    = *((int*) data);
	
	while (true) {
		fgets(buf, BUF_SIZE, stdin);	
		dprintf(fd, "%s", buf);
	}

	free(buf);

	return NULL;
}

int main(int argc, char* argv[])
{
	if (argc != 2) {
		fprintf(stderr, "USAGE: micromsg <server addr>\n");
		return 1;
	}

	int fd;
	struct sockaddr_in serv_addr = {0};
	struct hostent* server = NULL;

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return 1;

	server = gethostbyname(argv[1]);
	if (server == NULL)
		return 1;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port   = htons(8080);
	memcpy(&serv_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);

	if (connect(fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
		return 1;

	char* buf = malloc(BUF_SIZE);
	size_t recv_size;

	if ((recv_size = recv(fd, buf, BUF_SIZE, 0)) < 0)
		return 1;

	dprintf(fd, "NOSUPPORT\n");
	free(buf);

	pthread_t write_thread_id, read_thread_id;
	pthread_create(&read_thread_id, NULL, read_thread, (void*) &fd);
	pthread_create(&write_thread_id, NULL, write_thread, (void*) &fd);
	pthread_join(read_thread_id, NULL);
	pthread_join(write_thread_id, NULL);

	close(fd);

	return 0;
}
