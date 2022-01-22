double opfadd(double a, double b) { return a + b; }
double opfsub(double a, double b) { return a - b; }
double opfmul(double a, double b) { return a * b; }
double opfdiv(double a, double b) { return a / b; }
double opfpow(double a, double b) { return pow(a, b); }
double opfset(double a, double b) { return b; }

double opffloor(double a) { return floor(a); };
double opfceil(double a) { return floor(a + 1); };
double opfround(double a) { return floor(a + 0.5); };
double opfnegate(double a) { return 0 - a; };

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
	} else if (strcmp(opname, "=") == 0) {
		ret.optype = optype_b;
		ret.op = &opfset;
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
					fprintf(stderr, "No operation loaded at \"%s\" %i\n", arg, argc);
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
			char *ret = malloc(sizeof(void*) + sizeof(double));
			*((void**)ret) = &fmode;
			*((double*)(ret + sizeof(void*))) = reg[0];
			*argcp = argc;
			*argvp = argv;
			return ret;
		} else {
			op = getfop(arg);
			if (op.op == NULL) {
				fprintf(stderr, "Invalid operation at \"%s\" %i\n", arg, argc);
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
