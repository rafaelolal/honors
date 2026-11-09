#include "bmp.h"
#include <assert.h>
#include <stdlib.h>

static void put16(uint8_t* p, uint16_t v) {
  p[0] = v;
  p[1] = v >> 8;
}

static void put32(uint8_t* p, uint32_t v) {
  for (int i = 0; i < 4; i++)
    p[i] = v >> (8 * i);
}

void bmp_open(BmpWriter* b, const char* path, int width, int height) {
  assert(width > 0 && height > 0);
  b->width = width;
  b->row_bytes = ((size_t)width * 3 + 3) & ~(size_t)3;
  uint64_t image = (uint64_t)b->row_bytes * height;
  uint64_t file = 54 + image;
  // sizes are 32-bit fields in the header
  if (file > UINT32_MAX) {
    printf("bitmap %s would be %llu bytes, over BMP's 4GB limit\n", path,
           (unsigned long long)file);
    exit(1);
  }

  b->f = fopen(path, "wb");
  if (!b->f) {
    printf("cannot open file %s\n", path);
    exit(1);
  }
  // writes are a few KB each; a big buffer cuts syscalls
  setvbuf(b->f, NULL, _IOFBF, 1 << 22);

  uint8_t h[54] = {'B', 'M'};
  put32(h + 2, (uint32_t)file);
  put32(h + 10, 54); // pixel data offset
  put32(h + 14, 40); // BITMAPINFOHEADER size
  put32(h + 18, (uint32_t)width);
  put32(h + 22, (uint32_t)height); // positive: rows stored bottom-up
  put16(h + 26, 1);                // planes
  put16(h + 28, 24);               // bits per pixel
  put32(h + 34, (uint32_t)image);  // compression (30) stays 0 = none
  fwrite(h, 1, sizeof h, b->f);

  // calloc so the padding bytes are always zero
  b->row = (uint8_t*)calloc(b->row_bytes, 1);
  assert(b->row);
}

void bmp_write_row(BmpWriter* b, const uint8_t* rgb) {
  for (int c = 0; c < b->width; c++) {
    b->row[3 * c] = rgb[3 * c + 2];
    b->row[3 * c + 1] = rgb[3 * c + 1];
    b->row[3 * c + 2] = rgb[3 * c];
  }
  fwrite(b->row, 1, b->row_bytes, b->f);
}

void bmp_close(BmpWriter* b) {
  fclose(b->f);
  free(b->row);
}
