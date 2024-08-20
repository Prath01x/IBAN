#include "flood_fill.h"
#include <stdbool.h>
#include <stdlib.h>
#include "structures.h"
#include "util.h"

void k_flood_fill(image_t *img, int x, int y, pixel_t target_color, pixel_t go) {
    if (x < 0 || y < 0 || x >= img->w || y >= img->h || img->img[y * img->w + x].r != go.r ||
        img->img[y * img->w + x].g != go.g || img->img[y * img->w + x].b!= go.b) {
        return;
    }
    img->img[y * img->w + x] = target_color;
    k_flood_fill(img, x + 1, y, target_color, go);
    k_flood_fill(img, x - 1, y, target_color, go);
    k_flood_fill(img, x, y + 1, target_color, go);
    k_flood_fill(img, x, y - 1, target_color, go);
}
void flood_fill(image_t *img, int x, int y, pixel_t target_color, pixel_t go) {
   k_flood_fill(img, x, y, target_color, go);
}
void flood(image_t *img, int x, int y, pixel_t *target_color) {
    if (x < 0 || y < 0 || x >= img->w || y >= img->h) {
        return;
    }
    pixel_t go = img->img[y * img->w + x];
    if (go.r == target_color->r && go.g == target_color->g && go.b == target_color->b) {
        return;
    }
    flood_fill(img, x, y, *target_color, go);
}
