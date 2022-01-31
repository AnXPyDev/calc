enum color_optype_e {
	color_optype_err, color_optype_cc, color_optype_ccf, color_optype_ef, color_optype_cf
};

struct color_operation {
	enum color_optype_e color_optype;
	enum optype_e optype;
	void *op;
};

struct color_operation color_op_zero;

ColorValue color_opcc(ColorValue a, ColorValue b, operation_fb op) {
	ColorValue ret;
	
	ret.val[0] = op(a.val[0], b.val[0]);
	ret.val[1] = op(a.val[1], b.val[1]);
	ret.val[2] = op(a.val[2], b.val[2]);
	ret.val[3] = op(a.val[3], b.val[3]);

	return ret;
}

ColorValue color_opcf(ColorValue a, double f, operation_fb op) {
	ColorValue ret;

	ret.val[0] = op(a.val[0], f);
	ret.val[1] = op(a.val[1], f);
	ret.val[2] = op(a.val[2], f);
	ret.val[3] = op(a.val[3], f);

	return ret;
}

ColorValue color_opblend(ColorValue a, ColorValue b, double f) {
	ColorValue ret;

	ret.val[0] = lerp(a.val[0], b.val[0], f);
	ret.val[1] = lerp(a.val[1], b.val[1], f);
	ret.val[2] = lerp(a.val[2], b.val[2], f);
	ret.val[3] = lerp(a.val[3], b.val[3], f);

	return ret;
	
}

typedef ColorValue (*color_operation_ccf)(ColorValue, ColorValue, double);

struct color_operation getcop(char *opname) {
	struct color_operation ret = {0, 0, NULL};
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
	} else if (strcmp(opname, "blend") == 0) {
		ret.color_optype = color_optype_ccf;
		ret.op = color_opblend;
	}
	return ret;
}


int color_get_element_index(char *str) {
	if (str[1] == 'h' || str[1] == 'r') {
		return 0;
	} else if (str[1] == 's' || str[1] == 'g') {
		return 1;
	} else if (str[1] == 'l' || str[1] == 'b') {
		return 2;
	} else if (str[1] == 'a') {
		return 3;
	} else if (str[1] == '0') {
		return 4;
	}
	return -1;
}

struct colormode_sub_return {
	enum { colormode_sub_return_f, colormode_sub_return_cv } type;
	union {
		double f;
		ColorValue cv;
	} val;
};


