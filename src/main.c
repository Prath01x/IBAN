#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file_io.h"
#include "flood_fill.h"
#include "gui_caller.h"
#include "image_edit.h"
#include "structures.h"

static int usage(const char *progname) {
    fprintf(stderr, "Usage: %s COMMAND [INPUT_FILE] [-o OUTPUT_FILE]\n",
            progname);
    fprintf(stderr, "COMMAND is one of:\n");
    fprintf(stderr, "  read -- read .ppm file from standard input\n");
    fprintf(
        stderr,
        "  read-and-write -- read .ppm from stdin and write it to stdout\n");
    fprintf(stderr, "  rotate-counterclockwise -- read .ppm and rotate it\n");
    fprintf(stderr, "  rotate-clockwise -- read .ppm and rotate it\n");
    fprintf(stderr, "  mirror-horizontal -- read .ppm and modify it\n");
    fprintf(stderr, "  mirror-vertical -- read .ppm and modify it\n");
    fprintf(stderr,
            "  resize NEW_WIDTH NEW_HEIGHT -- read .ppm and modify it\n");
    fprintf(stderr,
            "  flood X Y R G B -- read .ppm and flood it with color (R,G,B) "
            "starting at coodrinates (X,Y)\n");
    fprintf(stderr, "  gui -- start the GUI\n");
    fprintf(stderr,
            "INPUT_FILE is the input .ppm file (or - for stdin), default is "
            "stdin\n");
    fprintf(stderr,
            "OUTPUT_FILE is the output .ppm file (or - for stdout), default is "
            "stdout, has to be specified with -o\n");
    return -1;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        return usage(argv[0]);
    }

    const char *command = argv[1];
    if (strcmp(command, "gui") == 0) {
        int status = start_gui();
        return status;
    }

    FILE *input = stdin;
    FILE *output = stdout;

    // -- Begin of parsing optional file handlers --
    int param_count = 0;
    // flood has 5 parameters
    if (strcmp(command, "flood") == 0) {
        param_count = 5;
    } else if (strcmp(command, "resize") == 0) {
        param_count = 2;
    }

    if (argc < param_count + 2) {
        return usage(argv[0]);
    }

    int in_param = -1;
    int out_param = -1;

    if (argc > param_count + 2) {
        if (strcmp(argv[param_count + 2], "-o") == 0) {
            out_param = param_count + 3;
        } else {
            in_param = param_count + 2;
        }
    }

    // assumes no superfluous arguments
    if (argc > param_count + 4) {
        if (strcmp(argv[param_count + 3], "-o") == 0) {
            out_param = param_count + 4;
        } else {
            in_param = param_count + 4;
        }
    }

    if (in_param >= argc || out_param >= argc) {
        return usage(argv[0]);
    }

    // already open handles here to keep file handling in one place
    if (in_param != -1) {
        input = fopen(argv[in_param], "r");
        if (input == NULL) {
            fprintf(stderr, "Error opening input file\n");
            return -1;
        }
    }

    if (out_param != -1) {
        output = fopen(argv[out_param], "w");
        if (output == NULL) {
            fprintf(stderr, "Error opening output file\n");
            return -1;
        }
    }
    // -- End of parsing optional file handlers --

    image_t image;

    if (image_read(&image, input) != 0) {
        fprintf(stderr, "Error: The input file is not in PPM format.\n");
        if (in_param != -1) {
            fclose(input);
        }
        return 1;
    }
    if (in_param != -1) {
        fclose(input);
    }

    if (strcmp(command, "read") == 0) {
        // Do nothing, image was already read
    } else if (strcmp(command, "read-and-write") == 0) {
        // Do nothing, write image below
    } else if (strcmp(command, "rotate-counterclockwise") == 0) {
        rotate_counterclockwise(&image);
    } else if (strcmp(command, "rotate-clockwise") == 0) {
        rotate_clockwise(&image);
    } else if (strcmp(command, "mirror-horizontal") == 0) {
        mirror_horizontal(&image);
    } else if (strcmp(command, "mirror-vertical") == 0) {
        mirror_vertical(&image);
    } else if (strcmp(command, "resize") == 0) {
        // too few arguments is already checked
        int width = atoi(argv[2]);
        int height = atoi(argv[3]);
        resize(&image, width, height);
    } else if (strcmp(command, "flood") == 0) {
        // too few arguments is already checked
        int x = atoi(argv[2]);
        int y = atoi(argv[3]);
        pixel_t target;
        target.r = atoi(argv[4]);
        target.g = atoi(argv[5]);
        target.b = atoi(argv[6]);
        flood(&image, x, y, &target);
    } else {
        image_free(&image);
        fprintf(stderr, "Error: Unkown command %s\n", argv[1]);
        return -1;
    }
    image_write(&image, output);
    if (out_param != -1) {
        fclose(output);
    }

    image_free(&image);
    return 0;
}
