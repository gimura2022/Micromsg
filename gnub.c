#include <stdio.h>
#include <stdarg.h>
#include <dirent.h>
#include <string.h>

#define gnub_impl
#include "gnub.h"

static const char* cflags     = " -std=c99 -Wall -Wno-unused-parameter -Wno-unused-function ";
static const char* cflags_rel = " -O3 -DRELEASE ";
static const char* cflags_deb = " -O0 -g3 -fsanitize=undefined -fsanitize=address -fsanitize-trap -DDEBUG ";

static const char* cppflags = " -I include ";

static const char* ldflags = " -fPIC -lgserver -lglog -lgstd ";

static char* cc = "cc";
static char* ar = "ar";
static char* prefix = "/usr/";

static char cflags_out[512]   = {0};
static char ldflags_out[512]  = {0};
static char cppflags_out[512] = {0};

static const char* executable_name = "micromsg";

static char*** argv_ptr;

static void die(const char* msg, ...)
{
	va_list args;
	va_start(args, msg);

	vfprintf(stderr, msg, args);

	va_end(args);

	exit(-1);
}

static void compile(void)
{
	gnub__compile_subproject("libglog", *argv_ptr);
	gnub__compile_subproject("libgstd", *argv_ptr);
	gnub__compile_subproject("libgserver", *argv_ptr);

	strcat(cflags_out, cflags);
	strcat(cppflags_out, cppflags);
	strcat(ldflags_out, ldflags);

	struct gnub__cmd_arr compile_commands = {0};

	char objects[GNUB_FIND_C_FILES_MAX_FILES][2][GNUB_MAX_FILE_NAME] = {0};
	size_t count;
	gnub__find_c_files("src/", objects, &count);

	for (size_t i = 0; i < count; i++) {
		gnub__append_command(&compile_commands, cc, cflags_out, cppflags_out, "-c", "-o",
				objects[i][1], objects[i][0], ldflags_out);
	}

	gnub__create_executable(&compile_commands, cc, executable_name, ldflags_out, cflags_out,
			objects, count);

	gnub__execute_commands(&compile_commands);
	gnub__free_commands(&compile_commands);
}

static void debug(void)
{
	strcpy(cflags_out, cflags_deb);
}

static void release(void)
{
	strcpy(cflags_out, cflags_rel);
}

int main(int argc, char* argv[])
{
	argv_ptr = &argv;

	cc     = getenv("CC")     == NULL ? cc : getenv("CC");
	ar     = getenv("AR")     == NULL ? ar : getenv("AR");
	prefix = getenv("PREFIX") == NULL ? prefix : getenv("PREFIX");

	gnub__recompile_self(argv);

	gnub__add_target("release", release);
	gnub__add_target("debug", debug);
	gnub__add_target("compile", compile);

	const char* default_targets[] = { "release", "compile" };
	gnub__run_targets(argc, argv, default_targets, array_lenght(default_targets));

	return 0;
}