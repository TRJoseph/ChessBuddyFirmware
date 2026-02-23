#ifndef GUI_THEME_H
#define GUI_THEME_H

#include <lvgl.h>

/*
 * ChessBuddy GUI Color Theme
 * Change these values to restyle the entire application.
 */

/* --- Background Gradient --- */
/* Screen background: horizontal/vertical blue gradient */
#define COLOR_BG_START      lv_color_hex(0x5BA8D6)  /* vibrant sky blue */
#define COLOR_BG_END        lv_color_hex(0x0A2E6A)  /* deep navy       */

/* --- Buttons --- */
/* Dark navy — primary action buttons (Start, etc.) */
#define COLOR_BTN_PRIMARY   lv_color_hex(0x041941)
/* Teal — option/selection card buttons (side, difficulty, time control) */
#define COLOR_BTN_ACCENT    lv_color_hex(0x00547B)
/* Red — destructive action buttons (End Game alert style) */
#define COLOR_BTN_DANGER    lv_color_hex(0xFF3131)
/* Green — start game confirmation button */
#define COLOR_BTN_START     lv_color_hex(0x008000)

/* --- Text --- */
/* White text used on dark/gradient backgrounds */
#define COLOR_TEXT_ON_DARK  lv_color_hex(0xFFFFFF)
/* Muted blue-grey — secondary/subtitle text on accent buttons */
#define COLOR_TEXT_MUTED    lv_color_hex(0xA3BECC)
/* Black — text on light surfaces */
#define COLOR_TEXT_DARK     lv_color_hex(0x000000)

/* --- Status Indicators --- */
/* Solid green — success icons/checkmarks */
#define COLOR_SUCCESS       lv_color_hex(0x00C853)
/* Light green — success message text */
#define COLOR_SUCCESS_TEXT  lv_color_hex(0x80EF80)
/* Red — error icons and message text */
#define COLOR_ERROR         lv_color_hex(0xFF474C)

/* --- Surfaces --- */
/* White — card/container backgrounds inside menus */
#define COLOR_SURFACE       lv_color_hex(0xFFFFFF)

#endif /* GUI_THEME_H */
