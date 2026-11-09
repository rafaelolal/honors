#ifndef __bmp_h
#define __bmp_h

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// Streams a 24-bit BMP to disk one row at a time, so the image is never held
// in memory. BMP stores rows bottom-up: write the last row first.
typedef struct {
  FILE* f;
  int width;
  size_t row_bytes; // 3 bytes per pixel, padded to a multiple of 4
  uint8_t* row;     // BGR row being written
} BmpWriter;

// exits if the file can't be opened or the image is over the format's 4GB
void bmp_open(BmpWriter* b, const char* path, int width, int height);

// rgb: width * 3 bytes, in R, G, B order
void bmp_write_row(BmpWriter* b, const uint8_t* rgb);

void bmp_close(BmpWriter* b);

#endif
