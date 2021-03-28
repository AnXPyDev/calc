#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <malloc.h>

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


int opiadd(int a, int b) { return a + b; }
int opisub(int a, int b) { return a - b; }
int opimul(int a, int b) { return a * b; }
int opidiv(int a, int b) { return a / b; }
int opipow(int a, int b) { return pow(a, b); }
int opinegate(int a) { return 0 - a; };

double opfadd(double a, double b) { return a + b; }
double opfsub(double a, double b) { return a - b; }
double opfmul(double a, double b) { return a * b; }
double opfdiv(double a, double b) { return a / b; }
double opfpow(double a, double b) { return pow(a, b); }

double opffloor(double a) { return floor(a); };
double opfceil(double a) { return floor(a + 1); };
double opfround(double a) { return floor(a + 0.5); };
double opfnegate(double a) { return 0 - a; };

struct operation getiop(char *opname) {
	struct operation ret = {0, NULL};
	if (strcmp(opname, "+") == 0) {
		ret.optype = optype_b;
		ret.op = &opiadd;
	} else if (strcmp(opname, "-") == 0) {
		ret.optype = optype_b;
		ret.op = &opisub;
	} else if (strcmp(opname, "*") == 0) {
		ret.optype = optype_b;
		ret.op = &opimul;
	} else if (strcmp(opname, "/") == 0) {
		ret.optype = optype_b;
		ret.op = &opidiv;
	} else if (strcmp(opname, "^") == 0) {
		ret.optype = optype_b;
		ret.op = &opipow;
	} else if (strcmp(opname, "negate") == 0) {
		ret.optype = optype_u;
		ret.op = &opinegate;
	}
	return ret;
}

struct operation getfop(char *opname) {
	struct operation ret = {0, NULL};
	if (strcmp(opname, "+") == 0) {
		ret.optype = optype_b;
		ret.op = &opfadd;
	} else if (strcmp(opname, "-") == 0) {
		ret.optype = optype_b;
		ret.op = &opfsub;
	} else if (strcmp(opname, "*") == 0) {
		ret.optype = optype_b;
		ret.op = &opfmul;
	} else if (strcmp(opname, "/") == 0) {
		ret.optype = optype_b;
		ret.op = &opfdiv;
	} else if (strcmp(opname, "^") == 0) {
		ret.optype = optype_b;
		ret.op = &opfpow;
	} else if (strcmp(opname, "floor") == 0) {
		ret.optype = optype_u;
		ret.op = &opffloor;
	} else if (strcmp(opname, "ceil") == 0) {
		ret.optype = optype_u;
		ret.op = &opfceil;
	} else if (strcmp(opname, "round") == 0) {
		ret.optype = optype_u;
		ret.op = &opfround;
	} else if (strcmp(opname, "negate") == 0) {
		ret.optype = optype_u;
		ret.op = &opfnegate;
	}
	return ret;
}

char _ret_ok;
char *ret_ok = &_ret_ok;

void *imode(int, int, int*, char***);
void *fmode(int, double, int*, char***);
void *hexmode(int, int, int*, char***);
void *submode(void*, void*, int*, char***);

void *imode(int init, int initval, int *argcp, char ***argvp) {
	int argc = *argcp;
	char **argv = *argvp;

	int reg[2] = {0, 0};
	int *regptr = &reg[0];
	if (init) {
		regptr++;
		reg[0] = initval;
	}
	struct operation op = {0, NULL};
	
	while (argc > 0) {
		char *arg = *argv;
		argv++;
		argc--;
		if (arg[0] >= 48 && arg[0] <= 57) {
			*regptr = atoi(arg);
			regptr++;
			if (regptr == &reg[1] + 1) {
				binop:;
				if (op.optype == optype_b && op.op != NULL) {
					operation_ib fptr = op.op;
					reg[0] = fptr(reg[0], reg[1]);
					regptr = &reg[1];
				} else {
					fprintf(stderr, "No operation loaded\n");
					return NULL;
				}
			} else {
				if (op.optype == optype_u && op.op != NULL) {
					operation_iu fptr = op.op;
					reg[0] = fptr(reg[0]);
					regptr = &reg[1];
				}
			}
		} else if (arg[0] == '[') {
			submode(&imode, regptr, &argc, &argv);
			regptr++;
			if (regptr == &reg[2]) {
				goto binop;
			}
		} else if (arg[0] == ']') {
			char *ret = malloc(sizeof(void*) + sizeof(int));
			*((void**)ret) = &imode;
			*((int*)(ret + sizeof(void*))) = reg[0];
			*argcp = argc;
			*argvp = argv;
			return ret;
		} else if (arg[0] == '@') {
			if (strcmp(arg, "@f") == 0) {
				void *stat = fmode(1, (double)reg[0], &argc, &argv);
				*argcp = argc;
				*argvp = argv;
				return stat;
			} else if (strcmp(arg, "@x") == 0) {
				void *stat = hexmode(1, reg[0], &argc, &argv);
				*argcp = argc;
				*argvp = argv;
				return stat;
			}
		} else {
			op = getiop(arg);
			if (op.op == NULL) {
				fprintf(stderr, "Invalid operation %s\n", arg);
				return NULL;
			}
			if (op.optype == optype_u && regptr == &reg[1]) {
				operation_iu fptr = op.op;
				reg[0] = fptr(reg[0]);
				regptr = &reg[1];
			}
		}
	}

	printf("%i\n", reg[0]);

	return 0;
}

