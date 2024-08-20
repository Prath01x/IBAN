#ifndef SRC_FLOOD_H
#define SRC_FLOOD_H

#include "structures.h"

/*
 * The 4-directional flood-fill algorithm.
 */
void flood(image_t *img, int x, int y, pixel_t *target_color);

#endif  // SRC_FLOOD_H
