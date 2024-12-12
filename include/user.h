#ifndef _user_h
#define _user_h

enum {
	USRST_CONNECTED = 0,
	USRST_LOGINED,
	USRST_ACTIVE,
};

struct user {
	int fd;

	char* login;
	char* passwd;
	char* ip;

	int user_state;

	struct user *next, *prev;
};

struct user* create_user(void);
void deleate_user(struct user* user);

typedef void (*user_enumerator_f)(struct user*, void*);
void enumerate_users(user_enumerator_f func, void* data);

#endif
