#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Creates WebGL context on DOM canvas element with ID "canvas". Sets CSS size and render target size to width&height.
void init_webgl(int width, int height);

// WebGL canvas clear
void clear_screen(float r, float g, float b, float a);

// Fills a rectangle (x0,y0) -> (x1,y1) with given solid color rgba. Coordinates are in pixel units.
// Bottom-left corner of screen is (0,0). Top-right corner is (width-1,height-1) pixels.
void fill_solid_rectangle(float x0, float y0, float x1, float y1, float r, float g, float b, float a);

// Draws a single character of text on screen at position (x0,y0). spacing: letter spacing. charSize: font size. shadow: if true, applies a drop shadow.
void fill_char(float x0, float y0, float r, float g, float b, float a, unsigned int ch, int charSize, int shadow);

// Draws a string of text on screen at position (x0,y0). spacing: letter spacing. charSize: font size. shadow: if true, applies a drop shadow.
void fill_text(float x0, float y0, float r, float g, float b, float a, const char *str, float spacing, int charSize, int shadow);

// Draws an image from given url to pixel coordinates x0,y0, applying uniform scaling factor scale, modulated with rgba. 
void fill_image(float x0, float y0, float scale, float r, float g, float b, float a, const char *url);

#ifdef __cplusplus
}
#endif
