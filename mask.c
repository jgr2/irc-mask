#include <stdlib.h>
#include <string.h>
/*
 * Hint: scroll down to match()
 */
/**********************************************************************/

typedef struct sbuff sbuff;

struct sbuff {
	char *p;
	size_t len, cap;
};

static void *sbuff_init(sbuff *s, size_t cap);
static void *sbuff_push(sbuff *s, char c);
static void *sbuff_xchg(sbuff *s, char c);
static void  sbuff_reset(sbuff *s);

# define     sbuff_free(s) \
	(free((s)->p))
# define     sbuff_head(s) \
	(((s)->len) ? *((s)->p + (s)->len - 1): -1)

/**********************************************************************/

enum META {EXACT, WILD_ONCE, WILD_MANY, EXIT};

typedef enum META meta;

typedef struct {
	meta m;
	sbuff s;
} token;

static long gettoken(token *t, char *p);

int match (char *mask, char *cand) {

	int r;
	unsigned i, j;

	char *p;
	token t;

	sbuff_init(&t.s, 32);
	t.m = 0;

	i = 0;
	j = 0;

	for (;;) {
		i += gettoken(&t, mask + i);

		switch (t.m) {
			case WILD_ONCE:
				if (cand[j]) {
					j++;
				}

				if (&t.s.len == 0) {
					break;
				}
				/* fallthrough */
			case EXACT:
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
	sbuff_free(&t.s);
	return r;
}

static long gettoken (token *t, char *p) {

	unsigned j;

	sbuff_reset(&t->s);

	j = 0;

	/* read a meta */
	if (!p[j]) {
		t->m = EXIT;
		return j;
	}

	if (p[j] == '*') {
		t->m = WILD_MANY;
	} else if (p[j] == '?') {
		t->m = WILD_ONCE;
	} else {
		sbuff_push(&t->s, p[j]); 
		t->m = EXACT;
	}
	j++;

	/* read a sub string */
	while (p[j]) {
		if (p[j] == '*' || p[j] == '?') {
			if (t->s.len && sbuff_head(&t->s) == '\\') {
				sbuff_xchg(&t->s, p[j]);
				j += 1;
			} else {
				break;
			}
		} else {
			sbuff_push(&t->s, p[j++]);
		}
	}
	return j;
}

static void *sbuff_init (sbuff *s, size_t cap) {
	
	if (cap) {
		s->len = 0;
		s->cap = cap;
		s->p = calloc(cap, sizeof *s->p);
	}

	return s->p;
}

static void *sbuff_push (sbuff *s, char c) {
	char *p;

	if (s->len >= s->cap) {
		if ((p = realloc(s->p, s->cap * 2)) == NULL) {
			return NULL;
		}

		s->cap *= 2;
		s->p = p;
	}

	p = s->p + s->len++;
	*p = c;
	return p;
}

static void *sbuff_xchg (sbuff *s, char c) {
	char *p;

	if (!s->len) {
		return NULL;
	}
	p = s->p + (s->len-1);
	*p = c;
	return p;
}

static void sbuff_reset (sbuff *s) {
	memset(s->p, '\0', s->len);
	s->len = 0;
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
