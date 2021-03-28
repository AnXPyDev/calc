#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "color.h"

struct RGBA {
	double r, g, b, a;
};

struct RGB {
	double r, g, b;
};

struct HSLA {
	double h, s, l, a;
};

struct HSL {
	double h, s, l;
};

double *maxrgb(struct RGB *color) {
	return color->r > color->g ? (color->r > color->b ? &color->r : &color->b) : (color->g > color->b ? &color->g : &color->b);
}

double *minrgb(struct RGB *color) {
	return color->r < color->g ? (color->r < color->b ? &color->r : &color->b) : (color->g < color->b ? &color->g : &color->b);
}

struct HSL rgbtohsl(struct RGB color) {
	struct HSL ret;
	double *cmin = minrgb(&color);
	double *cmax = maxrgb(&color);
	double delta = *cmax - *cmin;

	printf("%f %f\n", *cmax, *cmin);

	ret.l = (*cmax + *cmin) / 2;

	if (delta < 0.0001) {
		ret.h = 0;
		ret.s = 0;
	} else {
		if (cmax == &color.r) {
			ret.h = fmod((color.g - color.b) / delta, 6);
		} else if (cmax == &color.g) {
			ret.h = ((color.b - color.r) / delta) + 2;
		} else {
			ret.h = ((color.r - color.g) / delta) + 4;
		}

		ret.s = delta/(1 - fabs((*cmax + *cmin) - 1));
	}
	ret.h = ret.h / 6;
	if (ret.h < 0) {
		ret.h += 1;
	}
	return ret;
}

struct RGB hsltorgb(struct HSL color) {
	struct RGB ret;
	double C = (1 - fabs(2 * color.l - 1)) * color.s;
	double X = C * (1 - fabs(fmod(color.h, 2) - 1));
	double m = color.l - (C/2);

	double R, G, B;
	double os = (double)1/6;

	double H = color.h * 360;

	printf("cxmh %f %f %f %f\n", C, X, m, H);

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

	ret.r = R + m;
	ret.g = G + m;
	ret.b = B + m;

	return ret;
}

double clamp(double a, double min, double max) {
	if (a < min) {
		return min;
	} else if (a > max) {
		return max;
	}
	return a;
}

double wrap(double a, double min, double max) {
	if (a < min) {
		return a + (max - min);
	} else if (a > max) {
		return a - (max - min);
	}	
	return a;
}

struct RGB rgbclamp(struct RGB color) {
	color.r = clamp(color.r, 0, 1);
	color.g = clamp(color.g, 0, 1);
	color.b = clamp(color.b, 0, 1);
	return color;
}

struct HSL hslclamp(struct HSL color) {
	color.h = wrap(color.h, 0, 1);
	color.s = clamp(color.s, 0, 1);
	color.l = clamp(color.l, 0, 1);
	return color;
}