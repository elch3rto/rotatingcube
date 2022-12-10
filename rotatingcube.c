/* 
  ___ ___ ___  ___ _      _   ___ __  __ ___ ___ 
 |   \_ _/ __|/ __| |    /_\ |_ _|  \/  | __| _ \
 | |) | |\__ \ (__| |__ / _ \ | || |\/| | _||   /
 |___/___|___/\___|____/_/ \_\___|_|  |_|___|_|_\
                                                 
This code is written poorly. It has many flaws, it is 
badly optimised, it does not manage memory correctly, it
may cause unpredicted crashes and reboots, and its
formatting does not follow Flipper Zero developer 
guidelines. Also parts of code have been borrowed from 
applications/examples/example_images/example_images.c
*/

#include <furi.h>
#include <furi_hal.h>
#include <string.h>
#include <stdbool.h>
#include <gui/gui.h>
#include <input/input.h>
#include <math.h>
#include "rotatingcube_icons.h"
//#define DEBUG
//If uncommented, will display the position of every point on screen(Play around with it!)

//TYPE DEFINITIONS

typedef struct {
    int16_t x, y, z;
} Point;

typedef struct {
    uint8_t x1, y1, x2, y2;
} Line;

//VARS THAT DON'T CHANGE

static uint8_t x_screen_center = 63; //An alias for the center positions of the screen
static uint8_t y_screen_center = 31; //Screen is 128x64, from 0 to 63 and from 0 to 127
static uint8_t cube_size = 20; //Just the size of the cube (in pixels)
static double DEGTORAD = 3.1415 / 180; //Serves as an alias for quick conversion

//VARS THAT DO CHANGE

static int32_t yawangle = 0; //This is the angle of Z axis rotation
static int32_t pitchangle = 0; //This is the angle of Y axis rotation
static bool move_or_rotate = false; //This bool defines whether we're in move or rotate mode
static Point cloud_of_points[8]; //This is the array which stores positions of all 8 points of a CUBE
static Point* ptr_to_array =
    &cloud_of_points[0]; //This is a pointer to first elementarray "cloud of points"
static uint8_t num_of_points = 8; //This basically represents the len of array "cloud of points"

//THIS FUNCTION TAKES A POINTER TO AN ARRAY OF 8 POINTS
//AND FILLS THE ARRAY WITH A CLOUD OF POINTS THAT CORRESPOND TO 8 POINTS OF A CUBE
void calculate_points_for_cube(Point* ptr_to_array) {
    /* 
    We need: 
        4 dots in upper part
        4 dots in lower part

        WHAT points[8] WILL LOOK LIKE:
        [0] {-10,-10,-10}    /
        [1] {10, -10,-10}   |  Z is less than 0   
        [2] {10, 10, -10}   |  LOWER LAYER 
        [3] {-10, 10,-10}    \
          -------------
        [4] {-10,-10, 10}  \     
        [5] {10, -10, 10}   | Z is more than 0    
        [6] {10,  10, 10}   | UPPER LAYER
        [7] {-10, 10, 10}  /     
    
    */

    for(uint8_t i = 0; i < 8; i++) {
        (ptr_to_array + i)->x = (i == 0 || i == 3 || i == 4 || i == 7 ? -cube_size : cube_size);
        (ptr_to_array + i)->y = (i == 0 || i == 1 || i == 4 || i == 5 ? -cube_size : cube_size);
        (ptr_to_array + i)->z = (i < 4 ? -cube_size : cube_size);
        // fill in all the combinations
        //  of positive and negative signs
    }
}

//THIS FUNCTION TAKES A POINTER TO A POINT AND APPLIES ROTATION ON 2 AXIS TO THAT POINT
void calculate_point(Point* point) {
    double diag_yaw;
    double diag_pitch;
    double prev_yaw_RAD;
    double prev_pitch_RAD;
    double yaw_RAD = yawangle * DEGTORAD;
    double pitch_RAD = pitchangle * DEGTORAD;

    // 1: We rotate the point along the PITCH axis
    /*

  SCREEN
    |              y
    |              |-X(z,y)
    |              | |
    64     --------|---------z
    |              |
    |              |
  SCREEN      
      */

    diag_pitch = sqrt(pow(point->y, 2) + pow(point->z, 2));
    prev_pitch_RAD = atan2(point->y, point->z);
    pitch_RAD += prev_pitch_RAD;
    point->y = (diag_pitch * cos(pitch_RAD));
    point->z = (diag_pitch * sin(pitch_RAD));

    // 2: We rotate the point along the YAW axis
    /*            z
                  |----X(x,z)
                  |    |
          --------|--------x
                  |
                  |
        
    SCREEN_______128________SCREEN
    */

    diag_yaw = sqrt(pow(point->x, 2) + pow(point->z, 2));
    prev_yaw_RAD = atan2(point->z, point->x);
    yaw_RAD += prev_yaw_RAD;
    point->x = (diag_yaw * cos(yaw_RAD));
    point->z = (diag_yaw * sin(yaw_RAD));

    //return &point[0];
}

