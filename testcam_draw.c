#include <linux/string.h>
#include <linux/font.h>
#include "testcam_draw.h"
#include "testcam.h"

struct testcam_color {
	char data[32];
};

struct testcam_draw_desc {
	unsigned int bytesperpixel;
	const struct font_desc *font;
	void (*to_format)(struct testcam_color *color, char r, char g, char b);
};

static struct testcam_draw_desc draw_desc;

static int testcam_get_position(unsigned int image_w, unsigned int string_w)
{
	static int pos = 0;
	static int direction = 1;
	static const int step = 2;

	if (pos + step > image_w - string_w) {
		direction = -1;
	}
	else if (pos - step < 0) {
		direction = 1;
	}

	pos += direction * step;

	return pos;
}

static void to_rgb565(struct testcam_color *color, char r, char g, char b)
{
	uint16_t clr = ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3);
	color->data[0] = clr >> 1;
	color->data[1] = clr << 1;
}

static bool testcam_is_font_pixel_on(char symbol, int x, int y)
{
	const unsigned char *data = draw_desc.font->data;
	unsigned int bytesperline = draw_desc.font->width / 8;
	unsigned int symbol_offset = symbol * draw_desc.font->height * bytesperline;

	unsigned int byteinline = x / 8;
	char bitinline = x % 8;

	return data[symbol_offset + bytesperline * y + byteinline] & (1 << (7 - bitinline));
}

static inline void testcam_draw_pixel(struct testcam_color *color, char *addr)
{
	for (int i = 0; i < draw_desc.bytesperpixel; i++) {
		addr[i] = color->data[i];
	}
}

static void testcam_draw_symbol(struct testcam_color *color, char symbol, char *position,  unsigned int bytesperline)
{
	unsigned int width = draw_desc.font->width;
	unsigned int height = draw_desc.font->height;

	for(int i = 0; i < height; i++) {
		for(int j = 0; j < width; j++) {
			if (testcam_is_font_pixel_on(symbol, j, i)) {
				testcam_draw_pixel(color, position + draw_desc.bytesperpixel * j);
			}
		}
		position += bytesperline;
	}
}

void testcam_draw_string(const char *string, char *position, unsigned int image_w)
{
	unsigned int offset = draw_desc.font->width * draw_desc.bytesperpixel;
	struct testcam_color color;
	draw_desc.to_format(&color, 100, 100, 100);

	position += testcam_get_position(image_w * draw_desc.bytesperpixel, strlen(string) * draw_desc.font->width * draw_desc.bytesperpixel);

	for(int i = 0; string[i] != 0; i++) {
		testcam_draw_symbol(&color, string[i], position + offset * i, image_w * draw_desc.bytesperpixel);
	}
}

void testcam_draw_bg(unsigned int buffer_size, char *buffer)
{
	unsigned int limit = buffer_size - draw_desc.bytesperpixel + 1;
	struct testcam_color color;

	draw_desc.to_format(&color, 255, 0, 255);
	
	for(int i = 0; i < limit; i += draw_desc.bytesperpixel) {
		testcam_draw_pixel(&color, &buffer[i]);
	}
}

int testcam_init_draw(const char *fontname, unsigned int pixelformat)
{
	draw_desc.font = find_font(fontname);
	if (draw_desc.font == NULL) {
		TESTCAM_LOG("find_font failed");
		return -EINVAL;
	}

	if (pixelformat != V4L2_PIX_FMT_RGB565) {
		return -EINVAL;
	}

	draw_desc.bytesperpixel = 2;
	draw_desc.to_format = to_rgb565;

	return 0;
}
