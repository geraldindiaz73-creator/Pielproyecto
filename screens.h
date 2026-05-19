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
    SCREEN_ID_RESULTADOS = 6,
    _SCREEN_ID_LAST = 6
};

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *page_principal;
    lv_obj_t *datos_page;
    lv_obj_t *configuracion_page;
    lv_obj_t *medir_page;
    lv_obj_t *resultados;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *obj2;
    lv_obj_t *obj3;
    lv_obj_t *obj4;
    lv_obj_t *obj5;
    lv_obj_t *obj6;
    lv_obj_t *obj7;
    lv_obj_t *obj8;
    lv_obj_t *obj9;
    lv_obj_t *obj10;
    lv_obj_t *obj11;
    lv_obj_t *obj12;
    lv_obj_t *obj13;
    lv_obj_t *obj14;
    lv_obj_t *masculino;
    lv_obj_t *obj15;
    lv_obj_t *textarea1;
    lv_obj_t *textarea2;
    lv_obj_t *keybo1;
    lv_obj_t *obj16;
    lv_obj_t *guardar;
    lv_obj_t *obj17;
    lv_obj_t *alarma;
    lv_obj_t *obj18;
    lv_obj_t *button_medir;
    lv_obj_t *arc_estres;
    lv_obj_t *label_porcentaje;
    lv_obj_t *obj19;
    lv_obj_t *label_bpm;
    lv_obj_t *label_gsr;
    lv_obj_t *label_estado;
    lv_obj_t *obj20;
    lv_obj_t *button_guardarresultados;
    lv_obj_t *obj21;
    lv_obj_t *obj22;
    lv_obj_t *obj23;
    lv_obj_t *obj24;
    lv_obj_t *obj25;
    lv_obj_t *nombre_resultado;
    lv_obj_t *edad_resultado;
    lv_obj_t *sexo_resultado;
    lv_obj_t *label_resultado_pct;
    lv_obj_t *label_resultado_estado;
    lv_obj_t *obj26;
    lv_obj_t *label_result_bpm;
    lv_obj_t *label_result_gsr;
    lv_obj_t *label_result_tiempo;
    lv_obj_t *button_nueva;
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

void create_screen_resultados();
void tick_screen_resultados();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/