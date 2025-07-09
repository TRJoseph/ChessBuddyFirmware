#include <TFT_eSPI.h>        // Graphics and display library
#include "FT6336U.h"         // Touch controller
#include <Main_Definitions.h>
#include <lvgl.h>
#include "gui.h"
#include "wlan.h"
#include "gui_gateway.h"
#include "serverInterface.h"


/*Set to your screen resolution and rotation*/
#define TFT_HOR_RES   320
#define TFT_VER_RES   480
#define TFT_ROTATION  LV_DISPLAY_ROTATION_0

/*LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes*/
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

#if LV_USE_LOG != 0
void my_print( lv_log_level_t level, const char * buf )
{
    LV_UNUSED(level);
    Serial.println(buf);
    Serial.flush();
}
#endif

void setup()
{
    String LVGL_Arduino = String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

    Serial.begin( 115200 );
    Serial.println( LVGL_Arduino );

    start_touch_object();

    lv_init();

    /*Set a tick source so that LVGL will know how much time elapsed. */
    lv_tick_set_cb(my_tick);

    /* register print function for debugging */
    #if LV_USE_LOG != 0
        lv_log_register_print_cb( my_print );
    #endif

    lv_display_t * disp;

    disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, draw_buf, sizeof(draw_buf));
    lv_display_set_rotation(disp, TFT_ROTATION);

    /*Initialize the (dummy) input device driver*/
    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
    lv_indev_set_read_cb(indev, my_touch_read);

    // setup wifi preferences and credentials
    setup_preferences();

    /* Starts the ChessBuddy GUI */
    initializeGUI();

    /* Initializes the board and arm setup configuration*/
    setupBoard();

    /* Changes to the start screen */
    switch_to_start();

    // starts the gateway thread for GUI
    start_gui_gateway_task();

    /* */
    Serial.println( "Setup done" );
}

void loop()
{
    scanningUserMove(userSideToMove, false);
    delay(10);
}