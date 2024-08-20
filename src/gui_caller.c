#include "gui_caller.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file_io.h"
#include "flood_fill.h"
#include "image_edit.h"

#define INTENTIONAL_UNUSED(x) (void)(x)

// higher order function that first clones the image, then calls the function
// would be possible but not worth the effort
// image_map wrap(void (*f)(image_t*))

image_t *clone_image(image_t *img) {
    image_t *new_img = malloc(sizeof(image_t));
    new_img->w = img->w;
    new_img->h = img->h;
    new_img->img = malloc(sizeof(pixel_t) * img->w * img->h);
    memcpy(new_img->img, img->img, sizeof(pixel_t) * img->w * img->h);
    return new_img;
}

image_t *rotate_cw(image_t *img, void **args) {
    INTENTIONAL_UNUSED(args);
    image_t *new_img = clone_image(img);
    rotate_clockwise(new_img);
    return new_img;
}

image_t *rotate_ccw(image_t *img, void **args) {
    INTENTIONAL_UNUSED(args);
    image_t *new_img = clone_image(img);
    rotate_counterclockwise(new_img);
    return new_img;
}

image_t *mirror_h(image_t *img, void **args) {
    INTENTIONAL_UNUSED(args);
    image_t *new_img = clone_image(img);
    mirror_horizontal(new_img);
    return new_img;
}

image_t *mirror_v(image_t *img, void **args) {
    INTENTIONAL_UNUSED(args);
    image_t *new_img = clone_image(img);
    mirror_vertical(new_img);
    return new_img;
}

image_t *read_ppm(image_t *img, void **args) {
    INTENTIONAL_UNUSED(img);
    char *filename = (char *)(args[0]);
    printf("read filename: %s\n", filename);
    image_t *new_img = malloc(sizeof(image_t));
    FILE *file_handle = fopen(filename, "r");
    image_read(new_img, file_handle);
    fclose(file_handle);
    return new_img;
}

image_t *save_ppm(image_t *img, void **args) {
    char *filename = (char *)(args[0]);
    printf("write filename: %s\n", filename);
    FILE *file_handle = fopen(filename, "w");
    image_write(img, file_handle);
    fclose(file_handle);
    return NULL;
}

image_t *resize_img(image_t *img, void **args) {
    char *size = (char *)(args[0]);
    int w, h;
    sscanf(size, "%dx%d", &w, &h);
    printf("Resize to %dx%d\n", w, h);

    image_t *new_img = clone_image(img);
    resize(new_img, w, h);
    return new_img;
}

image_t *flood_image(image_t *img, void **args) {
    int *color = (int *)(args[0]);
    pixel_t fill_color;
    fill_color.r = color[0];
    fill_color.g = color[1];
    fill_color.b = color[2];
    int x, y;
    int *pos = (int *)(args[1]);
    x = pos[0];
    y = pos[1];
    printf("Flood fill at %dx%d with color %d %d %d\n", x, y, color[0],
           color[1], color[2]);

    image_t *new_img = clone_image(img);
    flood(new_img, x, y, &fill_color);
    return new_img;
}

int start_gui() {
    const argument_t read_params[] = {{
        .name = "Image",
        .type = ARG_FILE,
    }};
    const argument_t save_params[] = {{
        .name = "Image",
        .type = ARG_SAVEFILE,
    }};
    const argument_t resize_params[] = {{
        .name = "Size",
        .message = "Please enter the new size of the image as WxH",
        .type = ARG_STRING,
        .default_value = "100x100",
    }};
    const argument_t flood_params[] = {{
                                           .name = "Color",
                                           .type = ARG_COLOR,
                                       },
                                       {
                                           .name = "Position",
                                           .type = ARG_POSITION,
                                       }};
    image_action_t actions[] = {
        {"read", "Read PPM", "document-open", read_ppm, read_params, 1},
        {"write", "Write PPM", "document-save", save_ppm, save_params, 1},
        {"rotate-cw", "Rotate Clockwise", "object-rotate-right", rotate_cw,
         NULL, 0},
        {"rotate-ccw", "Rotate Counter-Clockwise", "object-rotate-left",
         rotate_ccw, NULL, 0},
        {"mirror-h", "Mirror Horizontal", "object-flip-horizontal", mirror_h,
         NULL, 0},
        {"mirror-v", "Mirror Vertical", "object-flip-vertical", mirror_v, NULL,
         0},
        {"resize", "Resize", "view-fullscreen", resize_img, resize_params, 1},
        {"flood", "Flood", "preferences-desktop-wallpaper", flood_image,
         flood_params, 2},

        // a few more ideas if you are eager to implement more
        // use another branch to make sure to not interfere with the project
        // submission
        // {"read", "Read PNG", "document-open", NULL, read_params, 1},
        // { "invert", "Colors", NULL, "Invert colors" },
        // { "sepia", "Colors", NULL, "Sepia" },
        // { "blur", "Filters", NULL, "Blur" },
        // { "sharpen", "Filters", NULL, "Sharpen" },
        // { "edge", "Filters", NULL, "Edge detection" },
        // { "emboss", "Filters", NULL, "Emboss" },
        // { "red", "Channels", NULL, "Red channel" },
        // { "green", "Channels", NULL, "Green channel" },
        // { "blue", "Channels", NULL, "Blue channel" },
        // { "hue", "Channels", NULL, "Hue channel" },
        // { "saturation", "Channels", NULL, "Saturation channel" },
        // { "value", "Channels", NULL, "Value channel" },
        // { "brightness", "Channels", NULL, "Brightness channel" },
        // { "contrast", "Channels", NULL, "Contrast channel" },
        // { "gamma", "Channels", NULL, "Gamma channel" },
        // { "histogram", "Channels", NULL, "Histogram" },

        // { "stablediffusion", "Stable Diffusion", "", NULL, NULL},
    };

    return initialize_gui(sizeof(actions) / sizeof(actions[0]), actions);
}
