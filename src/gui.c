#include "gui.h"

#include <assert.h>
#include <gtk/gtk.h>

#define UNUSED(x) (void)(x)

/*
 * CAUTION: You do not need to understand any of this code.
 * However, you are invited to learn a bit about GTK4.
 * If you find improvements, please let us know in the forum.
 * A simple GUI with buttons on the left and an image on the right.
 * The buttons are combined in a scrollbox.
 * Every important function has a small comment above it for orientation.
 */

// TODO: clean up by passing objects around
GtkWidget *picture;
int image_action_count;
image_action_t *image_actions;

/*
 * Main method for applying actions.
 * All parameters are given in args. We use a double array wrapper encoding to
 * handle casting and alignment. This seems complicated. However, higher-order
 * dependent programming is always complicated. This is just another opaque
 * representation. Conceptually: Extract pixels, apply action, write new pixels
 * back.
 */
void apply_image_action(image_action_t *action, void **args) {
    // TODO: handle case when image is null
    // extract image from picture
    GdkPaintable *paintable = gtk_picture_get_paintable(GTK_PICTURE(picture));

    // get image size
    int width, height;
    width = gdk_paintable_get_intrinsic_width(paintable);
    height = gdk_paintable_get_intrinsic_height(paintable);
    g_print("Image size: %d x %d\n", width, height);

    guchar *pixels = malloc(width * height * 4 * sizeof(guchar));
    gdk_texture_download(GDK_TEXTURE(paintable), pixels,
                         4 * width * sizeof(guchar));
    // BGRA format (Cairo's native format)

    // convert to image
    image_t image;
    image.w = width;
    image.h = height;
    image.img = malloc(width * height * sizeof(pixel_t));
    for (int i = 0; i < width * height; i++) {
        image.img[i].r = pixels[i * 4 + 2];
        image.img[i].g = pixels[i * 4 + 1];
        image.img[i].b = pixels[i * 4 + 0];
    }
    free(pixels);

    // call action
    if (!action->func) {
        free(image.img);
        g_print("Action has no function\n");
        return;
    }

    image_t *new_img = action->func(&image, args);
    // free args (results)
    for (int i = 0; i < action->arg_count; i++) {
        free(args[i]);
    }
    free(args);
    if (!new_img) {
        free(image.img);
        g_print("Action did not return an image\n");
        return;
    }
    free(image.img);

    width = new_img->w;
    height = new_img->h;
    // printf("New image size: %d x %d\n", width, height);
    pixels = malloc(width * height * 4 * sizeof(guchar));
    // convert back to pixels
    for (int i = 0; i < width * height; i++) {
        int pixel_idx = i * 4;
        pixels[pixel_idx + 0] = new_img->img[i].r;
        pixels[pixel_idx + 1] = new_img->img[i].g;
        pixels[pixel_idx + 2] = new_img->img[i].b;
    }

    free(new_img->img);
    free(new_img);

    g_autoptr(GBytes) bytes = NULL;
    GdkTexture *texture;
    GByteArray *pixels_arr;

    // avoid base pixel shift
    pixels_arr = g_byte_array_new();
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            g_byte_array_append(pixels_arr, &pixels[(y * width + x) * 4], 3);
        }
    }
    bytes = g_byte_array_free_to_bytes(pixels_arr);

    texture = gdk_memory_texture_new(width, height, GDK_MEMORY_R8G8B8, bytes,
                                     width * 3);
    gtk_picture_set_paintable(GTK_PICTURE(picture), GDK_PAINTABLE(texture));

    free(pixels);
}

// In the following functions, we handle user input for arguments
void gather_arguments(int, image_action_t *, void **);

// File open and save dialogs
static void on_file_response(GtkDialog *dialog, int response, void **data) {
    if (response == GTK_RESPONSE_ACCEPT) {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);

        g_autoptr(GFile) file = gtk_file_chooser_get_file(chooser);
        char *filename = g_file_get_path(file);

        int i = *((int *)(data[0]));
        free(data[0]);  // is forward->i
        image_action_t *action = data[1];
        void **result = data[2];
        free(data);  // is forward
        result[i] = malloc(strlen(filename) + 1);
        // better than strcpy
        snprintf(result[i], strlen(filename) + 1, "%s", filename);
        printf("Saved %s to result[%d]\n", filename, i);
        gather_arguments(i + 1, action, result);
    } else {
        printf("Cancelled\n");
    }

    gtk_window_destroy(GTK_WINDOW(dialog));
}

