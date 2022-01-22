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
