#include "gui.h"
#include "FT6336U.h"
#include "Main_Definitions.h"
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


lv_obj_t *start_screen;
lv_obj_t *wifi_prompt_screen;
lv_obj_t *settings_screen;
lv_obj_t *side_select_screen;
lv_obj_t *difficulty_screen;
lv_obj_t *time_control_screen;
lv_obj_t *start_game_screen;
lv_obj_t *active_game_screen;

struct GameInfo *gameInfo = (struct GameInfo *)malloc(sizeof(struct GameInfo));

std::stack<lv_obj_t*> screen_stack;

lv_obj_t *wifi_icon;

lv_obj_t *settings_menu;
lv_obj_t * main_page;
lv_obj_t *wifi_sub_page;
//lv_obj_t *arm_mechanics_sub_page;
lv_obj_t *wifiNetworkContainer;

lv_obj_t *loading_spinner;

static lv_obj_t * keyboard;


FT6336U ft6336u(I2C_SDA, I2C_SCL, RST_N_PIN, INT_N_PIN); // Touch controller object

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
static const lv_image_dsc_t * wifi_anim_arr[3] = {
  &wifi_low_strength,
  &wifi_med_strength,
  &wifi_full_strength
};

// Sets up global styles
static void style_init(void) {
    lv_style_init(&generic_btn_style);
    lv_style_set_radius(&generic_btn_style, 20);
    lv_style_set_bg_color(&generic_btn_style, lv_color_hex(0x041941));

    lv_style_init(&nobg_btn_style);
    lv_style_set_bg_opa(&nobg_btn_style, LV_OPA_TRANSP);
    lv_style_set_border_width(&nobg_btn_style, 0);
    lv_style_set_shadow_opa(&nobg_btn_style, LV_OPA_TRANSP);

    lv_style_init(&alert_btn_style);
    lv_style_set_radius(&alert_btn_style, 20);
    lv_style_set_bg_color(&alert_btn_style, lv_color_hex(0xFF3131));
    lv_style_set_border_opa(&alert_btn_style, LV_OPA_TRANSP);

    lv_style_init(&active_timer);
    lv_style_set_bg_opa(&active_timer, LV_OPA_TRANSP);
    lv_style_set_border_opa(&active_timer, LV_OPA_TRANSP);

    lv_style_init(&inactive_timer);
    lv_style_set_bg_color(&inactive_timer, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_bg_opa(&inactive_timer, LV_OPA_50);
    lv_style_set_border_opa(&inactive_timer, LV_OPA_50);

    lv_style_init(&calibration_container);
    lv_style_set_border_color(&calibration_container, lv_color_black());
    lv_style_set_border_width(&calibration_container, 2);
    lv_style_set_border_opa(&calibration_container, LV_OPA_20);
    lv_style_set_pad_top(&calibration_container, 10);
    lv_style_set_bg_color(&calibration_container, lv_color_hex(0xffffff));
    lv_style_set_bg_opa(&calibration_container, LV_OPA_COVER);

    lv_style_init(&temp_slider);
    lv_style_set_bg_opa(&temp_slider, LV_OPA_COVER);
    lv_style_set_bg_color(&temp_slider, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_bg_grad_color(&temp_slider, lv_palette_main(LV_PALETTE_RED));
    lv_style_set_bg_grad_dir(&temp_slider, LV_GRAD_DIR_HOR);
}

void start_touch_object() {
    ft6336u.begin();

    Serial.print("FT6336U Firmware Version: ");
    Serial.println(ft6336u.read_firmware_id());
    Serial.print("FT6336U Device Mode: ");
    Serial.println(ft6336u.read_device_mode());
}

void my_disp_flush( lv_display_t *disp, const lv_area_t *area, uint8_t * px_map)
{
    /*Copy `px map` to the `area`*/

    /*For example ("my_..." functions needs to be implemented by you)
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);

    my_set_window(area->x1, area->y1, w, h);
    my_draw_bitmaps(px_map, w * h);
     */

    /*Call it to tell LVGL you are ready*/
    lv_display_flush_ready(disp);
}

/*Read the touchpad*/
void my_touch_read(lv_indev_t * indev, lv_indev_data_t * data )
{
    // if screen is touched
    if(ft6336u.read_td_status() > 0) {
        uint16_t xTouch = ft6336u.read_touch1_x();
        uint16_t yTouch = ft6336u.read_touch1_y();
        data->point.x = xTouch;
        data->point.y = yTouch;
        Serial.printf("x: %d, y: %d\n", data->point.x, data->point.y);
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

/*use Arduinos millis() as tick source*/
uint32_t my_tick(void)
{
    //uint32_t testTime = esp_timer_get_time() / 1000;
    return esp_timer_get_time() / 1000; // Convert microseconds to milliseconds
}

void start_button_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
        if(WiFi.status() == WL_CONNECTED) {
            // go to first setup page
            switch_to_screen(side_select_screen);
            //switch_to_side_select_screen();
        } else {
            // go to settings prompt page
            switch_to_screen(wifi_prompt_screen);
            //switch_to_wifi_prompt_screen();
        }


    }
    else if(code == LV_EVENT_VALUE_CHANGED) {
        LV_LOG_USER("Toggled");
    }
}

void settings_button_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
        //switch_to_settings();
        switch_to_screen(settings_screen);

    }
    else if(code == LV_EVENT_VALUE_CHANGED) {
        LV_LOG_USER("Toggled");
    }
}

void settings_button_handler_special(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
        lv_screen_load(settings_screen);
    }
    else if(code == LV_EVENT_VALUE_CHANGED) {
        LV_LOG_USER("Toggled");
    }
}

static void back_event_handler(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target_obj(e);
    lv_obj_t * settings_menu = (lv_obj_t *)lv_event_get_user_data(e);
    
    if(lv_menu_back_button_is_root(settings_menu, obj)) {
        go_back_screen();
    }
}

static void default_back_btn_handler(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target_obj(e);
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
        go_back_screen();
    }   
}



void wifi_submenu_handler(lv_event_t * e) {

    // TODO: add some sort of UI spinner or something to indicate the networks are being loaded

    lv_obj_t *child = lv_obj_get_child(wifi_sub_page, 0);
    while (child != NULL) {
        lv_obj_t *next = lv_obj_get_child(wifi_sub_page, 1); // always get next from index 1
        lv_obj_del(child);
        child = next;
    }

    loading_spinner = lv_spinner_create(wifi_sub_page);
    lv_obj_set_size(loading_spinner, 100, 100);
    lv_obj_center(loading_spinner);
    lv_spinner_set_anim_params(loading_spinner, 10000, 200);

    startWifiScan();

    updateWifiWidget(WiFi.status());
}

