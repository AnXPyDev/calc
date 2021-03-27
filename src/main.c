#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

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

int imode(int, int, int, char **);
int fmode(int, double, int, char **);

int imode(int init, int initval, int argc, char **argv) {
	int reg[2];
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
			*regptr = atof(arg);
			regptr++;
			if (regptr == &reg[1] + 1) {
				if (op.optype == optype_b && op.op != NULL) {
					operation_ib fptr = op.op;
					reg[0] = fptr(reg[0], reg[1]);
					regptr = &reg[1];
				} else {
					fprintf(stderr, "No operation loaded\n");
					return 1;
				}
			} else {
				if (op.optype == optype_u && op.op != NULL) {
					operation_iu fptr = op.op;
					reg[0] = fptr(reg[0]);
					regptr = &reg[1];
				}
			}
		} else if (arg[0] == '@') {
			if (strcmp(arg, "@f") == 0) {
				return fmode(1, (float)reg[0], argc, argv);
			}
		} else {
			op = getiop(arg);
			if (op.op == NULL) {
				fprintf(stderr, "Invalid operation %s\n", arg);
				return 1;
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

int fmode(int init, double initval, int argc, char **argv) {
	double reg[2];
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
		if (arg[0] >= 48 && arg[0] <= 57) {
			*regptr = atof(arg);
			regptr++;
			if (regptr == &reg[1] + 1) {
				if (op.optype == optype_b && op.op != NULL) {
					operation_fb fptr = op.op;
					reg[0] = fptr(reg[0], reg[1]);
					regptr = &reg[1];
				} else {
					fprintf(stderr, "No operation loaded\n");
					return 1;
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
				return imode(1, (int)reg[0], argc, argv);
			}
		} else {
			op = getfop(arg);
			if (op.op == NULL) {
				fprintf(stderr, "Invalid operation %s\n", arg);
				return 1;
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

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "No arguments provided");
		return 1;
	}

	
	if (argv[1][0] == '@') {
		if (strcmp(argv[1], "@i") == 0) {
			return imode(0, 0, argc - 2, argv + 2);
		} else {
			return fmode(0, 0, argc - 2, argv + 2);
		}
	}

	return fmode(0, 0, argc - 1, argv + 1);
}