void *colormode(int init, Color initcolor, int *argcp, char ***argvp) {
	int argc = *argcp;
	char **argv = *argvp;
	
	enum ColorType color_type = RGBA;
	ColorValue creg[2] = { color_val_zero, color_val_zero };
	double freg = 0.f;

	ColorValue *cregptr = &creg[0];

	if (init == 1) {
		cregptr++;
		creg[0] = initcolor.val;
	}
	
	if (init) {
		color_type = initcolor.type;
	}

	struct color_operation op = {0, 0, NULL};

	double *element = NULL;

	while (argc > 0) {
		char *arg = *argv;
		argv++;
		argc--;
		
		if (arg[0] >= '0' && arg[0] <= '9') {
			freg = atof(arg);
			
			freg_loaded:;
			if (element) {
				if (op.optype == optype_b && op.op != NULL) {
					operation_fb fptr = op.op;
					*element = fptr(*element, freg);
					op = color_op_zero;
					normalizecolor(&creg[0], color_type);
				} else {
					fprintf(stderr, "No suitable operation loaded at \"%s\" %i\n", arg, argc);
					return NULL;
				}
			} else if (cregptr == &creg[2]) {
				if (op.color_optype == color_optype_ccf && op.op != NULL) {
					color_operation_ccf fptr = op.op;
					creg[0] = fptr(creg[0], creg[1], freg);
					cregptr = &creg[1];
					op = color_op_zero;
					normalizecolor(&creg[0], color_type);
				} else {
					fprintf(stderr, "No suitable operation loaded at \"%s\" %i\n", arg, argc);
					return NULL;
				}
			} else {
				if (op.color_optype == color_optype_cf && op.optype == optype_b && op.op != NULL) {
					operation_fb fptr = op.op;
					creg[0] = color_opcf(creg[0], freg, fptr);
					cregptr = &creg[1];
					op = color_op_zero;
					normalizecolor(&creg[0], color_type);
				} else {
					fprintf(stderr, "No suitable operation loaded at \"%s\" %i\n", arg, argc);
					return NULL;
				}

			}

		} else if (arg[0] == '#') {
			if (cregptr == &creg[2]) {
				fprintf(stderr, "Trying to push third value onto register at \"%s\" %i\n", arg, argc);
				return NULL;
			}
			element = NULL;
			*cregptr = colorvalfromhex(arg);
			cregptr++;

			creg_loaded:;
			if (cregptr == &creg[2]) {
				if (op.optype == optype_b && op.op != NULL) {
					operation_fb fptr = op.op;
					creg[0] = color_opcc(creg[0], creg[1], fptr);
					cregptr = &creg[1];
					op = color_op_zero;
					normalizecolor(&creg[0], color_type);
				} else if (op.color_optype == color_optype_ccf) { 
					continue;
				} else {
					fprintf(stderr, "No suitable operation loaded at \"%s\" %i\n", arg, argc);
					return NULL;
				}
			}

		} else if (arg[0] == '@') {
			if (strlen(arg) == 2) {
				int element_index = color_get_element_index(arg);
				if (element_index == 4) {
					element = NULL;
					continue;
				} else if (element_index != -1) {
					element = &(creg[0].val[element_index]);
					continue;
				}
			} 
		
			if (strcmp(arg, "@rgb") == 0) {
				if (color_type == HSL || color_type == HSLA) {
					creg[0] = hsltorgb(creg[0]);
					creg[1] = hsltorgb(creg[1]);
				}
				color_type = RGB;
				normalizecolor(&creg[0], color_type);
				element = NULL;
				continue;
			} else if (strcmp(arg, "@rgba") == 0) {
				if (color_type == HSL || color_type == HSLA) {
					creg[0] = hsltorgb(creg[0]);
					creg[1] = hsltorgb(creg[1]);
				}
				color_type = RGBA;
				normalizecolor(&creg[0], color_type);
				element = NULL;
				continue;
			} else if (strcmp(arg, "@hsl") == 0) {
				if (color_type == RGB || color_type == RGBA) {
					creg[0] = rgbtohsl(creg[0]);
					creg[1] = rgbtohsl(creg[1]);
				}
				color_type = HSL;
				normalizecolor(&creg[0], color_type);
				element = NULL;
				continue;
			} else if (strcmp(arg, "@hsla") == 0) {
				if (color_type == RGB || color_type == RGBA) {
					creg[0] = rgbtohsl(creg[0]);
					creg[1] = rgbtohsl(creg[1]);
				}
				color_type = HSLA;
				normalizecolor(&creg[0], color_type);
				element = NULL;
				continue;
			}

			if (!element) {
				fprintf(stderr, "Cannot switch mode at \"%s\" %i because no element is loaded\n", arg, argc);
				return NULL;
			}
			
			if (strcmp(arg, "@i") == 0) {
				void *stat = imode(1, (int)(floor(*element * 255 + 0.5)), &argc, &argv);
				*argcp = argc;
				*argvp = argv;
				return stat;
			} else if (strcmp(arg, "@x") == 0) {
				void *stat = hexmode(1, (int)(floor(*element * 255 + 0.5)), &argc, &argv);
				*argcp = argc;
				*argvp = argv;
				return stat;
			} else if (strcmp(arg, "@f") == 0) {
				void *stat = fmode(1, *element, &argc, &argv);
				*argcp = argc;
				*argvp = argv;
				return stat;
			}
		} else if (arg[0] == '[') {
			struct colormode_sub_return ret;
			submode(&colormode, &ret, &argc, &argv);
			
			if (ret.type == colormode_sub_return_f) {
				freg = ret.val.f;
				goto freg_loaded;
			} else if (ret.type == colormode_sub_return_cv) {
				if (cregptr == &creg[2]) {
					fprintf(stderr, "Trying to push third value onto register at \"%s\" %i\n", arg, argc);
					return NULL;
				}
				*cregptr = ret.val.cv;
				cregptr++;

				goto creg_loaded;
			}
		} else if (arg[0] == ']') {
			char *ret = malloc(sizeof(void*) + sizeof(struct colormode_sub_return));
			*(void**)ret = &colormode;
			struct colormode_sub_return *cret = (struct colormode_sub_return*)(ret + sizeof(void*));
			if (element) {
				cret->type = colormode_sub_return_f;
				cret->val.f = *element;
			} else {
				cret->type = colormode_sub_return_cv;
				cret->val.cv = creg[0];
			}
			*argcp = argc;
			*argvp = argv;
			return ret;
		} else {
			op = getcop(arg);
			if (op.op == NULL) {
				fprintf(stderr, "Invalid operation at \"%s\" %i\n", arg, argc);
				return NULL;
			}
			if (op.optype == optype_u && element != NULL) {
				operation_fu fptr = op.op;
				*element = fptr(*element);
				normalizecolor(&creg[0], color_type);
			}
		}
	}

	if (element) {
		printf("%f\n", *element);
		return NULL;
	}
	
	char hex[16];
	hexfromcolorval(creg[0], hex);

	if (color_type == RGB || color_type == HSL) {
		hex[6] = '\0';
	}

	printf("#%s\n", hex);

	return NULL;
}


void *dispatchcolormode(char *arg, int *argcp, char ***argvp) {
	if (strcmp(arg, "@c") == 0) {
		return colormode(0, color_zero, argcp, argvp);
	}

	Color initcolor = color_zero;
	if (strcmp(arg, "@rgb") == 0) {
		initcolor.type = RGB;
	} else if (strcmp(arg, "@rgba") == 0) {
		initcolor.type = RGBA;
	} else if (strcmp(arg, "@hsl") == 0) {
		initcolor.type = HSL;
	} else if (strcmp(arg, "@hsla") == 0) {
		initcolor.type = HSLA;
	}

	return colormode(2, initcolor, argcp, argvp);
}