void open_file_dialog(void **forward) {
    GtkWidget *dialog;
    GtkFileChooserAction file_action = GTK_FILE_CHOOSER_ACTION_OPEN;

    dialog = gtk_file_chooser_dialog_new("Open File", NULL, file_action,
                                         ("_Cancel"), GTK_RESPONSE_CANCEL,
                                         ("_Open"), GTK_RESPONSE_ACCEPT, NULL);

    gtk_window_present(GTK_WINDOW(dialog));

    g_signal_connect(dialog, "response", G_CALLBACK(on_file_response), forward);
}

void save_file_dialog(void **forward) {
    GtkWidget *dialog;
    GtkFileChooser *chooser;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;

    dialog = gtk_file_chooser_dialog_new("Save File", NULL, action, ("_Cancel"),
                                         GTK_RESPONSE_CANCEL, ("_Save"),
                                         GTK_RESPONSE_ACCEPT, NULL);
    chooser = GTK_FILE_CHOOSER(dialog);

    gtk_file_chooser_set_current_name(chooser, ("image.ppm"));

    gtk_window_present(GTK_WINDOW(dialog));

    g_signal_connect(dialog, "response", G_CALLBACK(on_file_response), forward);
}

void receive_input_response(GtkWidget *widget, int response_id,
                            void **user_data) {
    // if cancel just destroy dialog and return
    if (response_id == GTK_RESPONSE_CANCEL) {
        gtk_window_destroy(GTK_WINDOW(widget));
        return;
    }
    printf("Received input response\n");
    GtkWidget *entry = user_data[0];
    void **forward = user_data[1];
    GtkWidget *dialog = user_data[2];
    free(user_data);
    const char *text = gtk_editable_get_text(GTK_EDITABLE(entry));
    printf("Text: %s\n", text);

    int i = *((int *)(forward[0]));
    free(forward[0]);
    image_action_t *action = forward[1];
    void **result = forward[2];
    free(forward);
    result[i] = malloc(strlen(text) + 1);
    snprintf(result[i], strlen(text) + 1, "%s", text);

    gtk_window_destroy(GTK_WINDOW(dialog));

    gather_arguments(i + 1, action, result);
}

// Input dialog for general handling (strings, ints, fallback)
void input_dialog(GtkWindow *parent, const char *name, const char *message,
                  const char *preset, void **forward) {
    GtkWidget *dialog, *content_area, *label, *entry;

    dialog = gtk_dialog_new_with_buttons(
        name, parent, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        ("_Cancel"), GTK_RESPONSE_CANCEL, ("_OK"), GTK_RESPONSE_ACCEPT, NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    label = gtk_label_new(message);
    entry = gtk_entry_new();
    if (preset) {
        gtk_editable_set_text(GTK_EDITABLE(entry), preset);
    }

    gtk_box_append(GTK_BOX(content_area), label);
    gtk_box_append(GTK_BOX(content_area), entry);

    gtk_widget_set_visible(dialog, TRUE);

    void **args = malloc(3 * sizeof(void *));
    args[0] = entry;
    args[1] = forward;
    args[2] = dialog;
    g_signal_connect(dialog, "response", G_CALLBACK(receive_input_response),
                     args);
}

void receive_color_response(GtkWidget *widget, int response_id,
                            void **user_data) {
    if (response_id == GTK_RESPONSE_CANCEL) {
        gtk_window_destroy(GTK_WINDOW(widget));
        return;
    }
    printf("Received color response\n");
    GtkWidget *color_chooser = user_data[0];
    void **forward = user_data[1];
    GtkWidget *dialog = user_data[2];
    free(user_data);
    GdkRGBA color;
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(color_chooser), &color);
    int r = color.red * 255;
    int g = color.green * 255;
    int b = color.blue * 255;
    printf("Color: %d %d %d\n", r, g, b);

    int i = *((int *)(forward[0]));
    free(forward[0]);
    image_action_t *action = forward[1];
    void **result = forward[2];
    free(forward);
    result[i] = malloc(sizeof(int) * 3);
    ((int *)result[i])[0] = r;
    ((int *)result[i])[1] = g;
    ((int *)result[i])[2] = b;

    gtk_window_destroy(GTK_WINDOW(dialog));

    gather_arguments(i + 1, action, result);
}

