#ifndef GUI_GATEWAY_H
#define GUI_GATEWAY_H

#include <lvgl.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <esp_task_wdt.h>
#include "gui.h"
#include "wlan.h"

enum GuiActionType {
    GUI_ACTION_SWITCH_SCREEN,
    GUI_ACTION_GET_ACTIVE_SCREEN,
    GUI_ACTION_UPDATE_LABEL,
    GUI_ACTION_UPDATE_TIMER,
    GUI_ACTION_UPDATE_WIFI_ICON,
    GUI_ACTION_UPDATE_WIFI_LIST,
    GUI_ACTION_END_ENGINE_TURN
};

struct GuiMessage {
    GuiActionType action;
    void* data;
};

struct GuiGetActiveScreenRequest {
    QueueHandle_t responseQueue;
};

struct WifiUpdateData {
    wl_status_t wifiStatus;
};

struct WifiListUpdateData {
    int networkCount;
    Network* networks;
};


/* FUNCTION DECLARATIONS */

void start_gui_gateway_task();
void request_screen_switch(lv_obj_t* screen);
void request_label_update(lv_obj_t* label, const char* newText);
lv_obj_t* get_active_screen();
void request_wifi_icon_update(wl_status_t wifiStatus);
void request_wifi_list_update(int networkCount, struct Network* networks);
void request_end_engine_turn();

#endif