#ifndef SRC_IO_H
#define SRC_IO_H

#include <stdio.h>

#include "structures.h"

/**
 * Allocate and read the .ppm image from the given input file stream.
 * Return 0 on success, -1 if the input file is in a wrong format.
 */
int image_read(image_t *img, FILE *fin);

/**
 * Write the given image to the output file stream.
 */
void image_write(const image_t *img, FILE *fout);

/**
 * Free allocated memory.
 */
void image_free(image_t *img);

#endif  // SRC_IO_H
