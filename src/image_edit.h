#ifndef SRC_EDIT_H
#define SRC_EDIT_H

#include "structures.h"

/**
 * All of the following functions modify the given image.
 * Refer to the project description for sample outputs.
 */

void rotate_counterclockwise(image_t *img);
void rotate_clockwise(image_t *img);

void mirror_horizontal(image_t *img);
void mirror_vertical(image_t *img);

void resize(image_t *img, int new_width, int new_height);

#endif  // SRC_EDIT_H