//######### THE DRAW FUNCTION #########
//THIS FUNCTION TAKES ALL DATA, RUNS THE POINT POSITION CALCULATION AND DISPLAYS THE LINES
//THAT REPRESENT THE CUBE

static void app_draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);

    canvas_clear(canvas);

    canvas_set_font(canvas, FontKeyboard);

    calculate_points_for_cube(ptr_to_array);

    //This calculates all the points in the static array

    for(uint8_t i = 0; i < num_of_points; i += 1) {
        calculate_point(ptr_to_array + i);
    }

    //This is the loop, which draws 12 lines between 8 points
    for(uint8_t i = 0; i < 12; i++) {
        uint8_t n;
        uint8_t m;

        if(i <= 3) {
            n = i;
            m = (i == 0 ? 3 : i - 1);
        } else if(i >= 4 && i <= 7) {
            n = i;
            m = (i == 4 ? 7 : i - 1);
        } else if(i >= 8 && i <= 11) {
            n = i - 8;
            m = i - 4;
        } else {
            break;
        }

        //This is what draws the actual lines
        canvas_draw_line(
            canvas,
            (cloud_of_points[n].x + x_screen_center) % 128,
            (cloud_of_points[n].y + y_screen_center) % 64,
            (cloud_of_points[m].x + x_screen_center) % 128,
            (cloud_of_points[m].y + y_screen_center) % 64);
    }

//This is debugging code
#ifdef DEBUG
    for(uint8_t i = 0; i < 56; i += 7) {
        char debug_str[32];
        snprintf(
            debug_str,
            32,
            "%d %d %d",
            (cloud_of_points + (i / 7))->x,
            (cloud_of_points + (i / 7))->y,
            (cloud_of_points + (i / 7))->z);
        canvas_draw_str_aligned(canvas, 5, i, AlignLeft, AlignTop, debug_str);
    }

#endif

//This is what writes two variables into nice strings on the screen
#ifndef DEBUG
    char strone[32];
    char strtwo[32];

    snprintf(strone, 32, "Z: %ld deg.", yawangle);
    snprintf(strtwo, 32, "X: %ld deg.", pitchangle);
    canvas_draw_str_aligned(canvas, 5, 0, AlignLeft, AlignTop, strone);
    canvas_draw_str_aligned(canvas, 5, 10, AlignLeft, AlignTop, strtwo);
#endif
}

//######### CALLBACK(THIS RUNS WHEN A BUTTON IS PRESSED) #########

static void app_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx); // ====> WRITES TO DEBUG FILE,

    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

//######### MAIN FUNCTION(THIS RUNS AT START) #########

int32_t rotation_main(void* p) {
    UNUSED(p);
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    // Configure view port
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, app_draw_callback, view_port);
    view_port_input_callback_set(view_port, app_input_callback, event_queue);

    // Register view port in GUI
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    InputEvent event;

    bool running = true;
    while(running) {
        if(furi_message_queue_get(event_queue, &event, 100) == FuriStatusOk) {
            if((event.type == InputTypePress) || (event.type == InputTypeRepeat)) {
                switch(event.key) {
                case InputKeyRight:
                    if(move_or_rotate) {
                        if(yawangle >= -355) {
                            yawangle -= 5;
                        }
                    } else {
                        if(x_screen_center < 122 - cube_size * 1.4) {
                            x_screen_center += 5;
                        }
                    }

                    break;
                case InputKeyLeft:
                    if(move_or_rotate) {
                        if(yawangle <= 355) {
                            yawangle += 5;
                        }
                    } else {
                        if(x_screen_center > 4 + cube_size * 1.4) {
                            x_screen_center -= 5;
                        }
                    }

                    break;
                case InputKeyDown:
                    if(move_or_rotate) {
                        if(pitchangle <= 355) {
                            pitchangle += 5;
                        }
                    } else {
                        if(y_screen_center < 58 - cube_size * 1.4) {
                            y_screen_center += 5;
                        }
                    }
                    break;
                case InputKeyUp:
                    if(move_or_rotate) {
                        if(pitchangle >= -355) {
                            pitchangle -= 5;
                        }
                    } else {
                        if(y_screen_center > 4 + cube_size * 1.4) {
                            y_screen_center -= 5;
                        }
                    }
                    break;
                case InputKeyOk:
                    move_or_rotate = !move_or_rotate;
                    break;

                default:
                    running = false;
                    break;
                }
            }
        }

        //All the drawing stuff is contained in the function update

        view_port_update(view_port);
    }

    /*we exited from this "running == true" loop, now we clean everythink up
        1. Disable the VIEWPORT
        2. Remove the VIEWPORT with gui library
        3. Free ViewStack instance


    */
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);

    furi_record_close(RECORD_GUI);

    return 0;
}
