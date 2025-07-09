#include "gui_gateway.h"

QueueHandle_t guiQueue;

static void lvgl_gateway_task(void *pvParameters) {
    const TickType_t delay = pdMS_TO_TICKS(5);
    GuiMessage msg;
    int loopCount = 0;
    while (1) {
        while (xQueueReceive(guiQueue, &msg, 0) == pdTRUE) {
            switch (msg.action) {
                case GUI_ACTION_SWITCH_SCREEN:
                    lv_scr_load(static_cast<lv_obj_t*>(msg.data));
                    break;

                case GUI_ACTION_UPDATE_LABEL: {
                    struct LabelUpdateData {
                        lv_obj_t* label;
                        char text[64];
                    };
                    auto* data = static_cast<LabelUpdateData*>(msg.data);
                    lv_label_set_text(data->label, data->text);
                    delete data;
                    break;
                }
                case GUI_ACTION_GET_ACTIVE_SCREEN: {
                    GuiGetActiveScreenRequest* request = static_cast<GuiGetActiveScreenRequest*>(msg.data);
                    lv_obj_t* active_screen = lv_scr_act();  // safely called inside GUI thread

                    // Send the result back
                    xQueueSend(request->responseQueue, &active_screen, 0);

                    // Clean up the request struct allocated by sender
                    delete request;
                    break;
                }
                case GUI_ACTION_UPDATE_WIFI_ICON: {
                    WifiUpdateData* data = static_cast<WifiUpdateData*>(msg.data);
                    updateWifiWidget(data->wifiStatus);
                    delete data;
                    break;
                }
                case GUI_ACTION_UPDATE_WIFI_LIST: {
                    WifiListUpdateData* data = static_cast<WifiListUpdateData*>(msg.data);
                    updateWifiNetworkList(data->networkCount, data->networks);
                    delete data;
                    break;
                }
                case GUI_ACTION_END_ENGINE_TURN: {
                    end_engine_turn_handler();
                }
                default:
                    break;
            }
        }

        lv_timer_handler();
        vTaskDelay(delay);

        // DEBUGGING TO CHECK STACK SIZE
        // if (++loopCount % 100 == 0) {
        //     UBaseType_t watermark = uxTaskGetStackHighWaterMark(NULL);
        //     Serial.printf("[LVGL Task] Stack watermark: %u bytes\n", watermark);
        // }
    }
}

void start_gui_gateway_task() {
    guiQueue = xQueueCreate(10, sizeof(GuiMessage));
    xTaskCreatePinnedToCore(
        lvgl_gateway_task,
        "LVGL Gateway Task",
        8192,
        NULL,
        2,
        NULL,
        1
    );
}

void request_screen_switch(lv_obj_t* screen) {
    GuiMessage msg = {
        .action = GUI_ACTION_SWITCH_SCREEN,
        .data = screen
    };
    xQueueSend(guiQueue, &msg, portMAX_DELAY);
}

void request_label_update(lv_obj_t* label, const char* newText) {
    struct LabelUpdateData {
        lv_obj_t* label;
        char text[64];
    };

    auto* data = new LabelUpdateData;
    data->label = label;
    strncpy(data->text, newText, sizeof(data->text));
    data->text[sizeof(data->text) - 1] = '\0';

    GuiMessage msg = {
        .action = GUI_ACTION_UPDATE_LABEL,
        .data = data
    };

    xQueueSend(guiQueue, &msg, portMAX_DELAY);
}

lv_obj_t* get_active_screen() {
    lv_obj_t* result = nullptr;

    // Create a queue for receiving the response
    QueueHandle_t responseQueue = xQueueCreate(1, sizeof(lv_obj_t*));

    GuiGetActiveScreenRequest* request = new GuiGetActiveScreenRequest;
    request->responseQueue = responseQueue;

    GuiMessage msg = {
        .action = GUI_ACTION_GET_ACTIVE_SCREEN,
        .data = request
    };

    // Send the request to the GUI task
    xQueueSend(guiQueue, &msg, portMAX_DELAY);

    // Wait for the response (timeout can be added for safety)
    if(xQueueReceive(responseQueue, &result, pdMS_TO_TICKS(1000)) != pdTRUE) {
        // Timeout or error: handle appropriately (e.g., return nullptr or last known screen)
        result = nullptr;
    }

    vQueueDelete(responseQueue);
    return result;
}

void request_wifi_icon_update(wl_status_t wifiStatus) {
    WifiUpdateData* data = new WifiUpdateData{wifiStatus};

    GuiMessage msg = {
        .action = GUI_ACTION_UPDATE_WIFI_ICON,
        .data = data
    };

    xQueueSend(guiQueue, &msg, portMAX_DELAY);
}

void request_wifi_list_update(int networkCount, struct Network* networks) {
    WifiListUpdateData* data = new WifiListUpdateData{
        .networkCount = networkCount,
        .networks = networks
    };

    GuiMessage msg = {
        .action = GUI_ACTION_UPDATE_WIFI_LIST,
        .data = data
    };

    xQueueSend(guiQueue, &msg, portMAX_DELAY);
}

void request_end_engine_turn() {
    GuiMessage msg = {
        .action = GUI_ACTION_END_ENGINE_TURN,
        .data = NULL
    };
    xQueueSend(guiQueue, &msg, portMAX_DELAY);
}