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
		} else if (returner == &colormode) {
			struct colormode_sub_return *creturned = retval;
			if (creturned->type == colormode_sub_return_f) {
				*rp = (int)floor(creturned->val.f * 255.f + 0.49);
			} else {
				fprintf(stderr, "Cannot take color from submode into imode/hexmode at \"%s\" %i\n", **argvp, *argcp);
			}
		}
	} else if (caller == &fmode) {
		double *rp = (double*)ret;
		if (returner == &imode || returned == &hexmode) {
			*rp = (double)(*(int*)retval);
		} else if (returner == &fmode) {
			*rp = *(double*)retval;
		} else if (returner == &colormode) {
			struct colormode_sub_return *creturned = retval;
			if (creturned->type == colormode_sub_return_f) {
				*rp = creturned->val.f;
			} else {
				fprintf(stderr, "Cannot take color from submode into fmode at \"%s\" %i\n", **argvp, *argcp);
			}
		}
	} else if (caller == &colormode) {
		struct colormode_sub_return *cret = ret;
		if (returner == &colormode) {
			struct colormode_sub_return *creturned = retval;
			memcpy(cret, creturned, sizeof(struct colormode_sub_return));
		} else if (returner == &fmode) {
			cret->type = colormode_sub_return_f;
			cret->val.f = *(double*)retval;
		} else if (returner == &imode || returner == &hexmode) {
			cret->type = colormode_sub_return_f;
			cret->val.f = (double)(*(int*)retval) / 255.f;
		}
	}

	free(returned);

	*argcp = argc;
	*argvp = argv;
	return ret_ok;
}
