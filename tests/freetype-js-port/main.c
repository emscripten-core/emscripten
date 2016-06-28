#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#define MAX_FACES	10

static FT_Library	library;
static FT_Face		faces[MAX_FACES];	
static char*		names[MAX_FACES];	
static int			mults[MAX_FACES] = { 1 }; //set to 1 for default	
static FT_GlyphSlot	slot;

static unsigned char*	monomap;		// monochrome bitmap
static int				monomode;		// monochrome mode

// Unpack a monochrome bitmap into anti-aliased bitmap form
static int mono_unpack(FT_Bitmap* bitmap) {
	if (monomap)
		free(monomap);
	monomap = (unsigned char*)malloc(bitmap->width * bitmap->rows);
	if (!monomap)
		return -1;
	unsigned char* bytes = monomap;
	for (int y = 0; y < bitmap->rows; y++) {
		unsigned char* bits  = &bitmap->buffer[y * bitmap->pitch];
		for (int x = 0; x < bitmap->width; x++) {
			*bytes++ = (bits[x >> 3] & (1 << (7 - (x & 07)))) ? 255 : 0;
		}
	}
	return 0;
}

int monochrome(int enable) {
	monomode = enable ? 1 : 0;
	return 0;
}

// The multiplier allows globally adjusting the font size by mult-%.
int load_font(const char* path, const char* name, int mult) {
	FT_Error error;
	int i;

	// First look for a font with the same name
	for (i = 0; i < MAX_FACES; i++) {
		if (names[i] && strcasecmp(names[i], name) == 0) {
			FT_Done_Face(faces[i]);
			free(names[i]);
			names[i] = 0;
			faces[i] = 0;
			break;
		}
	}
	if (i == MAX_FACES) {
		for (i = 0; i < MAX_FACES && names[i]; i++)
			;
	}
	if (i == MAX_FACES) {
		printf("load_font(%s,%s): too many fonts!\n", path, name);
		return -1;
	}

	error = FT_New_Face(library, path, 0, &faces[i]);
	if (error) {
		printf("New_Face(%s,%s) Error! %d\n", path, name, error);
		return error;
	}
	names[i] = strdup(name);
	mults[i] = mult;
	return 0;
}

int close_font(const char* name) {
	for (int i = 0; i < MAX_FACES; i++) {
		if (names[i] && strcasecmp(names[i], name) == 0) {
			FT_Done_Face(faces[i]);
			free(names[i]);
			names[i] = 0;
			faces[i] = 0;
			break;
		}
	}
	return 0;
}

int find_font(const char* name) {
	for (int i = 0; i < MAX_FACES && names[i]; i++) {
		if (strcasecmp(name, names[i]) == 0) {
			return i;
		}
	}
	// If not found, default is 0
	return 0;
}

unsigned char* get_bitmap(int font, int ch, int width, int height) {
	FT_Error	error;
	FT_Face		face;

	if (font < 0 || font >= MAX_FACES || !names[font])
		font = 0;	// Default

	face   = faces[font];
	width  = (width * mults[font] * 64) / 100;
	height = (height * mults[font] * 64) / 100;

	
	/* 1pt == 1px == 72dpi */
	error = FT_Set_Char_Size(face, width, height, 72, 0 );
	if (error) {
		printf("Set_Char_Size Error! %d\n", error);
		return 0;
	}
	slot = face->glyph;

	error = FT_Load_Char(face, ch, FT_LOAD_RENDER);
	if (error) {
		printf("Load_Char Error! %d\n", error);
		return 0;
	}

	if (monomode) {
		if (mono_unpack(&slot->bitmap) != 0)
			return 0;
		return monomap;
	}

	return slot->bitmap.buffer;
}

unsigned get_left() {
	return slot->bitmap_left;
}
unsigned get_top() {
	return slot->bitmap_top;
}
unsigned get_width() {
	return slot->bitmap.width;
}
unsigned get_height() {
	return slot->bitmap.rows;
}
unsigned get_pitch() {
	return slot->bitmap.pitch;
}
unsigned get_advance() {
	return slot->advance.x >> 6;
}

int main() {
	FT_Error	  error;

	error = FT_Init_FreeType(&library);
	if (error) {
		printf("Init Error! %d\n", error);
		return 1;
	}

	return 0;
}