void color_choose_dialog(GtkWindow *parent, const char *name, void **forward) {
    GtkWidget *dialog, *content_area;

    dialog = gtk_dialog_new_with_buttons(
        name, parent, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        ("_Cancel"), GTK_RESPONSE_CANCEL, ("_OK"), GTK_RESPONSE_ACCEPT, NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget *color_chooser = gtk_color_chooser_widget_new();
    gtk_box_append(GTK_BOX(content_area), color_chooser);

    gtk_widget_set_visible(dialog, TRUE);

    void **args = malloc(3 * sizeof(void *));
    args[0] = color_chooser;
    args[1] = forward;
    args[2] = dialog;
    g_signal_connect(dialog, "response", G_CALLBACK(receive_color_response),
                     args);
}

void on_motion(GtkEventControllerMotion *controller, double x, double y,
               int *position) {
    UNUSED(controller);
    position[0] = x;
    position[1] = y;
}

void on_image_click(GtkWidget *widget, GdkEvent *event, void **data) {
    UNUSED(widget);
    UNUSED(event);
    int *pos = data[0];
    printf("x: %d, y: %d\n", pos[0], pos[1]);
    void **forward = data[1];
    GtkWidget *dialog = data[2];

    int i = *((int *)(forward[0]));
    free(forward[0]);
    image_action_t *action = forward[1];
    void **result = forward[2];
    result[i] = malloc(sizeof(int) * 2);
    ((int *)result[i])[0] = pos[0];
    ((int *)result[i])[1] = pos[1];
    free(pos);
    free(data);
    free(forward);

    gtk_window_destroy(GTK_WINDOW(dialog));

    gather_arguments(i + 1, action, result);
}

void position_picker_dialog(GtkWindow *parent, const char *name,
                            void **forward) {
    UNUSED(parent);
    GtkWidget *dialog, *content_area;

    dialog = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(dialog), name);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    // show image, connect motion controller that reacts on click
    GdkPaintable *paintable = gtk_picture_get_paintable(GTK_PICTURE(picture));
    GtkWidget *image = gtk_image_new_from_paintable(paintable);
    // gtk_box_append(GTK_BOX(content_area), image);
    // do not stretch image
    gtk_widget_set_hexpand(image, FALSE);
    gtk_widget_set_vexpand(image, FALSE);
    // set size to image size
    int width, height;
    width = gdk_paintable_get_intrinsic_width(paintable);
    height = gdk_paintable_get_intrinsic_height(paintable);
    gtk_widget_set_size_request(image, width, height);

    int *position = malloc(sizeof(int) * 2);
    GtkEventController *controller = gtk_event_controller_motion_new();
    gtk_widget_add_controller(image, controller);
    g_signal_connect(controller, "motion", G_CALLBACK(on_motion), position);

    void **args = malloc(3 * sizeof(void *));
    args[0] = position;
    args[1] = forward;
    args[2] = dialog;

    // using GestureClick (GTK4)
    GtkGesture *gesture = gtk_gesture_click_new();
    gtk_widget_add_controller(image, GTK_EVENT_CONTROLLER(gesture));
    g_signal_connect(gesture, "pressed", G_CALLBACK(on_image_click), args);

    gtk_box_append(GTK_BOX(content_area), image);

    gtk_widget_set_visible(dialog, TRUE);
}

