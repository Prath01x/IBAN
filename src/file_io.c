#include "file_io.h"
#include "structures.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "util.h"

int image_read(image_t *img, FILE *fin) {
    char magic[3];
    int color, w, h, r, g, b;

    if (fscanf(fin, " %2s", magic) != 1 || magic[0] != 'P' || magic[1] != '3')
        return -1;
    if (fscanf(fin, " %d %d", &w, &h) != 2)
        return -1;
    if (fscanf(fin, " %d", &color) != 1 || color != 255)
        return -1;

    img->w = w;
    img->h = h;
    img->img = (pixel_t *)malloc(sizeof(pixel_t) * w * h);
    if (img->img == NULL)
        return -1;

    for (int i = 0; i < w * h; i++) {
        if (fscanf(fin, " %d %d %d", &r, &g, &b) != 3) {
            free(img->img);
            return -1;
        }
        img->img[i].r = r;
        img->img[i].g = g;
        img->img[i].b = b;
    }

    return 0;
}

void image_write(const image_t *img, FILE *fout) {
    fprintf(fout, "P3\n%d %d\n255\n", img->w, img->h);

    for (int i = 0; i < img->w * img->h; i++) {
        fprintf(fout, "%d %d %d\n", img->img[i].r, img->img[i].g, img->img[i].b);
    }
}

void image_free(image_t *img) {
    if (img->img != NULL) {
        free(img->img);
    }
    img->w = 0;
    img->h = 0;
    img->img = NULL;
}