static void ta_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target_obj(e);

    struct NetworkInfo * networkInfo = (struct NetworkInfo*)lv_event_get_user_data(e);

    if(code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED) {
        /*Focus on the clicked text area*/
        if(keyboard != NULL) lv_keyboard_set_textarea(keyboard, ta);
    }

    else if(code == LV_EVENT_READY) {
        Serial.write("USER INPUTTED TEXT");
        
        LV_LOG_USER("Ready, current text: %s", lv_textarea_get_text(ta));

        const char* input_text = lv_textarea_get_text(ta);
        Serial.println("USER INPUTTED TEXT:");
        Serial.println(input_text);  // Serial output

        // Hide the container and show the loading spinner instead
        if (networkInfo->container) {
            lv_obj_add_flag(networkInfo->container, LV_OBJ_FLAG_HIDDEN);
        }

        // attempt to connect to network the user clicked the checkbox
        loading_spinner = lv_spinner_create(networkInfo->network_sub_page);
        lv_obj_set_size(loading_spinner, 100, 100);
        lv_obj_center(loading_spinner);
        lv_spinner_set_anim_params(loading_spinner, 10000, 200);

        lv_refr_now(NULL);

        // FOR NOW I WANT THIS TO BE BLOCKING UNTIL I CAN GET TO DISABLING THE BACK BUTTON, ETC, ETC
        connectToWifiNetworkBlocking(networkInfo->network.ssid, input_text);

        lv_obj_del(loading_spinner);
        lv_obj_clear_flag(networkInfo->container, LV_OBJ_FLAG_HIDDEN);

        wl_status_t wifiStatus = WiFi.status();

        // wifi success UI updates
        if(wifiStatus == WL_CONNECTED) {
            
            // pswd btn label
            lv_obj_t * child = lv_obj_get_child(networkInfo->container, 0);
            lv_obj_add_flag(child, LV_OBJ_FLAG_HIDDEN);

            // pswd btn 
            child = lv_obj_get_child(networkInfo->container, 1);
            lv_obj_add_flag(child, LV_OBJ_FLAG_HIDDEN);

            // success btn
            child = lv_obj_get_child(networkInfo->container, 2);
            lv_obj_clear_flag(child, LV_OBJ_FLAG_HIDDEN);

            // failure btn
            child = lv_obj_get_child(networkInfo->container, 3);
            lv_obj_add_flag(child, LV_OBJ_FLAG_HIDDEN);

            // separator
            child = lv_obj_get_child(networkInfo->container, 4);
            lv_obj_add_flag(child, LV_OBJ_FLAG_HIDDEN);
            // keyboard
            child = lv_obj_get_child(networkInfo->container, 5);
            lv_obj_add_flag(child, LV_OBJ_FLAG_HIDDEN);
        } else {
            // success btn
            lv_obj_t * child = lv_obj_get_child(networkInfo->container, 2);
            lv_obj_add_flag(child, LV_OBJ_FLAG_HIDDEN);

            // failure btn
            child = lv_obj_get_child(networkInfo->container, 3);
            lv_obj_clear_flag(child, LV_OBJ_FLAG_HIDDEN);
        }

        updateWifiWidget(wifiStatus);
    }
}

