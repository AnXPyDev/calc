#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "hsluv.c"

enum ColorType {
	RGBA, HSLA, RGB, HSL, HSLUV
};

typedef struct {
	double val[4];
} ColorValue;

typedef struct {
	enum ColorType type;
	ColorValue val;
} Color;

ColorValue color_val_zero = { {0.f,0.f,0.f,1.f } };
Color color_zero = {RGBA, { {0.f,0.f,0.f,1.f } }};

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

double huetorgb(double v1, double v2, double vH){
	if (vH < 0)
		vH += 1;

	if (vH > 1)
		vH -= 1;

	if ((6 * vH) < 1)
		return (v1 + (v2 - v1) * 6 * vH);

	if ((2 * vH) < 1)
		return v2;

	if ((3 * vH) < 2)
		return (v1 + (v2 - v1) * ((2.0f / 3) - vH) * 6);

	return v1;
}



ColorValue hsltorgb(ColorValue color) {
	ColorValue ret;

	ret.val[3] = color.val[3];

	if (color.val[1] == 0) {
		ret.val[0] = ret.val[1] = ret.val[2] = color.val[2];
		return ret;
	}

	float v1, v2;

	v2 = (color.val[2] < 0.5) ? (color.val[2] * (1 + color.val[1])) : ((color.val[2] + color.val[1]) - (color.val[2] * color.val[1]));
	v1 = 2 * color.val[2] - v2;

	ret.val[0] = huetorgb(v1, v2, color.val[0] + (1.0f / 3));
	ret.val[1] = huetorgb(v1, v2, color.val[0]);
	ret.val[2] = huetorgb(v1, v2, color.val[0] - (1.0f / 3));

	return ret;
	/* double C = (1 - fabs(2 * color.val[2] - 1)) * color.val[1];
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
	ret.val[3] = color.val[3]; */

	return ret;
}

ColorValue rgbtohsluv(ColorValue color) {
	ColorValue ret;
	ret.val[3] = color.val[3];
	rgb2hsluv(color.val[0], color.val[1], color.val[2], &ret.val[0], &ret.val[1], &ret.val[2]);
	ret.val[0] /= 360.0;
	ret.val[1] /= 100.0;
	ret.val[2] /= 100.0;
	//fprintf(stderr, "rgbtohsluv %lf %lf %lf %lf %lf %lf\n", color.val[0], color.val[1], color.val[2], ret.val[0], ret.val[1], ret.val[2]);
	return ret;
}

ColorValue hsluvtorgb(ColorValue color) {
	ColorValue ret;
	ret.val[3] = color.val[3];
	hsluv2rgb(color.val[0] * 360.0, color.val[1] * 100.0, color.val[2] * 100.0, &ret.val[0], &ret.val[1], &ret.val[2]);
	//fprintf(stderr, "hsluvtorgb %lf %lf %lf %lf %lf %lf\n", color.val[0], color.val[1], color.val[2], ret.val[0], ret.val[1], ret.val[2]);
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
	if (type == HSLA || type == HSL || type == HSLUV) {
		clr->val[0] = wrap(clr->val[0], 0.f, 1.f);
	}

	clr->val[0] = clamp(clr->val[0], 0.f, 1.f);
	clr->val[1] = clamp(clr->val[1], 0.f, 1.f);
	clr->val[2] = clamp(clr->val[2], 0.f, 1.f);
	clr->val[3] = clamp(clr->val[3], 0.f, 1.f);
}

// From webkit
ColorValue applyHueRotateFilter(ColorValue color, double amount) {
	ColorValue ret;
	float pi = 3.14159265358979323846;
	float c = cos(amount * 2 * pi);
	float s = sin(amount * 2 * pi);
	ret.val[0] = color.val[0] * (0.213 + c * 0.787 - s * 0.213) + color.val[1] * (0.715 - c * 0.715 - s * 0.715) + color.val[2] * (0.072 - c * 0.072 + s * 0.928);
	ret.val[1] = color.val[0] * (0.213 - c * 0.213 + s * 0.143) + color.val[1] * (0.715 + c * 0.285 + s * 0.140) + color.val[2] * (0.072 - c * 0.072 - s * 0.283);
	ret.val[2] = color.val[0] * (0.213 - c * 0.213 - s * 0.787) +  color.val[1] * (0.715 - c * 0.715 + s * 0.715) + color.val[2] * (0.072 + c * 0.928 + s * 0.072);
	ret.val[3] = color.val[3];
	return ret;
}


// From inkscape
/*int pxclamp(int v, int min, int max) {
	if (v < min) {
		return min;
	} else if (v > max) {
		return max;
	}
	return v;
}

ColorValue applyHueRotateFilter(ColorValue color, double amount) {

	double sinhue = sin(amount * M_PI * 2);
	double coshue = cos(amount * M_PI * 2);

	int mat[9];

	mat[0] = round((0.213 +0.787*coshue -0.213*sinhue)*255);
	mat[1] = round((0.715 -0.715*coshue -0.715*sinhue)*255);
	mat[2] = round((0.072 -0.072*coshue +0.928*sinhue)*255);

	mat[3] = round((0.213 -0.213*coshue +0.143*sinhue)*255);
	mat[4] = round((0.715 +0.285*coshue +0.140*sinhue)*255);
	mat[5] = round((0.072 -0.072*coshue -0.283*sinhue)*255);

	mat[6] = round((0.213 -0.213*coshue -0.787*sinhue)*255);
	mat[7] = round((0.715 -0.715*coshue +0.715*sinhue)*255);
	mat[8] = round((0.072 +0.928*coshue +0.072*sinhue)*255);

	int r = color.val[0] * 255;
	int g = color.val[1] * 255;
	int b = color.val[2] * 255;
	int a = color.val[3] * 255;

	int maxpx = a * 255;

	int ro = r * mat[0] + g * mat[1] + b * mat[2];
	int go = r * mat[3] + g * mat[4] + b * mat[5];
	int bo = r * mat[6] + g * mat[7] + b * mat[8];
	ro = (pxclamp(ro, 0, maxpx) + 127) / 255;
	go = (pxclamp(go, 0, maxpx) + 127) / 255;
	bo = (pxclamp(bo, 0, maxpx) + 127) / 255;


	ColorValue ret;
	ret.val[0] = (double)ro / 255;
	ret.val[1] = (double)go / 255;
	ret.val[2] = (double)bo / 255;
	ret.val[3] = color.val[3];
	return ret;
}*/


