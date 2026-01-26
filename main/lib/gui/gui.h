#ifndef GUI_H
#define GUI_H

#include <lvgl.h>
#include "wlan.h"
#include "maincontrol.h"
#include "FT6336U.h"
#include <Main_Definitions.h>
#include "User_Setup.h"
#include "wlan.h"
#include <stack>
#include "serverInterface.h"
#include "maincontrol.h"
#include "gui_gateway.h"

// Misc Image Includes
#include "main_logo.h"
#include "monitor.h"
#include "robotic_arm.h"
#include "checkerboard.h"

// Piece Image Includes
#include "black_pawn.h"
#include "black_knight.h"
#include "black_rook.h"
#include "black_queen.h"
#include "black_king.h"
#include "white_pawn.h"
#include "white_knight.h"
#include "white_rook.h"
#include "white_queen.h"
#include "white_king.h"
#include "white_king_large.h"
#include "black_king_large.h"

#include "lightning.h"
#include "rapid_clock.h"

// Wifi Signal Image Includes
#include "wifi_off.h"
#include "wifi_low_strength.h"
#include "wifi_med_strength.h"
#include "wifi_full_strength.h"

class GUI {
private:
    GUI();
    lv_obj_t *start_screen;
    lv_obj_t *wifi_prompt_screen;
    lv_obj_t *settings_screen;
    lv_obj_t *side_select_screen;
    lv_obj_t *difficulty_screen;
    lv_obj_t *time_control_screen;
    lv_obj_t *start_game_screen;
    lv_obj_t *active_game_screen;
    
    static std::stack<lv_obj_t*> screen_stack;

    lv_obj_t *wifi_icon;

    lv_obj_t *settings_menu;
    lv_obj_t *main_page;
    lv_obj_t *wifi_sub_page;
    //lv_obj_t *arm_mechanics_sub_page;
    lv_obj_t *wifiNetworkContainer;

    lv_obj_t *loading_spinner;

    lv_obj_t * keyboard;

    //FT6336U ft6336u(I2C_SDA, I2C_SCL, RST_N_PIN, INT_N_PIN); // Touch controller object 
    static FT6336U ft6336u; // Touch controller object

    typedef enum {
        LV_MENU_ITEM_BUILDER_VARIANT_1,
        LV_MENU_ITEM_BUILDER_VARIANT_2
    } lv_menu_builder_variant_t;

    /* STYLES */
    static lv_style_t generic_btn_style;
    static lv_style_t nobg_btn_style;
    static lv_style_t alert_btn_style;
    static lv_style_t screen_style;
    static lv_style_t active_timer;
    static lv_style_t inactive_timer;
    static lv_style_t calibration_container;
    static lv_style_t temp_slider;


    /* Variables for Chess Clock Page (active game page) */
    static int user_total_seconds;
    static int user_minutes;
    static int user_seconds;

    lv_timer_t* user_timer;

    static int computer_total_seconds;
    static int computer_minutes;
    static int computer_seconds;

    lv_timer_t* computer_timer;

    // wifi animation images array
    static const lv_image_dsc_t * wifi_anim_arr[3];

    // class references
    WLAN* wlan;

public:
    // GUI is a singleton
    static GUI& instance() {
        static GUI gui_instance;
        return gui_instance;
    }

    // this will hold a reference to the GUI singleton as well as any other random data for LVGL callbacks
    struct GUI_EXTRA {
        GUI* gui;
        void* extra;
    };

    struct NetworkInfo {
        struct WLAN::cNetwork network;
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

    struct executeCalibrationData {
        lv_obj_t * status_icon;
        lv_obj_t * message_box;
    };

    // this struct holds each sides area container on the chess clock active game menu
    struct SidesContainer{
        lv_obj_t* user_side_container;
        lv_obj_t* computer_side_container;
    };

    static SidesContainer sides_container;

    struct SliderInfo {
        lv_obj_t *slider_label;
        int32_t val;
        byte t;
    };

    GameInfo gameInfo;
    std::vector<NetworkInfo> networkInfo;

    void initializeGUI(WLAN* wifiRef);
    void set_active_game_state();

    lv_obj_t* create_text(lv_obj_t * parent, const char * icon, const char * txt,
                              lv_menu_builder_variant_t builder_variant, lv_font_t * fontSize = (lv_font_t *)&lv_font_montserrat_18, int iconScalingFactor = 256);
    lv_obj_t* create_slider(lv_obj_t * parent, const char * icon, const char * txt, int32_t min, int32_t max,
                                int32_t val, lv_font_t * fontSize = (lv_font_t *)&lv_font_montserrat_18, int iconScalingFactor = 256);
    char* get_time_control_label(TimeControl time_control_value);

    void start_touch_object();

    // screen function definitions
    // main pages
    void setup_top_layer();
    void setup_start_screen();
    void setup_wifi_prompt_screen();
    void setup_settings_screen();
    void setup_difficulty_screen();
    void setup_side_select_screen();
    void setup_start_game_screen();
    void setup_active_game_screen();
    void setup_time_control_screen();
    void setup_screen_template(lv_obj_t * screen, char* title);

    // sub pages
    void setup_arm_mechanics_subpage(lv_obj_t * arm_mechanics_page);


    void switch_to_start();
    void static switch_to_screen(lv_obj_t* new_screen);
    void go_back_screen();

    void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t * px_map);
    static void my_touch_read(lv_indev_t * indev, lv_indev_data_t * data);
    static uint32_t my_tick(void);

    static void style_init(void);

    // LVGL event handlers (button handler definitions)
    static void start_button_handler(lv_event_t * e);
    static void settings_button_handler(lv_event_t * e);
    static void settings_button_handler_special(lv_event_t * e);
    static void back_event_handler(lv_event_t * e);
    static void default_back_btn_handler(lv_event_t * e);
    static void wifi_submenu_handler(lv_event_t * e);
    static void wifi_credentials_handler(lv_event_t * e);
    static void network_submenu_handler(lv_event_t * e);
    static void disconnect_network_submenu_handler(lv_event_t * e);
    static void execute_calibration_routine_handler_settings(lv_event_t * e);
    static void run_calibration_handler_settings(lv_event_t * e);
    static void slider_event_cb(lv_event_t * e);
    static void side_select_btn_handler(lv_event_t * e);
    static void difficulty_btn_handler(lv_event_t * e);
    static void time_control_btn_handler(lv_event_t * e);
    static void execute_calibration_routine_handler(lv_event_t * e);
    static void start_game_btn_handler(lv_event_t * e);
    static void end_turn_btn_handler(lv_event_t * e);
    static void end_game_button_handler(lv_event_t * e);

    // wifi-related function declarations
    void updateWifiWidget(wl_status_t wifiStatus);
    void updateWifiNetworkList(int networkCount, struct WLAN::cNetwork* networks);

    // misc
    void end_engine_turn_handler();
    void calibration_handler_popup();
    static void clock_timer(lv_timer_t * timer);
};




#endif