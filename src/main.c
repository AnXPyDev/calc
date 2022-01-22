#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <malloc.h>

double lerp(double a, double b, double m) {
	return a + (b - a) * m;
}

double clamp(double a, double min, double max) {
	if (a < min) {
		return min;
	} else if (a > max) {
		return max;
	}
	return a;
}

double wrap(double a, double min, double max) {
	if (a < min) {
		return max - fmod(max - a, max - min);
	} else if (a > max) {
		return min + fmod(a - min, max - min);
	}
	return a;
}

typedef double (*operation_fb)(double, double);
typedef double (*operation_fu)(double);
typedef int (*operation_ib)(int, int);
typedef int (*operation_iu)(int);

enum optype_e {
	optype_err, optype_b, optype_u
};

struct operation {
	enum optype_e optype;
	void *op;
};


char _ret_ok;
char *ret_ok = &_ret_ok;

void *imode(int, int, int*, char***);
void *fmode(int, double, int*, char***);
void *hexmode(int, int, int*, char***);
void *submode(void*, void*, int*, char***);


#include "imode.h"
#include "fmode.h"
#include "hexmode.h"

#include "color.h"
#include "colormode.h"

#include "submode.h"

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "No arguments provided");
		return 1;
	}

	argc--;
	argv++;
	char *arg = *argv;
	if (arg[0] == '@') {
		argc--;
		argv++;
		if (strcmp(arg, "@x") == 0) {
			return hexmode(0, 0, &argc, &argv) == NULL;
		} else if (strcmp(arg, "@i") == 0) {
			return imode(0, 0, &argc, &argv) == NULL;
		} else if (strcmp(arg, "@rgb") == 0 || strcmp(arg, "@rgba") == 0 || strcmp(arg, "@hsl") == 0 || strcmp(arg, "@hsla") == 0 || strcmp(arg, "@c") == 0) {
			return dispatchcolormode(arg, &argc, &argv) == NULL;
		} else {
			return fmode(0, 0, &argc, &argv) == NULL;
		}
	}

	return fmode(0, 0, &argc, &argv) == NULL;
}
