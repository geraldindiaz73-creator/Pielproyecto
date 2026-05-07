#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_PAGE_PRINCIPAL = 2,
    SCREEN_ID_DATOS_PAGE = 3,
    SCREEN_ID_CONFIGURACION_PAGE = 4,
    SCREEN_ID_MEDIR_PAGE = 5,
    _SCREEN_ID_LAST = 5
};

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *page_principal;
    lv_obj_t *datos_page;
    lv_obj_t *configuracion_page;
    lv_obj_t *medir_page;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *obj2;
    lv_obj_t *obj3;
    lv_obj_t *obj4;
    lv_obj_t *obj5;
    lv_obj_t *textarea1;
    lv_obj_t *textarea2;
    lv_obj_t *keybo1;
    lv_obj_t *obj6;
    lv_obj_t *obj7;
    lv_obj_t *obj8;
    lv_obj_t *button_medir;
    lv_obj_t *arc_humedad;
    lv_obj_t *label_valor;
    lv_obj_t *obj9;
} objects_t;

extern objects_t objects;

void create_screen_main();
void tick_screen_main();

void create_screen_page_principal();
void tick_screen_page_principal();

void create_screen_datos_page();
void tick_screen_datos_page();

void create_screen_configuracion_page();
void tick_screen_configuracion_page();

void create_screen_medir_page();
void tick_screen_medir_page();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/