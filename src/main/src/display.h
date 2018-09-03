#ifndef DISPLAY_H_
#define DISPLAY_H_
#include "encoder.h"

void display_init();
void display_frame(int width, int height, const byte_t* data);

#endif