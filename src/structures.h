#ifndef SRC_STRUCTURES_H
#define SRC_STRUCTURES_H

/**
 * We use PPM format where each pixel is in RGB
 */
struct pixel {
    int r;
    int g;
    int b;
};
typedef struct pixel pixel_t;

/**
 * The image is decribed by its width .w (number of columns), height .h
 * (number of rows), and the matrix of pixels .img represented by a single
 * array in row-major format. That is, given an image_t struct img and
 * coordinates X (between 0 and img->w) and Y (between 0 and img->h), the
 * pixel on position (X,Y) can be obtained by img->img[Y * img->w + X].
 */
struct image {
    int w;
    int h;
    pixel_t *img;
};
typedef struct image image_t;

#endif  // SRC_STRUCTURES_H
