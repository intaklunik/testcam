#ifndef TESTCAM_DRAW_H
#define TESTCAM_DRAW_H

int testcam_init_draw(const char *fontname, unsigned int pixelformat);
void testcam_draw_string(const char *string, char *position, unsigned int image_w);
void testcam_draw_bg(unsigned int buffer_size, char *buffer);

#endif //TESTCAM_DRAW_H

