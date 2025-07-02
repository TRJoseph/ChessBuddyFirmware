#ifndef GUI_H
#define GUI_H

#include <lvgl.h>
#include "wlan.h"
#include "boardcontrol.h"


struct NetworkInfo {
    struct Network network;
    lv_obj_t* network_sub_page;
    lv_obj_t* container;
};

// In seconds
enum TimeControl {
    FIVEMINBLITZ = 300,
    TENMINRAPID = 600,
    THIRTYMINRAPID = 1800
};

struct GameInfo {
    char* difficulty;
    char* side_to_play;
    TimeControl time_control;
};

// this struct holds each sides area container on the chess clock active game menu
typedef struct {
    lv_obj_t* user_side_container;
    lv_obj_t* computer_side_container;
} SidesContainer;

static SidesContainer sides_container;


void initializeGUI();

void start_touch_object();

// screen function definitions
void setup_top_layer();
void setup_start_screen();
void setup_wifi_prompt_screen();
void setup_settings_screen();
void setup_difficulty_screen();
void setup_side_select_screen();
void setup_start_game_screen();
void setup_active_game_screen();

void switch_to_start();

void switch_to_screen(lv_obj_t* new_screen);
void go_back_screen();

void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t * px_map);
void my_touch_read(lv_indev_t * indev, lv_indev_data_t * data);
uint32_t my_tick(void);


// button handler definitions
static void start_button_handler(lv_event_t * e);
static void settings_button_handler(lv_event_t * e);
static void back_button_handler(lv_event_t * e);


void updateWifiWidget(wl_status_t wifiStatus);
void updateWifiNetworkList(int networkCount, struct Network* networks);


#endif