// handles dispatch to other functions
void gather_arguments(int i, image_action_t *action, void **result) {
    if (i == action->arg_count) {
        apply_image_action(action, result);
        return;
    }

    // freed in receiver functions => before calling gather_arguments again
    void **forward = malloc(3 * sizeof(void *));
    // free in receiver functions => after retrieving i for incrementing
    forward[0] = malloc(sizeof(int));
    *((int *)(forward[0])) = i;
    forward[1] = action;
    forward[2] = result;

    // input_dialog(NULL, "Test", forward);

    argument_t arg = action->args[i];
    switch (arg.type) {
        case ARG_FILE:
            open_file_dialog(forward);
            break;
        case ARG_SAVEFILE:
            save_file_dialog(forward);
            break;
        case ARG_STRING:
            input_dialog(NULL, arg.name, arg.message, arg.default_value,
                         forward);
            break;
        case ARG_COLOR:
            color_choose_dialog(NULL, arg.name, forward);
            break;
        case ARG_POSITION:
            position_picker_dialog(NULL, arg.name, forward);
            break;
        default:
            g_print("Argument type not implemented\n");
            break;
    }
}

/*
 * This function is invoked on button press.
 * We first gather the arguments, then apply the action.
 * Due to the async nature of the file dialogs, we need to use callbacks.
 * As C has some problems with higher-order functions and no support for
 * closures, we use and unrolled evaluated continuation passing approach by
 * instanting callbacks using passed paramters.
 */
static void handle_image_action(GtkWidget *widget, gpointer data) {
    UNUSED(widget);
    image_action_t *action = (image_action_t *)data;
    g_print("Image action: %s\n", action->name);
    void **result = malloc(action->arg_count * sizeof(void *));
    gather_arguments(0, action, result);
}

/*
 * Build the main GUI
 */
static void activate(GtkApplication *app, gpointer user_data) {
    UNUSED(user_data);
    GtkWidget *window;
    GtkWidget *button;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Window");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    // windows h box
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    // left: buttons, quit in scrollbox
    // right: infolabel, image

    // left column
    GtkWidget *leftbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_hexpand(leftbox, TRUE);
    gtk_widget_set_vexpand(leftbox, TRUE);

    GtkWidget *scrollbox = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scrollbox, TRUE);
    // add left column to scrollbox (GTK4 style)
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrollbox), leftbox);
    // make left column not collapsable
    gtk_scrolled_window_set_min_content_width(GTK_SCROLLED_WINDOW(scrollbox),
                                              200);

    // right column
    GtkWidget *rightbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_hexpand(rightbox, TRUE);
    gtk_widget_set_vexpand(rightbox, TRUE);

    // add left and right column to window (GTK4 style)
    gtk_window_set_child(GTK_WINDOW(window), hbox);
    gtk_box_append(GTK_BOX(hbox), scrollbox);
    gtk_box_append(GTK_BOX(hbox), rightbox);

    // add buttons from image_actions
    for (int i = 0; i < image_action_count; i++) {
        image_action_t action = image_actions[i];
        GtkWidget *image = gtk_image_new_from_icon_name(action.icon);
        button = gtk_button_new();
        GtkWidget *label = gtk_label_new(action.label);
        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_box_append(GTK_BOX(box), image);
        gtk_box_append(GTK_BOX(box), label);
        gtk_button_set_child(GTK_BUTTON(button), box);
        // For a reader: why would &action be wrong?
        g_signal_connect(button, "clicked", G_CALLBACK(handle_image_action),
                         &(image_actions[i]));

        gtk_box_append(GTK_BOX(leftbox), button);
    }

    button = gtk_button_new_with_label("Quit");
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(gtk_window_destroy),
                             window);
    gtk_box_append(GTK_BOX(leftbox), button);

    // image (use picture for resizing)
    picture = gtk_picture_new_for_filename("test.png");
    // make image expand to fill space
    gtk_widget_set_hexpand(picture, TRUE);
    gtk_widget_set_vexpand(picture, TRUE);

    // add image to right column
    gtk_box_append(GTK_BOX(rightbox), picture);

    gtk_widget_set_visible(window, TRUE);
}

int initialize_gui(int action_count, image_action_t *actions) {
    GtkApplication *app;
    int status;

    image_action_count = action_count;
    image_actions = actions;

    app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), 0, NULL);
    g_object_unref(app);

    return status;
}