void *fmode(int init, double initval, int *argcp, char ***argvp) {
	int argc = *argcp;
	char **argv = *argvp;
	double reg[2] = {0, 0};
	double *regptr = &reg[0];
	if (init) {
		regptr++;
		reg[0] = initval;
	}
	struct operation op = {0, NULL};
	
	while (argc > 0) {
		char *arg = *argv;
		argv++;
		argc--;
		if (arg[0] >= '0' && arg[0] <= '9') {
			*regptr = atof(arg);
			regptr++;
			if (regptr == &reg[1] + 1) {
				binop:;
				if (op.optype == optype_b && op.op != NULL) {
					operation_fb fptr = op.op;
					reg[0] = fptr(reg[0], reg[1]);
					regptr = &reg[1];
				} else {
					fprintf(stderr, "No operation loaded\n");
					return NULL;
				}
			} else {
				if (op.optype == optype_u && op.op != NULL) {
					operation_fu fptr = op.op;
					reg[0] = fptr(reg[0]);
					regptr = &reg[1];
				}
			}
		} else if (arg[0] == '@') {
			if (strcmp(arg, "@i") == 0) {
				void *stat = imode(1, (int)reg[0], &argc, &argv);
				*argcp = argc;
				*argvp = argv;
				return stat;
			} else if (strcmp(arg, "@x") == 0) {
				void *stat = hexmode(1, (int)reg[0], &argc, &argv);
				*argcp = argc;
				*argvp = argv;
				return stat;
			}
		} else if (arg[0] == '[') {
			submode(&fmode, regptr, &argc, &argv);
			regptr++;
			if (regptr == &reg[2]) {
				goto binop;
			}
		} else if (arg[0] == ']') {
			char *ret = malloc(sizeof(void*) + sizeof(int));
			*((void**)ret) = &fmode;
			*((double*)(ret + sizeof(void*))) = reg[0];
			*argcp = argc;
			*argvp = argv;
			return ret;
		} else {
			op = getfop(arg);
			if (op.op == NULL) {
				fprintf(stderr, "Invalid operation %s\n", arg);
				return NULL;
			}
			if (op.optype == optype_u && regptr == &reg[1]) {
				operation_fu fptr = op.op;
				reg[0] = fptr(reg[0]);
				regptr = &reg[1];
			}
		}
	}

	printf("%f\n", reg[0]);

	return 0;
}

void *hexmode(int init, int initval, int *argcp, char ***argvp) {
	int argc = *argcp;
	char **argv = *argvp;
	int reg = 0;

	if (init) {
		reg = initval;
	}

	while (argc > 0) {
		char *arg = *argv;
		argv++;
		argc--;


		if (arg[0] == '@') {
			if (strcmp(arg, "@i") == 0) {
				void *stat = imode(1, reg, &argc, &argv);
				*argcp = argc;
				*argvp = argv;
				return stat;
			} else if (strcmp(arg, "@f") == 0) {
				void *stat = fmode(1, (double)reg, &argc, &argv);
				*argcp = argc;
				*argvp = argv;
				return stat;
			}
		} else if (arg[0] == '[') {
			submode(&hexmode, &reg, &argc, &argv);
		} else if (arg[0] == ']') {
			char *ret = malloc(sizeof(void*) + sizeof(int));
			*((void**)ret) = (void*)&hexmode;
			*((int*)(ret + sizeof(void*))) = reg;
			*argcp = argc;
			*argvp = argv;
			return ret;
		} else {
			if (arg[0] == '#') {
				arg++;
			}
			sscanf(arg, "%x", &reg);
		}
	}

	printf("%X\n", reg);

	return 0;
}

void *submode(void *caller, void *ret, int *argcp, char ***argvp) {
	int argc = *argcp;
	char **argv = *argvp;

	void *returned = NULL;

	char *arg = *argv;

	if (arg[0] == '@') {
		argc--;
		argv++;
		if (strcmp(arg, "@i") == 0) {
			returned = imode(0, 0, &argc, &argv);
		} else if (strcmp(arg, "@x") == 0) {
			returned = hexmode(0, 0, &argc, &argv);
		} else {
			returned = fmode(0, 0, &argc, &argv);
		}
	} else {
		returned = fmode(0, 0, &argc, &argv);
	}

	void *returner = *(void**)returned;
	void *retval = (returned + sizeof(void*));

	if (caller == &imode || caller == &hexmode) {
		int *rp = (int*)ret;
		if (returner == &imode || returner == &hexmode) {
			*rp = *(int*)retval;
		} else if (returner == &fmode) {
			*rp = (int)(*(double*)retval);
		}
	} else if (caller == &fmode) {
		double *rp = (double*)ret;
		if (returner == &imode || returned == &hexmode) {
			*rp = (double)(*(int*)retval);
		} else if (returner == &fmode) {
			*rp = *(double*)retval;
		}
	}

	free(returned);

	*argcp = argc;
	*argvp = argv;
	return ret_ok;
}

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
		} else {
			return fmode(0, 0, &argc, &argv) == NULL;
		}
	}

	return fmode(0, 0, &argc, &argv) == NULL;
}
