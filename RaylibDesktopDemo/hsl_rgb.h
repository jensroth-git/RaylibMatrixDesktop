#pragma once
#include <vector>

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

typedef struct rgb {
    float r, g, b;
} RGB;

typedef struct hsl {
    float h, s, l;
} HSL;

/*
 * Converts an RGB color value to HSL. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
 * Assumes r, g, and b are contained in the set [0, 255] and
 * returns HSL in the set [0, 1].
 */
HSL rgb2hsl(float r, float g, float b) {

    HSL result;

    r /= 255;
    g /= 255;
    b /= 255;

    float max = MAX(MAX(r, g), b);
    float min = MIN(MIN(r, g), b);

    result.h = result.s = result.l = (max + min) / 2;

    if (max == min) {
        result.h = result.s = 0; // achromatic
    }
    else {
        float d = max - min;
        result.s = (result.l > 0.5) ? d / (2 - max - min) : d / (max + min);

        if (max == r) {
            result.h = (g - b) / d + (g < b ? 6 : 0);
        }
        else if (max == g) {
            result.h = (b - r) / d + 2;
        }
        else if (max == b) {
            result.h = (r - g) / d + 4;
        }

        result.h /= 6;
    }

    return result;

}

////////////////////////////////////////////////////////////////////////

/*
 * Converts an HUE to r, g or b.
 * returns float in the set [0, 1].
 */
float hue2rgb(float p, float q, float t) {

    if (t < 0)
        t += 1;
    if (t > 1)
        t -= 1;
    if (t < 1. / 6)
        return p + (q - p) * 6 * t;
    if (t < 1. / 2)
        return q;
    if (t < 2. / 3)
        return p + (q - p) * (2. / 3 - t) * 6;

    return p;

}

////////////////////////////////////////////////////////////////////////

/*
 * Converts an HSL color value to RGB. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
 * Assumes h, s, and l are contained in the set [0, 1] and
 * returns RGB in the set [0, 255].
 */
RGB hsl2rgb(float h, float s, float l) {

    RGB result;

    if (0 == s) {
        result.r = result.g = result.b = l; // achromatic
    }
    else {
        float q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        float p = 2 * l - q;
        result.r = hue2rgb(p, q, h + 1. / 3) * 255;
        result.g = hue2rgb(p, q, h) * 255;
        result.b = hue2rgb(p, q, h - 1. / 3) * 255;
    }

    return result;

}

struct GradientStop {
	float at;
	HSL color;
};


// Linear interpolation helper function
float lerpFloat(float a, float b, float t) {
    return a + t * (b - a);
}

// Function to interpolate HSL colors and convert to RGB
RGB lerpHSL(const std::vector<GradientStop>& palette, float alpha) {
    if (palette.empty()) return { 0, 0, 0 };

    // Clamp alpha to [0,1]
    alpha = std::clamp(alpha, 0.0f, 1.0f);

    // Find the surrounding stops
    int lowerIdx = 0, upperIdx = palette.size() - 1;
    for (size_t i = 0; i < palette.size() - 1; ++i) {
        if (alpha >= palette[i].at && alpha <= palette[i + 1].at) {
            lowerIdx = i;
            upperIdx = i + 1;
            break;
        }
    }

    // If alpha is outside the range, return the closest color
    if (lowerIdx == upperIdx) {
        return hsl2rgb(palette[lowerIdx].color.h, palette[lowerIdx].color.s, palette[lowerIdx].color.l);
    }

    // Compute interpolation factor
    float t1 = palette[lowerIdx].at;
    float t2 = palette[upperIdx].at;
    float t = (alpha - t1) / (t2 - t1);

    // Interpolate HSL values
    float h = lerpFloat(palette[lowerIdx].color.h, palette[upperIdx].color.h, t);
    float s = lerpFloat(palette[lowerIdx].color.s, palette[upperIdx].color.s, t);
    float l = lerpFloat(palette[lowerIdx].color.l, palette[upperIdx].color.l, t);

    // Convert interpolated HSL to RGB
    return hsl2rgb(h, s, l);
}

