#include <stdio.h>
#include <math.h>
#include <stdlib.h>

enum ColorType {
	RGBA, HSLA, RGB, HSL
};

typedef struct {
	double val[4];
} ColorValue;

typedef struct {
	enum ColorType type;
	ColorValue val;
} Color;

ColorValue color_val_zero = { {0,0,0,0 } };
Color color_zero = {RGBA, { {0,0,0,0 } }};

double *maxrgb(ColorValue *color) {
	return color->val[0] > color->val[1] ? (color->val[0] > color->val[2] ? &color->val[0] : &color->val[2]) : (color->val[1] > color->val[2] ? &color->val[1] : &color->val[2]);
}

double *minrgb(ColorValue *color) {
	return color->val[0] < color->val[1] ? (color->val[0] < color->val[2] ? &color->val[0] : &color->val[2]) : (color->val[1] < color->val[2] ? &color->val[1] : &color->val[2]);
}

ColorValue rgbtohsl(ColorValue color) {
	ColorValue ret;
	double *cmin = minrgb(&color);
	double *cmax = maxrgb(&color);
	double delta = *cmax - *cmin;

	ret.val[2] = (*cmax + *cmin) / 2;

	if (delta < 0.0001) {
		ret.val[0] = 0;
		ret.val[1] = 0;
	} else {
		if (cmax == &color.val[0]) {
			ret.val[0] = fmod((color.val[1] - color.val[2]) / delta, 6);
		} else if (cmax == &color.val[1]) {
			ret.val[0] = ((color.val[2] - color.val[0]) / delta) + 2;
		} else {
			ret.val[0] = ((color.val[0] - color.val[1]) / delta) + 4;
		}

		ret.val[1] = delta/(1 - fabs((*cmax + *cmin) - 1));
	}
	ret.val[0] = ret.val[0] / 6;
	if (ret.val[0] < 0) {
		ret.val[0] += 1;
	}
	ret.val[3] = color.val[3];

	return ret;
}

ColorValue hsltorgb(ColorValue color) {
	ColorValue ret;
	double C = (1 - fabs(2 * color.val[2] - 1)) * color.val[1];
	double X = C * (1 - fabs(fmod(color.val[0], 2) - 1));
	double m = color.val[2] - (C/2);

	double R = 0;
	double G = 0;
	double B = 0;

	double H = color.val[0] * 360;

	if (0 <= H && H < 60) {
		R = C; G = X; B = 0;
	} else if (60 <= H && H < 120) {
		R = X; G = C; B = 0;
	} else if (120 <= H && H < 180) {
		R = 0; G = C; B = X;
	} else if (180 <= H && H < 240) {
		R = 0; G = X; B = C;
	} else if (240 <= H && H < 300) {
		R = X; G = 0; B = C;
	} else if (300 <= H && H < 360) {
		R = C; G = 0; B = X;
	} 

	ret.val[0] = R + m;
	ret.val[1] = G + m;
	ret.val[2] = B + m;
	ret.val[3] = color.val[3];

	return ret;
}

ColorValue colorvalfromhex(char *str) {

	unsigned int len = strlen(str);

	char xstr[3] = "00";

	if (str[0] == '#') {
		len--;
		str++;
	}

	len /= 2;

	if ( len > 4 ) {
		len = 4;
	} else if ( len == 0 ) {
		return color_val_zero;
	}
	
	ColorValue ret;
	double *valptr = &ret.val[0];

	for (unsigned int i = 0; i < len; i++) {
		xstr[0] = str[i * 2];
		xstr[1] = str[i * 2 + 1];
		*valptr = (double)strtol(xstr, NULL, 16) / 255.f;
		valptr++;
	}

	if ( len < 4 ) {
		double def = *(valptr - 1);

		while (valptr != &ret.val[3]) {
			*valptr = def;
			valptr++;
		}

		ret.val[3] = 1.f;
	}

	return ret;
}

void hexfromcolorval(ColorValue clr, char *target) {
	for (unsigned int i = 0; i < 4; i++) {
		sprintf(target + i * 2, "%02X", (int)(floor(clr.val[i] * 255.f + 0.499)));
	}
}

void normalizecolor(ColorValue *clr, enum ColorType type) {
	if (type == HSLA || type == HSL) {
		clr->val[0] = wrap(clr->val[0], 0.f, 1.f);
	}

	clr->val[0] = clamp(clr->val[0], 0.f, 1.f);
	clr->val[1] = clamp(clr->val[1], 0.f, 1.f);
	clr->val[2] = clamp(clr->val[2], 0.f, 1.f);
	clr->val[3] = clamp(clr->val[3], 0.f, 1.f);
}





