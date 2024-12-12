#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include <gserver/gserver.h>
#include <gstd/allocators.h>
#include <glog.h>

#define BUF_SIZE 1024 * 4

static struct glog__logger logger         = {0};
static struct glog__logger gserver_logger = {0};
static struct gstd__memmanager memmanager = { .allocator = malloc, .deallocator = free };

static void send_data(int fd, char* buf, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	vsprintf(buf, fmt, args);
	send(fd, buf, strlen(buf), 0);

	va_end(args);
}

static bool read_data(int fd, char* buf)
{
	size_t read_size;
	if ((read_size = read(fd, buf, BUF_SIZE)) < 0)
		return false;

	buf[read_size] = '\0';

	return true;
}

static void unexended_client(const struct gserver__reciver_args* args, char* buf)
{
	send_data(args->fd, buf, "Welcome in micromsg server!\n");
	send_data(args->fd, buf, "Enter login: ");
	if (!read_data(args->fd, buf)) return;

	send_data(args->fd, buf, "%s\n", buf); 
}

static int reciver(const struct gserver__reciver_args* args)
{
	char* buf = malloc(BUF_SIZE);
	size_t read_size;

	send_data(args->fd, buf, "EXCONNECT\n");
	if (!read_data(args->fd, buf))
		return 1;

	if (strcmp(buf, "SUPPORT\n") == 0) {
		send_data(args->fd, buf, "ERROR\n");
		send_data(args->fd, buf, "Extended clients is not supported.\n"); 

		goto done;
	} else if (strcmp(buf, "NOSUPPORT\n") == 0) {
		unexended_client(args, buf);
		goto done;
	} else {
		send_data(args->fd, buf, "ERROR\n");
		send_data(args->fd, buf, "Unexepted message.\n");

		goto done;
	}

done:
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
