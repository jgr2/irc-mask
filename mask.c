#include <stdlib.h>
#include <string.h>
/*
 * Hint: scroll down to match()
 */
/**********************************************************************/

typedef struct {
	char *p;
	size_t len, cap;
} String;

void *string_init (String *s, size_t cap) {
	
	if (cap) {
		s->len = 0;
		s->cap = cap;
		s->p = malloc(cap);
	}

	return s->p;
}

# define string_free(s) (free((s)->p))

void *string_push (String *s, char c) {
	char *p;

	if (s->len >= s->cap) {
		if ((p = realloc(p, s->cap * 2)) == NULL) {
			return NULL;
		}

		s->cap *= 2;
		s->p = p;
	}

	p = s->p + s->len++;
	*p = c;
	*(p+1) = '\0'; // possible write error
	return p;
}

# define string_reset(s) ((s)->len = 0, *(s)->p = '\0')

/**********************************************************************/

typedef enum Meta Meta;
enum Meta {EXACT, WILD_ONCE, WILD_MANY, EXIT};

typedef struct {
	Meta m;
	String s;
} Token;

void gettoken (Token *t, char *p, unsigned *i) {

	unsigned j;

	string_reset(&t->s);

	j = 0;
	// read a meta
	if (!p[j]) {
		t->m = EXIT;
		return;
	}

	if (p[j] == '*') {
		t->m = WILD_MANY;
	} else if (p[j] == '?') {
		t->m = WILD_ONCE;
	} else {
		string_push(&t->s, p[j]); //
		t->m = EXACT;
	}
	j++;

	// read a sub string
	while (p[j]) {
		if (p[j] == '*' || p[j] == '?') {
			if (t->s.len && t->s.p[t->s.len-1] == '\\') {
				t->s.p[t->s.len-1] = p[j];
				j += 1;
			} else {
				break;
			}
		} else {
			string_push(&t->s, p[j++]);
		}
	}
	*i += j;
}

int match (char *mask, char *cand) {

	int r;
	unsigned i, j;

	char *p;
	Token t;

	string_init(&t.s, 32);
	t.m = 0;

	i = 0;
	j = 0;

	for (;;) {
		gettoken(&t, mask + i, &i);

		switch (t.m) {
			case WILD_ONCE:
				if (cand[j]) {
					j++;
				}
				if (&t.s.len != 0) {
					goto _EXACT;
				}
				break;
			case EXACT:
_EXACT:
				r = strncmp(t.s.p, cand+j, t.s.len);
				if (r) {
					goto RETURN;
				}
				j += t.s.len;
				break;
			case WILD_MANY:
				if (t.s.len) {
					p = strstr(cand + j, t.s.p);

					if (!p) {
						r = -1;
						goto RETURN;
					}

					j += (p - (cand + j)) + t.s.len;
				
					break;
				} else if (!mask[i]) {
					r = 0;
					goto RETURN;
				}
				break;
			case EXIT:
				r = mask[i] - cand[j]; 
				goto RETURN;
		}
	}
RETURN:
	string_free(&t.s);
	return r;
}
/**********************************************************************/

#include <assert.h>
#include <stdio.h>

struct t {
	char *mask, *cand;
} test[] = {
#if 0
#endif	
	{"*", "foobar"},
	{"*bar", "foobar"},
	{"foo*", "foobar"},
	{"foo*r", "foobar"},

	{"foo*ar", "foobar"},
	{"foo*bar", "foobar"},
	
	{"foo?ar", "foobar"},
	{"fo??ar", "foobar"},
	{"fo???r", "foobar"},
	{"f????r", "foobar"},
	{"f?????", "foobar"},
	{"??????", "foobar"},

	{"f\\*obar", "f*obar"},
	{"f\\*oba\\?", "f*oba?"},
#if 0
#endif

};

size_t ntests = sizeof test / sizeof test[0];

int main () {
	size_t i;

	for (i = 0; i < ntests; i++) {
		if (match(test[i].mask, test[i].cand)) {
			fprintf(stderr, "\"%s\"\t\"%s\"\n", test[i].mask, test[i].cand);
			exit(EXIT_FAILURE);
		}
	}
	exit(EXIT_SUCCESS);
}
