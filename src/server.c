#define _GNU_SOURCE
#include <stdio.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <gserver/gserver.h>
#include <gstd/allocators.h>
#include <glog.h>

#include "server.h"
#include "user.h"

#define BUF_SIZE 1024 * 4
#define MAX_USERS 256

static struct glog__logger gserver_logger = {0};

struct glog__logger logger         = {0};
struct gstd__memmanager memmanager = { .allocator = malloc, .deallocator = free };

static bool read_data(int fd, char* buf)
{
	size_t read_size;
	if ((read_size = read(fd, buf, BUF_SIZE)) < 0)
		return false;

	buf[read_size] = '\0';

	return true;
}

static void send_to_all_enumerator(struct user* user, void* data)
{
	char* buf = data;
	dprintf(user->fd, "%s", buf);
}

static void send_to_all(char* buf, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	enumerate_users(send_to_all_enumerator, buf);
}

static void unexended_client_auth(const struct gserver__reciver_args* args, char* buf, struct user* user)
{
	dprintf(args->fd, "Enter login: ");
	if (!read_data(args->fd, buf))
		return;

	user->login = malloc(strlen(buf));
	buf[strlen(buf) - 1] = '\0';
	strcpy(user->login, buf);

	user->user_state = USRST_LOGINED;
}

static void unexended_client_loop(const struct gserver__reciver_args* args, char* buf, struct user* user)
{
	user->user_state = USRST_ACTIVE;
	send_to_all(buf, "User %s connected!\n", user->login);

	while (read_data(args->fd, buf)) {
		char* msg = malloc(strlen(buf));
		buf[strlen(buf) - 1] = '\0';
		memset(msg, '\0', strlen(buf));
		strcpy(msg, buf);

		send_to_all(buf, "<%s> %s\n", user->login, msg);

		free(msg);
	}

	send_to_all(buf, "User %s disconnected!\n", user->login); 
	user->user_state = USRST_CONNECTED;
}

static int reciver(const struct gserver__reciver_args* args)
{
	char* buf = malloc(BUF_SIZE);

	struct user* user = create_user();
	user->user_state  = USRST_CONNECTED;

	user->login  = NULL;
	user->passwd = NULL;
	
	user->fd = args->fd;

	socklen_t len = 0;
	struct sockaddr_in addr = {0};
	if (getpeername(args->fd, (struct sockaddr*) &addr, &len) < 0)
		goto done;

	user->ip = inet_ntoa(addr.sin_addr);

	dprintf(args->fd, "EXCONNECT\n");
	if (!read_data(args->fd, buf))
		goto done;

	if (strcmp(buf, "SUPPORT\n") == 0) {
		dprintf(args->fd, "ERROR\n");
		dprintf(args->fd, "Extended clients is not supported.\n");

		goto done;
	} else if (strcmp(buf, "NOSUPPORT\n") == 0) {
		unexended_client_auth(args, buf, user);
		unexended_client_loop(args, buf, user);

		goto done;
	} else {
		dprintf(args->fd, "ERROR\n");
		dprintf(args->fd, "Invalid message.\n");

		goto done;
	}

done:
	if (user->login != NULL) free(user->login);
	if (user->passwd != NULL) free(user->passwd);

	deleate_user(user);
	free(buf);

	return 0;
}

static void init(void)
{
	glog__init();

	glog__logger_from_prefix(&logger, "micromsg server");
	glog__logger_from_prefix(&gserver_logger, "gserver");

	gserver__init(&gserver_logger, &memmanager);
}

int main(int argc, char* argv[])
{
	init();
	gserver__start_server(reciver, NULL, 8080);
	
	return 0;
}
