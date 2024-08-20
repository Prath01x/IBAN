#include "image_edit.h"

#include <stdlib.h>

#include "structures.h"
#include "util.h"




    void rotate_counterclockwise(image_t *img) {
    int w1 = img->w;
    int h1 = img->h;
    pixel_t *omg = img->img;

    img->w = h1;
    img->h = w1;

    img->img = (pixel_t*)malloc(sizeof(pixel_t) * img->w * img->h);
    if (img->img == NULL) {
        free(omg);
        return;
    }

    for (int i = 0; i < h1; i++) {
        for (int j = 0; j < w1; j++) {
            img->img[(w1 - 1 - j) * img->w + i] = omg[i * w1 + j];
        }
    }

    free(omg);
}



void rotate_clockwise(image_t *img) {

    int w1 = img->w;
    int h1 = img->h;
    pixel_t *omg = img->img;

    
    img->w = h1;
    img->h = w1;

    img->img = (pixel_t*)malloc(sizeof(pixel_t) * img->w * img->h);
    if (img->img == NULL) {
        free(omg);
        return;
    }

    for (int i = 0; i < h1; i++) {
        for (int j = 0; j < w1; j++) {
            img->img[j * img->w + (h1 - 1 - i)] = omg[i * w1 + j];
        }
    }
 free(omg); 
}

void mirror_horizontal(image_t *img) {
      int width = img->w;
    int height = img->h;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width / 2; j++) {
            pixel_t t = img->img[i * width + j];
            img->img[i * width + j] = img->img[i * width + (width - 1 - j)];
            img->img[i * width + (width - 1 - j)] = t;
        }
    }
}


void mirror_vertical(image_t *img) {
    int width = img->w;
    int height = img->h;

    for (int i = 0; i < height / 2; i++) {
        for (int j = 0; j < width; j++) {
            pixel_t t = img->img[i * width + j];
            img->img[i * width + j] = img->img[(height - 1 - i) * width + j];
            img->img[(height - 1 - i) * width + j] = t;
        }
    }
}

void resize(image_t *img, int new_width, int new_height) {
    
    pixel_t* bb=(pixel_t*)malloc(sizeof(pixel_t)* new_width * new_height);
    if(bb==NULL){
        return;
    }
int i = 0;
while (i < new_height) {
    int j = 0;
    while (j < new_width) {
        bb[i * new_width + j].r = 0;
        bb[i * new_width + j].g = 0;
        bb[i * new_width + j].b = 0;
        j++;
    }
    i++;
}





  int w1;
if (new_width < img->w) {
    w1 = new_width;
} else {
    w1 = img->w;
}

int h1;
if (new_height < img->h) {
    h1 = new_height;
} else {
    h1 = img->h;
}





    for (i=0; i<h1; i++) {
        for (int j=0; j<w1; j++) {
            bb[i*new_width+j]=img->img[i*img->w+j];

    }
    }
    free(img->img);
    img->img=bb;
    img->w=new_width;
    img->h=new_height;


}