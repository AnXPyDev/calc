int opiadd(int a, int b) { return a + b; }
int opisub(int a, int b) { return a - b; }
int opimul(int a, int b) { return a * b; }
int opidiv(int a, int b) { return a / b; }
int opipow(int a, int b) { return pow(a, b); }
int opinegate(int a) { return 0 - a; };

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
