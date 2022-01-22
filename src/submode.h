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
		} else if (strcmp(arg, "@rgb") == 0 || strcmp(arg, "@rgba") == 0 || strcmp(arg, "@hsl") == 0 || strcmp(arg, "@hsla") == 0 || strcmp(arg, "@c") == 0) {
			returned = dispatchcolormode(arg, &argc, &argv);
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
	} else if (caller == &fmode || caller == &colormode) {
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
