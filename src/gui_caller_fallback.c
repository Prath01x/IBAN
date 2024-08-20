#include <stdio.h>

#include "gui_caller.h"

int start_gui() {
    printf("GTK4 is not available, using fallback\n");
    return 1;
}