void network_submenu_handler(lv_event_t * e) {

    struct NetworkInfo * networkInfo = (struct NetworkInfo*)lv_event_get_user_data(e);

    if (!networkInfo) return;

    lv_obj_t *child = lv_obj_get_child(networkInfo->network_sub_page, 0);
    while (child != NULL) {
        lv_obj_t *next = lv_obj_get_child(networkInfo->network_sub_page, 1); // always get next from index 1
        lv_obj_del(child);
        child = next;
    }

    // lv_obj_t *cont = lv_menu_cont_create(network_sub_page);
    // lv_obj_set_style_bg_color(cont, lv_color_hex(0xffffff), LV_PART_MAIN);
    // lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, LV_PART_MAIN);

    // lv_obj_t *network_cont_label = lv_label_create(cont);
    // lv_label_set_text(network_cont_label, network->ssid.c_str());
    // lv_label_set_long_mode(network_cont_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    // lv_obj_set_flex_grow(network_cont_label, 1);
 
    /* Create a container with vertical flex layout */
    lv_obj_t *cont = lv_menu_cont_create(networkInfo->network_sub_page);
    networkInfo->container = cont;
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);  
    lv_obj_set_size(cont, 320, 440); 

    /* Password label */
    lv_obj_t * pwd_label = lv_label_create(cont);
    lv_label_set_text(pwd_label, "Enter Network Password:");
    lv_obj_set_style_pad_top(pwd_label, 10, 0); 

    /* Password text area */
    lv_obj_t * pwd_ta = lv_textarea_create(cont);
    lv_textarea_set_text(pwd_ta, "");
    lv_textarea_set_password_mode(pwd_ta, true);
    lv_textarea_set_one_line(pwd_ta, true);
    lv_obj_set_width(pwd_ta, lv_pct(80));
    lv_obj_add_event_cb(pwd_ta, ta_event_cb, LV_EVENT_ALL, networkInfo);

    lv_obj_t * success_status_label = lv_label_create(cont);
    lv_label_set_text(success_status_label, "Connected Successfully");
    lv_obj_align(success_status_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(success_status_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_text_font(success_status_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(success_status_label, lv_color_hex(0x80EF80), LV_PART_MAIN);
    lv_label_set_long_mode(success_status_label, LV_LABEL_LONG_WRAP);

    lv_obj_t * failure_status_label = lv_label_create(cont);
    lv_obj_set_width(failure_status_label, 300);
    lv_label_set_text(failure_status_label, "Connection failed (wrong password or AP unreachable).");
    lv_obj_align(failure_status_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(failure_status_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_text_font(failure_status_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(failure_status_label, lv_color_hex(0xff474c), LV_PART_MAIN);
    lv_label_set_long_mode(failure_status_label, LV_LABEL_LONG_WRAP);
    

    /* Spacer to push keyboard to the bottom */
    lv_obj_t * spacer = lv_obj_create(cont);
    lv_obj_set_size(spacer, 1, 1);
    lv_obj_set_flex_grow(spacer, 1); 
    lv_obj_set_style_opa(spacer, LV_OPA_TRANSP, 0);

    /* Keyboard */
    keyboard = lv_keyboard_create(cont);
    lv_obj_set_height(keyboard, 160); 
    lv_obj_set_width(keyboard, 320);
    lv_keyboard_set_textarea(keyboard, pwd_ta);

    lv_obj_set_scrollbar_mode(networkInfo->network_sub_page, LV_SCROLLBAR_MODE_OFF);

}

void disconnect_network_submenu_handler(lv_event_t * e) {
    struct NetworkInfo * networkInfo = (struct NetworkInfo*)lv_event_get_user_data(e);

    if (!networkInfo) return;

    lv_obj_t *child = lv_obj_get_child(networkInfo->network_sub_page, 0);
    while (child != NULL) {
        lv_obj_t *next = lv_obj_get_child(networkInfo->network_sub_page, 1); // always get next from index 1
        lv_obj_del(child);
        child = next;
    }

    disconnectFromWifiNetwork();
    lv_menu_clear_history(settings_menu);
    lv_menu_set_page(settings_menu, main_page);
    
    updateWifiWidget(WiFi.status());
}


// scan end callback, updates the wifi network list within settings
void updateWifiNetworkList(int networkCount, struct Network* networks) {

  // ends UI spinner to indicate the networks are loaded
  if (loading_spinner) {
      lv_obj_del(loading_spinner);
      loading_spinner = NULL;
  }

  if (networks != NULL && networkCount > 0) {
  
    for(int i = 0; i < networkCount; ++i) {
        // DEBUG INFORMATION
        Serial.println("First network details:");
        Serial.printf("  SSID: %s\n", networks[i].ssid.c_str());
        Serial.printf("  Signal Strength: %d dBm\n", networks[i].rssi);
        Serial.printf("  Channel: %d\n", networks[i].channel);
        Serial.printf("  Encryption: %d\n", networks[i].encryptionType);

        // in case modification of network labels is needed
        String networkLabel = networks[i].ssid;

        lv_obj_t* network_sub_page = lv_menu_page_create(settings_menu, networkLabel.c_str());

        lv_obj_t *cont = lv_menu_cont_create(wifi_sub_page);
        lv_obj_set_style_bg_color(cont, lv_color_hex(0xffffff), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, LV_PART_MAIN);

        struct NetworkInfo* networkInfo = new struct NetworkInfo;
        networkInfo->network = networks[i];
        networkInfo->network_sub_page = network_sub_page;

        lv_obj_t *wifi_image;
        wifi_image = lv_image_create(cont);
        if(networks[i].rssi > -60) {
        // "strong" signal
        lv_image_set_src(wifi_image, &wifi_full_strength);
        } else if (networks[i].rssi > -80) {
        // "fair" signal
        lv_image_set_src(wifi_image, &wifi_med_strength);
        } else {
        // "weak" signal
        lv_image_set_src(wifi_image, &wifi_low_strength);
        }

        lv_obj_t *wifi_cont_label = lv_label_create(cont);
        lv_label_set_text(wifi_cont_label, networkLabel.c_str());
        lv_label_set_long_mode(wifi_cont_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_set_flex_grow(wifi_cont_label, 1);

        lv_menu_set_load_page_event(settings_menu, cont, network_sub_page);

        // place disconnect button instead of passwords prompt screen 
        if(WiFi.status() == WL_CONNECTED && networkCount == 1) {
            // disconnect from network button
            lv_obj_t *disconnect_btn = lv_button_create(network_sub_page);
            lv_obj_add_event_cb(disconnect_btn, disconnect_network_submenu_handler, LV_EVENT_CLICKED, networkInfo);
            lv_obj_align(disconnect_btn, LV_ALIGN_CENTER, 0, 0);
            lv_obj_remove_flag(disconnect_btn, LV_OBJ_FLAG_PRESS_LOCK);
            lv_obj_set_size(disconnect_btn, 200, 80);
            lv_obj_add_style(disconnect_btn, &generic_btn_style, 0);

            lv_obj_t *disconnect_label;
            disconnect_label = lv_label_create(disconnect_btn);
            lv_label_set_text(disconnect_label, "Disconnect");
            lv_obj_center(disconnect_label);
            lv_obj_set_style_text_font(disconnect_label, &lv_font_montserrat_30, 0);

            return;
        } else {
            lv_obj_add_event_cb(cont, network_submenu_handler, LV_EVENT_CLICKED, networkInfo);
        }


    }
  } else {
    lv_obj_t *cont = lv_menu_cont_create(wifi_sub_page);
    lv_obj_t *label = lv_label_create(cont);
    lv_label_set_text(label, "No networks found or scan failed");
    lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_flex_grow(label, 1);
  }
}

void updateWifiWidget(wl_status_t wifiStatus) {
    // get connection status
    // Delete the previous icon if it exists
    if (wifi_icon) {
        lv_obj_del(wifi_icon);
        wifi_icon = NULL;
    }
    if (wifiStatus == WL_CONNECTED) {
        // Create a static image instead
        wifi_icon = lv_image_create(lv_layer_top());
        lv_obj_align(wifi_icon, LV_ALIGN_TOP_RIGHT, -5, 8);

        int signalStrength = getWifiSignalStrength();
        if(signalStrength > -60) {
          // "strong" signal
          lv_image_set_src(wifi_icon, &wifi_full_strength);
        } else if (signalStrength > -80) {
          // "fair" signal
          lv_image_set_src(wifi_icon, &wifi_med_strength);
        } else {
          // "weak" signal
          lv_image_set_src(wifi_icon, &wifi_low_strength);
        }

    } else if(wifiStatus == WL_CONNECT_FAILED || wifiStatus == WL_IDLE_STATUS || wifiStatus == WL_DISCONNECTED) {
        wifi_icon = lv_image_create(lv_layer_top());
        lv_obj_align(wifi_icon, LV_ALIGN_TOP_RIGHT, -5, 8);
        lv_image_set_src(wifi_icon, &wifi_off);
    } else {
        // Create an animated icon
        wifi_icon = lv_animimg_create(lv_layer_top());
        lv_obj_align(wifi_icon, LV_ALIGN_TOP_RIGHT, -5, 8);
        lv_animimg_set_src(wifi_icon, (const void **)wifi_anim_arr, 3);
        lv_animimg_set_duration(wifi_icon, 1000);
        lv_animimg_set_repeat_count(wifi_icon, LV_ANIM_REPEAT_INFINITE);
        lv_animimg_start(wifi_icon);
    }
}

void setup_top_layer() {
    style_init();
    updateWifiWidget(WiFi.status());
}

void setup_start_screen() {
    start_screen = lv_obj_create(NULL);

    // background styling 
    lv_obj_set_style_bg_color(start_screen, lv_color_hex(0x7295CA), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(start_screen, lv_color_hex(0x0D57A2), 0);
    lv_obj_set_style_bg_grad_dir(start_screen, LV_GRAD_DIR_HOR, 0);

    // title button
    lv_obj_t *title_label = lv_label_create(start_screen);
    lv_label_set_text( title_label, "ChessBuddy" );
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_42, 0);

    // MAIN LOGO IMAGE OBJECT
    lv_obj_t *title_image = lv_image_create(start_screen);
    lv_image_set_src(title_image, &main_logo);
    lv_obj_align(title_image, LV_ALIGN_CENTER, 0, 0);
    lv_image_set_scale(title_image, 512);

    // Start button and label
    lv_obj_t *start_btn_label;
    lv_obj_t *start_btn = lv_button_create(start_screen);
    lv_obj_add_event_cb(start_btn, start_button_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(start_btn, LV_ALIGN_BOTTOM_MID, 0, -50);
    lv_obj_remove_flag(start_btn, LV_OBJ_FLAG_PRESS_LOCK);
    lv_obj_set_size(start_btn, 120, 50);
    lv_obj_add_style(start_btn, &generic_btn_style, 0);

    start_btn_label = lv_label_create(start_btn);
    lv_label_set_text(start_btn_label, "Start");
    lv_obj_center(start_btn_label);
    lv_obj_set_style_text_font(start_btn_label, &lv_font_montserrat_30, 0);

    // Settings button and label
    lv_obj_t* settings_btn = lv_btn_create(start_screen);
    lv_obj_set_size(settings_btn, 50, 50);
    lv_obj_align(settings_btn, LV_ALIGN_BOTTOM_RIGHT, -5, -5);
    lv_obj_add_style(settings_btn, &nobg_btn_style, 0);
    lv_obj_add_event_cb(settings_btn, settings_button_handler, LV_EVENT_ALL, NULL);

    lv_obj_t* settings_btn_icon = lv_label_create(settings_btn);
    lv_label_set_text(settings_btn_icon, LV_SYMBOL_SETTINGS);
    lv_obj_center(settings_btn_icon);
    lv_obj_set_style_text_font(settings_btn_icon, &lv_font_montserrat_30, 0);
}

void setup_wifi_prompt_screen() {
    wifi_prompt_screen = lv_obj_create(NULL);

    lv_obj_set_style_bg_color(wifi_prompt_screen, lv_color_hex(0x7295CA), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(wifi_prompt_screen, lv_color_hex(0x0D57A2), 0);
    lv_obj_set_style_bg_grad_dir(wifi_prompt_screen, LV_GRAD_DIR_HOR, 0);

    lv_obj_t *title_label = lv_label_create(wifi_prompt_screen);
    lv_label_set_text( title_label, "You must first connect your ChessBuddy to a wireless network in order to play!");
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xA3BECC), LV_PART_MAIN);
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0); 
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_36, 0);
    lv_label_set_long_mode(title_label, LV_LABEL_LONG_WRAP);  
    lv_obj_set_width(title_label, 300);                               

    // Settings button and label
    lv_obj_t* settings_btn = lv_btn_create(wifi_prompt_screen);
    lv_obj_set_size(settings_btn, 80, 80);
    lv_obj_align(settings_btn, LV_ALIGN_CENTER, 0, 80);
    lv_obj_add_style(settings_btn, &nobg_btn_style, 0);
    lv_obj_add_event_cb(settings_btn, settings_button_handler_special, LV_EVENT_ALL, NULL);

    lv_obj_t* settings_btn_icon = lv_label_create(settings_btn);
    lv_label_set_text(settings_btn_icon, LV_SYMBOL_SETTINGS);
    lv_obj_center(settings_btn_icon);
    lv_obj_set_style_text_font(settings_btn_icon, &lv_font_montserrat_42, 0);
}

static void execute_calibration_routine_handler_settings(lv_event_t * e)
{
    executeCalibrationData* calibrationData = (executeCalibrationData*) lv_event_get_user_data(e);
    runCalibrationRoutine();

    lv_obj_del(lv_obj_get_parent(calibrationData->message_box));
    lv_label_set_text(calibrationData->status_icon, LV_SYMBOL_OK);
    lv_obj_set_style_text_color(calibrationData->status_icon, lv_color_hex(0x00C853), LV_PART_MAIN);
    free(calibrationData);
}

static void run_calibration_handler_settings(lv_event_t * e)
{
    lv_obj_t * obj = (lv_obj_t *) lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);

    lv_obj_t * mbox1 = lv_msgbox_create(NULL);

    // struct to hold message box reference and to hold status icon for updating
    executeCalibrationData* calibrationData = (executeCalibrationData*) malloc(sizeof(executeCalibrationData));
    calibrationData->status_icon = obj;
    calibrationData->message_box = mbox1;

    lv_msgbox_add_title(mbox1, "Calibration");

    lv_msgbox_add_text(mbox1, "Please ensure any nearby objects are out of the way to give the arm clearance!");
    lv_msgbox_add_close_button(mbox1);

    lv_obj_t * btn;
    btn = lv_msgbox_add_footer_button(mbox1, "Run");
    lv_obj_add_event_cb(btn, execute_calibration_routine_handler_settings, LV_EVENT_CLICKED, calibrationData);
    return;
}

static lv_obj_t * create_text(lv_obj_t * parent, const char * icon, const char * txt,
                              lv_menu_builder_variant_t builder_variant, lv_font_t * fontSize = (lv_font_t *)&lv_font_montserrat_18, int iconScalingFactor = 256)
{
    lv_obj_t * obj = lv_menu_cont_create(parent);

    lv_obj_t * img = NULL;
    lv_obj_t * label = NULL;

    if(icon) {
        img = lv_image_create(obj);
        lv_image_set_src(img, icon);
        lv_image_set_scale(img, iconScalingFactor);
    }

    if(txt) {
        label = lv_label_create(obj);
        lv_label_set_text(label, txt);
        lv_label_set_long_mode(label, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
        lv_obj_set_flex_grow(label, 1);
        lv_obj_set_style_text_font(label, fontSize, 0);
    }

    if(builder_variant == LV_MENU_ITEM_BUILDER_VARIANT_2 && icon && txt) {
        lv_obj_add_flag(img, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
        lv_obj_swap(img, label);
    }

    return obj;
}

static void slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target_obj(e);
    struct SliderInfo * sliderInfo = (SliderInfo *)lv_event_get_user_data(e);
    char buf[16];
    uint32_t sliderParameter;

    if(sliderInfo->t == 0) {
        // changing velocity slider, in mm/s
        sliderParameter = lv_slider_get_value(slider) * (baseGearDiameter / stepsPerRevolution);
        lv_snprintf(buf, sizeof(buf), "%d mm/s", sliderParameter);
    } else {
        // changing acceleration slider, in mm/s²
        sliderParameter = lv_slider_get_value(slider) * 20;
        lv_snprintf(buf, sizeof(buf), "%d mm/s^2", sliderParameter);
    }
    lv_label_set_text(sliderInfo->slider_label, buf);
    //lv_obj_align_to(sliderInfo->slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}

static lv_obj_t * create_slider(lv_obj_t * parent, const char * icon, const char * txt, int32_t min, int32_t max,
                                int32_t val, lv_font_t * fontSize = (lv_font_t *)&lv_font_montserrat_18, int iconScalingFactor = 256)
{
    lv_obj_t * obj = create_text(parent, icon, txt, LV_MENU_ITEM_BUILDER_VARIANT_2, fontSize, iconScalingFactor);
    lv_obj_t * slider = lv_slider_create(obj);
    lv_obj_t * slider_label = lv_label_create(obj);
    lv_obj_remove_flag(slider_label, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    lv_obj_add_flag(slider_label, LV_OBJ_FLAG_FLOATING);
    char buf[16];
    uint32_t sliderParameter;

    struct SliderInfo* sliderInfo = new struct SliderInfo;
    sliderInfo->slider_label = slider_label;
    sliderInfo->val = val;

    if(strcmp(txt, "Velocity") == 0) {
        // changing velocity slider, in mm/s
        sliderInfo->t = 0;
        sliderParameter = referenceStepperSpeed * (baseGearDiameter / stepsPerRevolution);
        lv_snprintf(buf, sizeof(buf), "%d mm/s", sliderParameter);
    } else {
        // changing acceleration slider, in mm/s²
        sliderInfo->t = 1;
        sliderParameter = referenceStepperAccelScalar * 20;
        lv_snprintf(buf, sizeof(buf), "%d mm/s^2", sliderParameter);
    }

    lv_label_set_text(slider_label, buf);

    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, (void *) sliderInfo);
    lv_obj_set_flex_grow(slider, 1);
    lv_slider_set_range(slider, min, max);
    lv_slider_set_value(slider, val, LV_ANIM_OFF);
    
    lv_obj_set_style_pad_right(slider, 10, 0); 

    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    if(icon == NULL) {
        lv_obj_add_flag(slider, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    }

    return slider;
}



void setup_arm_mechanics_subpage(lv_obj_t * arm_mechanics_page) {
    lv_obj_set_flex_flow(arm_mechanics_page, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(arm_mechanics_page, 20, 0);

    lv_obj_t* calibration_cont = lv_obj_create(arm_mechanics_page);
    lv_obj_set_size(calibration_cont, 300, 150);
    lv_obj_set_flex_flow(calibration_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_style(calibration_cont, &calibration_container, LV_PART_MAIN);

    lv_obj_t* status_row = lv_obj_create(calibration_cont);
    lv_obj_set_size(status_row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_left(status_row, 10, 0);
    lv_obj_set_flex_flow(status_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_gap(status_row, 10, 0);
    lv_obj_set_style_border_width(status_row, 0, 0);
    lv_obj_set_style_bg_opa(status_row, LV_OPA_TRANSP, 0);

    // This shows current robotic arm calibration status
    lv_obj_t* status_label = lv_label_create(status_row);
    lv_label_set_text(status_label, "Calibration Status:");
    lv_obj_set_style_text_font(status_label, &lv_font_montserrat_20, 0);

    // Status icon depending on whether the arm has been actively calibrated 
    lv_obj_t* status_icon = lv_label_create(status_row);
    
    if(calibrationStatus) {
        lv_label_set_text(status_icon, LV_SYMBOL_OK);
         lv_obj_set_style_text_color(status_icon, lv_color_hex(0x00C853), LV_PART_MAIN);
    } else {
        lv_label_set_text(status_icon, LV_SYMBOL_CLOSE);
        lv_obj_set_style_text_color(status_icon, lv_color_hex(0xFF474C), LV_PART_MAIN);
    }

    lv_obj_set_style_text_font(status_icon, &lv_font_montserrat_20, 0);

    // calibration button
    lv_obj_t* calib_btn = lv_btn_create(calibration_cont);
    lv_obj_set_size(calib_btn, LV_PCT(100), 50);
    lv_obj_add_style(calib_btn, &generic_btn_style, LV_PART_MAIN);
    lv_obj_add_event_cb(calib_btn, run_calibration_handler_settings, LV_EVENT_CLICKED, status_icon);

    lv_obj_t* calib_btn_label = lv_label_create(calib_btn);
    lv_label_set_text(calib_btn_label, "Run Calibration Routine");
    lv_obj_center(calib_btn_label);
    lv_obj_set_style_text_font(calib_btn_label, &lv_font_montserrat_20, 0);

    lv_obj_t* other_arm_mechanics_cont = lv_obj_create(arm_mechanics_page);
    lv_obj_set_size(other_arm_mechanics_cont, 300, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(other_arm_mechanics_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_style(other_arm_mechanics_cont, &calibration_container, LV_PART_MAIN);

    // TODO: replace last number parameter on this slider
    lv_obj_t* velocity_slider = create_slider(other_arm_mechanics_cont, (const char *)&lightning, "Velocity", minStepperSpeed, maxStepperSpeed, referenceStepperSpeed);
    lv_obj_add_style(velocity_slider, &temp_slider, LV_PART_MAIN);
    lv_obj_add_style(velocity_slider, &temp_slider, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(velocity_slider, lv_color_black(), LV_PART_KNOB);
    lv_obj_t* accel_slider = create_slider(other_arm_mechanics_cont, (const char *)&rapid_clock, "Acceleration", minStepperAccel, maxStepperAccel, referenceStepperAccelScalar);
    lv_obj_add_style(accel_slider, &temp_slider, LV_PART_MAIN);
    lv_obj_add_style(accel_slider, &temp_slider, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(accel_slider, lv_color_black(), LV_PART_KNOB);
}

void setup_settings_screen() {
    settings_screen = lv_obj_create(NULL);

    settings_menu = lv_menu_create(settings_screen);
    lv_menu_set_mode_root_back_button(settings_menu, LV_MENU_ROOT_BACK_BUTTON_ENABLED);
    lv_obj_set_style_bg_color(settings_menu, lv_color_hex(0x7295CA), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(settings_menu, lv_color_hex(0x0D57A2), 0);
    lv_obj_set_style_bg_grad_dir(settings_menu, LV_GRAD_DIR_VER, 0);

    lv_obj_add_event_cb(settings_menu, back_event_handler, LV_EVENT_CLICKED, settings_menu);
    lv_obj_set_size(settings_menu, 320, 480);
    lv_obj_center(settings_menu);


    // lv_obj_t * root_page = lv_menu_page_create(settings_menu, "Settings");
    // lv_menu_set_page(settings_menu, root_page);
    //section = lv_menu_section_create(root_page);

    lv_obj_t * back_btn = lv_menu_get_main_header_back_button(settings_menu);

    lv_obj_set_size(back_btn, 30, 40);  // Much larger touch target (was likely around 30x30)
    lv_obj_set_style_pad_all(back_btn, 7, 0);  // Add plenty of padding
    // lv_obj_set_style_radius(back_btn, 10, 0);   // Rounded corners
    lv_obj_t * back_icon = lv_obj_get_child(back_btn, 0);
    if(back_icon != NULL) {
        lv_obj_set_style_text_font(back_icon, &lv_font_montserrat_24, 0);  // Much larger icon
        lv_obj_center(back_icon);
    }
    
    lv_obj_t * cont;
    lv_obj_t * label;
    lv_obj_t * section;


    /*Create wifi sub page*/
    wifi_sub_page = lv_menu_page_create(settings_menu, NULL);

    /*Create arm mechanics sub page*/
    lv_obj_t * arm_mechanics_sub_page = lv_menu_page_create(settings_menu, "Arm Mechanics");
    setup_arm_mechanics_subpage(arm_mechanics_sub_page);

    lv_obj_t * chessboard_sub_page = lv_menu_page_create(settings_menu, "Board Settings");
    //setup_board_subpage(chessboard_sub_page);

    /*Create a main page*/
    main_page = lv_menu_page_create(settings_menu, "Settings");
    
    // top section
    section = lv_menu_section_create(main_page);

    // wifi settings settings_menu item
    cont = lv_menu_cont_create(main_page);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, LV_PART_MAIN);
    
    lv_menu_set_load_page_event(settings_menu, cont, wifi_sub_page);

    lv_obj_add_event_cb(cont, wifi_submenu_handler, LV_EVENT_CLICKED, NULL);

    lv_obj_t *wifi_full_strength_image = lv_image_create(cont);
    lv_image_set_src(wifi_full_strength_image, &wifi_full_strength);
    
    lv_obj_t *wifi_cont_label = lv_label_create(cont);
    lv_label_set_text(wifi_cont_label, "Wi-Fi");
    lv_label_set_long_mode(wifi_cont_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_flex_grow(wifi_cont_label, 1);

    // display settings settings_menu item
    cont = lv_menu_cont_create(main_page);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, LV_PART_MAIN);
    
    lv_obj_t *monitor_image = lv_image_create(cont);
    lv_image_set_src(monitor_image, &monitor);

    lv_obj_t *display_cont_label = lv_label_create(cont);
    lv_label_set_text(display_cont_label, "Display Configuration");
    lv_label_set_long_mode(display_cont_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_flex_grow(display_cont_label, 1);

    // ROBOTIC ARM MECHANICS SETTINGS PAGE (IMPORTANT)
    // for settings related to the chess buddy arm
    cont = lv_menu_cont_create(main_page);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, LV_PART_MAIN);

    lv_menu_set_load_page_event(settings_menu, cont, arm_mechanics_sub_page);
    
    lv_obj_t *robotic_arm_image = lv_image_create(cont);
    lv_image_set_src(robotic_arm_image, &robotic_arm);

    lv_obj_t *robotic_arm_cont_label = lv_label_create(cont);
    lv_label_set_text(robotic_arm_cont_label, "Arm Mechanics");
    lv_label_set_long_mode(robotic_arm_cont_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_flex_grow(robotic_arm_cont_label, 1);

    // ELECTRONIC CHESS BOARD SETTINGS PAGE
    // settings related to the configuration/behavior of the chessboard
    cont = lv_menu_cont_create(main_page);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, LV_PART_MAIN);

    lv_menu_set_load_page_event(settings_menu, cont, chessboard_sub_page);

    lv_obj_t *chessboard_image = lv_image_create(cont);
    lv_image_set_src(chessboard_image, &checkerboard);

    lv_obj_t *chessboard_cont_label = lv_label_create(cont);
    lv_label_set_text(chessboard_cont_label, "Board Settings");
    lv_label_set_long_mode(chessboard_cont_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_flex_grow(chessboard_cont_label, 1);


    lv_menu_set_page(settings_menu, main_page);
}

void setup_screen_template(lv_obj_t * screen, char* title) {
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x7295CA), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(screen, lv_color_hex(0x0D57A2), 0);
    lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_VER, 0);

     // Create back button
    lv_obj_t *back_btn = lv_btn_create(screen);
    lv_obj_set_size(back_btn, 30, 40);
    lv_obj_align(back_btn, LV_ALIGN_TOP_LEFT, 6, 4);
    lv_obj_set_style_pad_all(back_btn, 7, 0);
    lv_obj_add_event_cb(back_btn, default_back_btn_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_style(back_btn, &nobg_btn_style, 0);

    // Icon label
    lv_obj_t *back_icon = lv_label_create(back_btn);
    lv_label_set_text(back_icon, LV_SYMBOL_LEFT);  // Arrow icon
    lv_obj_center(back_icon);
    lv_obj_set_style_text_font(back_icon, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(back_icon, lv_color_hex(0x00000), 0);


    lv_obj_t *title_label = lv_label_create(screen);
    lv_label_set_text(title_label, title);
    lv_obj_align(title_label, LV_ALIGN_TOP_LEFT, 50, 14);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xffffff), 0);
}

static void side_select_btn_handler(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target_obj(e);
    char * selected_side = (char *)lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
        Serial.write(selected_side);
        gameInfo->side_to_play = selected_side;

        // switch to difficulty screen
        //switch_to_difficulty_screen();
        switch_to_screen(difficulty_screen);
    }
}

void setup_side_select_screen() {
    side_select_screen =  lv_obj_create(NULL);

    setup_screen_template(side_select_screen, "Choose Your Side");

    lv_obj_t * parent = lv_obj_create(side_select_screen);
    lv_obj_set_size(parent, 320, 440);
    lv_obj_align(parent, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(parent, 10, 0);

    lv_obj_set_style_bg_opa(parent, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(parent, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    const char* sideLabels[5] = {
        "White",
        "Black"
    };

    const lv_image_dsc_t * sideIcons[5] {
        &white_king_large,
        &black_king_large
    };

    for(int i = 0; i < 2; i++) {
        lv_obj_t * cont = lv_btn_create(parent);
        lv_obj_set_size(cont, 180, 180);
        lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_add_event_cb(cont, side_select_btn_handler, LV_EVENT_CLICKED, (void *)sideLabels[i]);
        lv_obj_set_style_radius(cont, 8, 0);
        lv_obj_set_style_bg_color(cont, lv_color_hex(0x00547B), 0);
        lv_obj_set_style_pad_all(cont, 10, 0);

        lv_obj_t * icon = lv_image_create(cont);
        lv_img_set_src(icon, sideIcons[i]);
        lv_image_set_scale(icon, 256);  // REMINDER 256 is normal scale, 512 is double, 128 is half

        // Label (right)
        lv_obj_t * label = lv_label_create(cont);

        lv_label_set_text(label, sideLabels[i]);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0xA3BECC), 0);
        lv_obj_set_width(label, 180);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    }

}


static void difficulty_btn_handler(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target_obj(e);
    char * selected_difficulty = (char *)lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
        Serial.write(selected_difficulty);
        gameInfo->difficulty = selected_difficulty;

        // SWITCH TO TIME CONTROL SCREEN
        //switch_to_time_control_screen();
        switch_to_screen(time_control_screen);
    }
}

void setup_difficulty_screen() {
    difficulty_screen = lv_obj_create(NULL);

    setup_screen_template(difficulty_screen, "Choose Your Difficulty");

    lv_obj_t * parent = lv_obj_create(difficulty_screen);
    lv_obj_set_size(parent, 320, 440);
    lv_obj_align(parent, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(parent, 10, 0);

    lv_obj_set_style_bg_opa(parent, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(parent, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    const char* difficultyLevels[5] = {
        "Beginner",
        "Intermediate",
        "Advanced",
        "Expert",
        "Grandmaster"
    };

    const lv_image_dsc_t * difficultyIcons[5] {
        &black_pawn,
        &black_knight,
        &black_rook,
        &black_queen,
        &black_king
    };

    // Loop to create 5 containers
    for(int i = 0; i < 5; i++) {
        // Create a horizontal container
        lv_obj_t * cont = lv_btn_create(parent);
        lv_obj_set_size(cont, 300, 60);
        lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_add_event_cb(cont, difficulty_btn_handler, LV_EVENT_CLICKED, (void *)difficultyLevels[i]);
        lv_obj_set_style_radius(cont, 8, 0);
        lv_obj_set_style_bg_color(cont, lv_color_hex(0x00547B), 0);
        lv_obj_set_style_pad_all(cont, 10, 0);

        // Icon (left)
        // lv_obj_t * icon = lv_label_create(cont);
        // lv_label_set_text(icon, difficultyIcons[i]);
        //lv_obj_set_style_text_font(icon, &lv_font_montserrat_30, 0);
        // lv_image_set_scale(title_image, 256); 
        //lv_obj_set_style_text_color(icon, lv_color_hex(0xA3BECC), 0);

        lv_obj_t * icon = lv_image_create(cont);
        lv_img_set_src(icon, difficultyIcons[i]);
        lv_image_set_scale(icon, 256);  // REMINDER 256 is normal scale, 512 is double, 128 is half

        // Label (right)
        lv_obj_t * label = lv_label_create(cont);

        lv_label_set_text(label, difficultyLevels[i]);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0xA3BECC), 0);
        lv_obj_set_width(label, 220);  // Optional width to control wrapping if needed
        //lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);    
    }
}

// TimeControl map_time_control_labels(char * label) {
//     if (strcmp(label, "5 Min (Blitz)") == 0) {
//         return TimeControl::FIVEMINBLITZ;
//     } else if (strcmp(label, "10 Min (Rapid)") == 0) {
//         return TimeControl::TENMINRAPID;
//     } else if (strcmp(label, "30 Min (Rapid)") == 0) {
//         return TimeControl::THIRTYMINRAPID;
//     }
//     return TimeControl::TENMINRAPID;
// }

char * get_time_control_label(TimeControl time_control_value) {
    switch(time_control_value) {
        case(TimeControl::FIVEMINBLITZ):
            return "5 Min (Blitz)";
        case(TimeControl::TENMINRAPID):
            return "10 Min (Rapid)";
        case (TimeControl::THIRTYMINRAPID):
            return "30 Min (Rapid)";
    }
    return "10 Min (Rapid)";
}


static void time_control_btn_handler(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target_obj(e);
    TimeControl time_control = *(TimeControl *)lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
        gameInfo->time_control = time_control;

        // SWITCH TO START GAME SCREEN
        //switch_to_time_control_screen();
        setup_start_game_screen();
        switch_to_screen(start_game_screen);

       
        // Serial.print("Difficulty: ");
        // Serial.println(gameInfo->difficulty);

        // Serial.print("Side to play: ");
        // Serial.println(gameInfo->side_to_play);

        // Serial.print("Time control: ");
        // Serial.println(map_time_control_labels(gameInfo->time_control));
    }
}

void setup_time_control_screen() {
    time_control_screen = lv_obj_create(NULL);

    setup_screen_template(time_control_screen, "Choose Time Control");

    lv_obj_t * parent = lv_obj_create(time_control_screen);
    lv_obj_set_size(parent, 320, 440);
    lv_obj_align(parent, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(parent, 10, 0);

    lv_obj_set_style_bg_opa(parent, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(parent, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    static TimeControl timeControls[3] = {
        FIVEMINBLITZ,
        TENMINRAPID,
        THIRTYMINRAPID
    };

    const lv_image_dsc_t * timeControlIcons[3] {
        &lightning,
        &rapid_clock,
        &rapid_clock,
    };

    // Loop to create time control containers
    for(int i = 0; i < 3; i++) {
        // Create a horizontal container
        lv_obj_t * cont = lv_btn_create(parent);
        lv_obj_set_size(cont, 250, 80);
        lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_add_event_cb(cont, time_control_btn_handler, LV_EVENT_CLICKED, (void *)&timeControls[i]);
        lv_obj_set_style_radius(cont, 40, 0);
        lv_obj_set_style_bg_color(cont, lv_color_hex(0x00547B), 0);
        lv_obj_set_style_pad_all(cont, 10, 0);

        // Icon (left)
        // lv_obj_t * icon = lv_label_create(cont);
        // lv_label_set_text(icon, difficultyIcons[i]);
        //lv_obj_set_style_text_font(icon, &lv_font_montserrat_30, 0);
        // lv_image_set_scale(title_image, 256); 
        //lv_obj_set_style_text_color(icon, lv_color_hex(0xA3BECC), 0);

        lv_obj_t * icon = lv_image_create(cont);
        lv_img_set_src(icon, timeControlIcons[i]);
        lv_image_set_scale(icon, 256);  // REMINDER 256 is normal scale, 512 is double, 128 is half

        // Label (right)
        lv_obj_t * label = lv_label_create(cont);

        lv_label_set_text(label, get_time_control_label(timeControls[i]));
        lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0xA3BECC), 0);
        lv_obj_set_width(label, 200);  // Optional width to control wrapping if needed
        //lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);    
    }
}

static void execute_calibration_routine_handler(lv_event_t * e)
{
    lv_obj_t* message_box = (lv_obj_t*) lv_event_get_user_data(e);
    
    runCalibrationRoutine();

    lv_obj_del(lv_obj_get_parent(message_box));
}

static void calibration_handler_popup()
{
    lv_obj_t * mbox1 = lv_msgbox_create(NULL);

    lv_msgbox_add_title(mbox1, "Calibration");

    lv_msgbox_add_text(mbox1, "You must calibrate before playing a game! Please ensure any nearby objects are out of the way to give the arm clearance!");
    lv_msgbox_add_close_button(mbox1);

    lv_obj_t * btn;
    btn = lv_msgbox_add_footer_button(mbox1, "Run");
    lv_obj_add_event_cb(btn, execute_calibration_routine_handler, LV_EVENT_CLICKED, mbox1);
    return;
}

void start_game_btn_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");

        // if arm is calibrated, proceed to game start, else pop up message
        if(calibrationStatus == true) {
            setup_active_game_screen();
            switch_to_screen(active_game_screen);

            // initialize the board for a game to start
            boardStartNewGame();
        } else {
            calibration_handler_popup();
        }
    }
}

void setup_start_game_screen() {

    if(start_game_screen != NULL) {
        lv_obj_del(start_game_screen); 
    }

    start_game_screen = lv_obj_create(NULL);
    setup_screen_template(start_game_screen, "");
    

    lv_obj_t * start_game_btn = lv_btn_create(start_game_screen);
    lv_obj_set_size(start_game_btn, 250, 80);

    lv_obj_set_style_radius(start_game_btn, 40, 0);
    lv_obj_set_style_bg_color(start_game_btn, lv_color_hex(0x008000), 0);
    lv_obj_set_style_pad_all(start_game_btn, 10, 0);
    lv_obj_set_style_border_opa(start_game_btn, LV_OPA_TRANSP, 0);
    lv_obj_align(start_game_btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(start_game_btn, start_game_btn_handler, LV_EVENT_CLICKED, NULL);

    lv_obj_t * start_game_btn_label = lv_label_create(start_game_btn);
    lv_label_set_text(start_game_btn_label, "Start Game");
    lv_obj_set_style_text_font(start_game_btn_label, &lv_font_montserrat_30, 0);
    lv_obj_set_style_text_color(start_game_btn_label, lv_color_hex(0xffffff), 0);
    lv_obj_center(start_game_btn_label);

    lv_obj_t * spacer = lv_obj_create(start_game_screen);
    lv_obj_align(spacer, LV_ALIGN_CENTER, 0, 70);
    lv_obj_set_size(spacer, 300, 2);
    lv_obj_set_style_bg_color(spacer, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_border_opa(spacer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_radius(spacer, 50, 0);

    char buffer[100];

    snprintf(buffer, sizeof(buffer), "Game Options: \nDifficulty: %s\nSide (User): %s\nTime Control: %s", gameInfo->difficulty, gameInfo->side_to_play, get_time_control_label(gameInfo->time_control));


    lv_obj_t * game_options_label = lv_label_create(start_game_screen);
    lv_label_set_text(game_options_label, buffer);
    lv_obj_set_style_text_font(game_options_label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(game_options_label, lv_color_hex(0xffffff), 0);
    lv_obj_align(game_options_label, LV_ALIGN_CENTER, 0, 120);

}

void clock_timer(lv_timer_t * timer)
{
    lv_obj_t * selected_clock_label = (lv_obj_t *) lv_timer_get_user_data(timer);

    int minutes;
    int seconds;

    if(userSideToMove) {
        if (user_total_seconds > 0) {
            user_total_seconds--;
        }
        minutes = user_total_seconds / 60;
        seconds = user_total_seconds % 60;
    } else {
        if (computer_total_seconds > 0) {
            computer_total_seconds--;
        }
        minutes = computer_total_seconds / 60;
        seconds = computer_total_seconds % 60;
    }

    

    //Serial.write("Decrementing...");
    char clk_buf[10];
    snprintf(clk_buf, sizeof(clk_buf), "%02d:%02d", minutes, seconds);

    lv_label_set_text(selected_clock_label, clk_buf);

}


void end_turn_btn_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    SidesContainer * sides = (SidesContainer *)lv_event_get_user_data(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
        Serial.print("Clicked: ");

        lv_obj_remove_style(sides->computer_side_container, &inactive_timer, LV_PART_MAIN);
        lv_obj_add_style(sides->computer_side_container, &active_timer, LV_PART_MAIN);

        lv_obj_add_style(sides->user_side_container, &inactive_timer, LV_PART_MAIN);
        lv_obj_remove_style(sides->user_side_container, &active_timer, LV_PART_MAIN);


        // pause the user timer and start the computer's clock
        lv_timer_pause(user_timer);
        lv_timer_resume(computer_timer);
        

        // swap to computer move
        //userSideToMove = false;

        // scan last time to get finalized move
        scanningUserMove(true, true);

        printMoveHistory();

        resetPieceDetectionParameters();

        //non-blocking task for the http request to get a move
        xTaskCreatePinnedToCore(
            getBestMoveTask,     // Task function
            "GetBestMoveTask",   // Name
            8192,               // Stack size (in words, 4 bytes each) — adjust as needed
            NULL,                // Parameters
            3,                   // Priority
            NULL,                // Task handle
            0                    // Core ID (0 = good for background/IO tasks)
        );
        //getBestMoveFromServer();
    }
}

// this is called outside to update the UI when the robot is finished executing its move
void end_engine_turn_handler() {
    lv_obj_remove_style(sides_container.computer_side_container, &active_timer, LV_PART_MAIN);
    lv_obj_add_style(sides_container.computer_side_container, &inactive_timer, LV_PART_MAIN);

    lv_obj_add_style(sides_container.user_side_container, &active_timer, LV_PART_MAIN);
    lv_obj_remove_style(sides_container.user_side_container, &inactive_timer, LV_PART_MAIN);


    // pause the user timer and start the computer's clock
    lv_timer_pause(computer_timer);
    lv_timer_resume(user_timer);
}

void end_game_button_handler(lv_event_t * e) {
    // TODO: clear the stack if this will switch you back to start screen, else dont
    activeGame = false;
    switch_to_screen(start_screen);
}

///
/// THIS CONTROLS INTERFACING WITH BOARDCONTROL TO SET UP A NEW GAME
///
void set_active_game_state() {
    if(active_game_screen != NULL) {
        lv_obj_del(active_game_screen); 
        active_game_screen = NULL;
    }

    if (user_timer != NULL) {
        lv_timer_del(user_timer);
        user_timer = NULL;
    }
    if (computer_timer != NULL) {
        lv_timer_del(computer_timer);
        computer_timer = NULL;
    }

}



// THIS GUI FUNCTION IS EXTRA IMPORTANT AS IT INTERACTS DIRECTLY WITH THE CHESS BOARD ELECTRONICS CODE
void setup_active_game_screen() {

    // TODO: maybe need to do some checks here first to ensure the pieces are in the correct starting arrangement, etc, etc
    set_active_game_state();

    active_game_screen = lv_obj_create(NULL);

    lv_obj_set_style_bg_color(active_game_screen, lv_color_hex(0x7295CA), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(active_game_screen, lv_color_hex(0x0D57A2), 0);
    lv_obj_set_style_bg_grad_dir(active_game_screen, LV_GRAD_DIR_HOR, 0);

    lv_obj_t * top_cont = lv_obj_create(active_game_screen);
    lv_obj_align(top_cont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_size(top_cont, 320, 239);
    // lv_obj_set_style_bg_opa(top_cont, LV_OPA_TRANSP, 0);
    // lv_obj_set_style_border_opa(top_cont, LV_OPA_TRANSP, 0);
    //lv_obj_remove_style(top_cont, NULL, LV_PART_MAIN | LV_STATE_PRESSED);

    // USER IS GOING FIRST THE TOP CONTAINER SHOULD BE DARKENED INITIALLY (IF THE USER IS PLAYING WHITE)
    lv_obj_add_style(top_cont, &inactive_timer, LV_PART_MAIN);

    lv_obj_t * bottom_button = lv_btn_create(active_game_screen);
    lv_obj_align(bottom_button, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_size(bottom_button, 320, 239);
    //lv_obj_remove_style(bottom_button, NULL, LV_PART_MAIN | LV_STATE_PRESSED);

    // THE ACTIVE TIMER STYLE IS SET INITIALLY IF THE USER IS PLAYING WHITE
    lv_obj_add_style(bottom_button, &active_timer, LV_PART_MAIN);

    // end game button and label
    lv_obj_t *end_game_btn = lv_button_create(active_game_screen);
    lv_obj_add_event_cb(end_game_btn, end_game_button_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_align(end_game_btn, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_remove_flag(end_game_btn, LV_OBJ_FLAG_PRESS_LOCK);
    lv_obj_set_size(end_game_btn, 150, 50);
    lv_obj_add_style(end_game_btn, &alert_btn_style, 0);

    lv_obj_t *end_game_btn_label = lv_label_create(end_game_btn);
    lv_label_set_text(end_game_btn_label, "END GAME");
    lv_obj_center(end_game_btn_label);
    lv_obj_set_style_text_font(end_game_btn_label, &lv_font_montserrat_24, 0);

    lv_obj_t * spacer = lv_obj_create(active_game_screen);
    lv_obj_align(spacer, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(spacer, 315, 2);
    lv_obj_set_style_bg_color(spacer, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_border_opa(spacer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_radius(spacer, 50, 0);

    
    user_total_seconds = gameInfo->time_control;
    user_minutes = user_total_seconds / 60;
    user_seconds = user_total_seconds % 60;

    char user_clk_buffer[10];
    snprintf(user_clk_buffer, sizeof(user_clk_buffer), "%02d:%02d", user_minutes, user_seconds);


    lv_obj_t * user_clock = lv_label_create(bottom_button);
    lv_obj_center(user_clock);
    lv_label_set_text(user_clock, user_clk_buffer);
    lv_obj_set_style_text_font(user_clock, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(user_clock, lv_color_hex(0xffffff), 0);

    computer_total_seconds = gameInfo->time_control;
    computer_minutes = computer_total_seconds / 60;
    computer_seconds = computer_total_seconds % 60;

    char computer_clk_buffer[10];
    snprintf(computer_clk_buffer, sizeof(computer_clk_buffer), "%02d:%02d", computer_minutes, computer_seconds);

    lv_obj_t * computer_clock = lv_label_create(top_cont);
    lv_obj_center(computer_clock);
    lv_label_set_text(computer_clock, computer_clk_buffer);
    lv_obj_set_style_text_font(computer_clock, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(computer_clock, lv_color_hex(0xffffff), 0);

    // every 1 second, the clock timer decrements for whoever's side it is
    user_timer = lv_timer_create(clock_timer, 1000, (void *)user_clock);
    lv_timer_pause(user_timer);
    computer_timer = lv_timer_create(clock_timer, 1000, (void*)computer_clock);
    lv_timer_pause(computer_timer);

    sides_container.user_side_container = bottom_button;
    sides_container.computer_side_container = top_cont;

    lv_obj_add_event_cb(bottom_button, end_turn_btn_handler, LV_EVENT_CLICKED, (void*)&sides_container);
}

void switch_to_screen(lv_obj_t* new_screen) {
    lv_obj_t* current = lv_screen_active();
    screen_stack.push(current); 
    lv_screen_load(new_screen); 
}

void go_back_screen() {
    if (!screen_stack.empty()) {
        lv_obj_t* prev_screen = screen_stack.top();
        screen_stack.pop();
        lv_scr_load(prev_screen);
    } else {
        Serial.println("No previous screen in stack.");
    }
}

void switch_to_start() {
    lv_screen_load(start_screen);
}


void initializeGUI() {
    setup_top_layer();
    setup_start_screen();
    setup_wifi_prompt_screen();
    setup_settings_screen();
    setup_difficulty_screen();
    setup_side_select_screen();
    setup_time_control_screen();
    //setup_start_game_screen();
}
