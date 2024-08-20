#ifndef SRC_GUI_H
#define SRC_GUI_H

#include "structures.h"

typedef image_t *(*image_map)(image_t *, void **);

enum argument_type {
    // handle manually via string
    //   ARG_INT,
    //   ARG_FLOAT,
    ARG_FILE,
    ARG_SAVEFILE,
    ARG_STRING,
    //   ARG_BOOL,
    //   ARG_CHOICE,
    ARG_COLOR,
    ARG_POSITION,
};

// Arguments the user can provide to the image_map functions
struct argument {
    const char *name;
    const char *message;
    const char *default_value;
    int type;
};
typedef struct argument argument_t;

// Defines modularly actions to chose from in the GUI
// => plug in all image functions you implement to play with
struct image_action {
    const char *name;
    const char *label;
    const char *icon;
    image_map func;
    const argument_t *args;
    int arg_count;
};
typedef struct image_action image_action_t;

int initialize_gui(int action_count, image_action_t *actions);

#endif  // SRC_GUI_H
