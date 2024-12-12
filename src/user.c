#include <stdlib.h>
#include <stddef.h>

#include "user.h"

static struct user* start = NULL;
static struct user* end   = NULL;

struct user* create_user(void)
{
	if (start == NULL && end == NULL) {
		start = malloc(sizeof(struct user));
		end   = start;

		start->next = NULL;
		start->prev = NULL;

		return start;
	}

	end->next       = malloc(sizeof(struct user));
	end->next->next = NULL;
	end->next->prev = end;
	end             = end->next;

	return end;
}

void deleate_user(struct user* user)
{
	user->prev->next = user->next;
	user->next->prev = user->prev;

	free(user);
}

void enumerate_users(user_enumerator_f func, void* data)
{
	for (struct user* i = start; i != NULL; i = i->next) 
		func(i, data);
}
