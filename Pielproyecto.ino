#include <lvgl.h>
#include <SPI.h>
#include "Arduino_GFX_Library.h"
#include "HWCDC.h"
#include "TouchDrvGT911.hpp"
#include "ui.h"
#include <Wire.h>
#include <Adafruit_ADS1X15.h>

// =======================
// PINES
// =======================
#define I2C_SDA 15
#define I2C_SCL 7
#define PIN_RST_TOUCH 16

// =======================
// OBJETOS
// =======================
Adafruit_ADS1115 ads;
TouchDrvGT911 GT911;
HWCDC USBSerial;

int16_t x[5], y[5];

// =======================
// VARIABLES
// =======================
bool adsConectado = false;

int hidratacionActual = 0;

// =======================
// CALIBRACIÓN SENSOR
// AJUSTA ESTOS VALORES
// =======================
int ADC_SECO = 22000;
int ADC_HUMEDO = 11000;

// =======================
// PANTALLA
// =======================
Arduino_DataBus *bus = new Arduino_SWSPI(
  GFX_NOT_DEFINED,
  42,
  2,
  1,
  GFX_NOT_DEFINED
);

Arduino_ESP32RGBPanel *rgbpanel =
new Arduino_ESP32RGBPanel(
  40, 39, 38, 41, 46,
  3, 8, 18, 17, 14,
  13, 12, 11, 10, 9,
  5, 45, 48, 47, 21,
  1, 10, 8, 50,
  1, 10, 8, 20
);

Arduino_RGB_Display *gfx =
new Arduino_RGB_Display(
  480,
  480,
  rgbpanel,
  2,
  true,
  bus,
  GFX_NOT_DEFINED,
  st7701_type1_init_operations,
  sizeof(st7701_type1_init_operations)
);

// =======================
// DISPLAY
// =======================
void my_disp_flush(
  lv_disp_drv_t *disp,
  const lv_area_t *area,
  lv_color_t *color_p
) {

  uint32_t w =
  (area->x2 - area->x1 + 1);

  uint32_t h =
  (area->y2 - area->y1 + 1);

  gfx->draw16bitRGBBitmap(
    area->x1,
    area->y1,
    (uint16_t *)&color_p->full,
    w,
    h
  );

  lv_disp_flush_ready(disp);
}

// =======================
// TOUCH
// =======================
void my_touchpad_read(
  lv_indev_drv_t *indev_driver,
  lv_indev_data_t *data
) {

  uint8_t touched =
  GT911.getPoint(x, y, 1);

  if (touched > 0) {

    data->state =
    LV_INDEV_STATE_PR;

    data->point.x =
    gfx->width() - x[0];

    data->point.y =
    gfx->height() - y[0];

  } else {

    data->state =
    LV_INDEV_STATE_REL;
  }
}

// =======================
// ACTUALIZAR UI
// =======================
void actualizarUI(int humedad) {

  lv_obj_t *scr =
  lv_scr_act();

  lv_obj_t *child;

  uint32_t i = 0;

  lv_obj_t *arc_obj = NULL;
  lv_obj_t *label_obj = NULL;

  while (
    (child = lv_obj_get_child(scr, i))
    != NULL
  ) {

    // ===================
    // BUSCAR ARCO
    // ===================
    if (
      lv_obj_check_type(
        child,
        &lv_arc_class
      )
    ) {

      arc_obj = child;
    }

    // ===================
    // BUSCAR LABEL
    // ===================
    if (
      lv_obj_check_type(
        child,
        &lv_label_class
      )
    ) {

      const char *txt =
      lv_label_get_text(child);

      if (
        txt &&
        strcmp(txt, "Nombre") != 0 &&
        strcmp(txt, "Edad") != 0 &&
        strcmp(txt, "Sexo") != 0 &&
        strcmp(txt, "Menu Principal") != 0 &&
        strcmp(txt, "Datos Usuario") != 0 &&
        strcmp(txt, "Configuracion") != 0 &&
        strcmp(
          txt,
          "ANALIZADOR DE HIDRATACION DE LA PIEL"
        ) != 0
      ) {

        label_obj = child;
      }
    }

    i++;
  }

  // ===================
  // ACTUALIZAR ARCO
  // ===================
  if (arc_obj) {

    lv_arc_set_value(
      arc_obj,
      humedad
    );
  }

  // ===================
  // ACTUALIZAR LABEL
  // ===================
  if (label_obj) {

    char buffer[10];

    sprintf(
      buffer,
      "%d%%",
      humedad
    );

    lv_label_set_text(
      label_obj,
      buffer
    );
  }
}

// =======================
// SETUP
// =======================
void setup() {

  USBSerial.begin(115200);

  Wire.begin(
    I2C_SDA,
    I2C_SCL
  );

  // ===================
  // BACKLIGHT
  // ===================
  Wire.beginTransmission(0x20);

  Wire.write(0x01);

  Wire.write(0xFF);

  Wire.endTransmission();

  // ===================
  // RESET TOUCH
  // ===================
  pinMode(
    PIN_RST_TOUCH,
    OUTPUT
  );

  digitalWrite(
    PIN_RST_TOUCH,
    LOW
  );

  delay(50);

  digitalWrite(
    PIN_RST_TOUCH,
    HIGH
  );

  delay(200);

  // ===================
  // TOUCH
  // ===================
  if (
    !GT911.begin(
      Wire,
      0x5D,
      I2C_SDA,
      I2C_SCL
    )
  ) {

    GT911.begin(
      Wire,
      0x14,
      I2C_SDA,
      I2C_SCL
    );
  }

  // ===================
  // SENSOR
  // ===================
  if (ads.begin(0x48)) {

    adsConectado = true;

    ads.setGain(GAIN_ONE);

    USBSerial.println(
      "ADS1115 detectado"
    );

  } else {

    USBSerial.println(
      "ADS1115 NO detectado"
    );
  }

  // ===================
  // PANTALLA
  // ===================
  gfx->begin();

  lv_init();

  size_t buf_size =
  gfx->width() *
  gfx->height() / 4;

  lv_color_t *buf1 =
  (lv_color_t *)
  heap_caps_malloc(
    buf_size *
    sizeof(lv_color_t),
    MALLOC_CAP_DMA
  );

  static lv_disp_draw_buf_t draw_buf;

  lv_disp_draw_buf_init(
    &draw_buf,
    buf1,
    NULL,
    buf_size
  );

  static lv_disp_drv_t disp_drv;

  lv_disp_drv_init(
    &disp_drv
  );

  disp_drv.hor_res =
  gfx->width();

  disp_drv.ver_res =
  gfx->height();

  disp_drv.flush_cb =
  my_disp_flush;

  disp_drv.draw_buf =
  &draw_buf;

  lv_disp_drv_register(
    &disp_drv
  );

  // ===================
  // TOUCH DRIVER
  // ===================
  static lv_indev_drv_t indev_drv;

  lv_indev_drv_init(
    &indev_drv
  );

  indev_drv.type =
  LV_INDEV_TYPE_POINTER;

  indev_drv.read_cb =
  my_touchpad_read;

  lv_indev_drv_register(
    &indev_drv
  );

  // ===================
  // UI
  // ===================
  ui_init();
}

// =======================
// LOOP
// =======================
void loop() {

  lv_timer_handler();

  lv_tick_inc(5);

  // ===================
  // LEER SENSOR
  // ===================
  if (adsConectado) {

    static unsigned long lastS = 0;

    if (
      millis() - lastS > 1000
    ) {

      lastS = millis();

      int16_t adc0 =
      ads.readADC_SingleEnded(0);

      // =================
      // CONVERTIR %
      // =================
      hidratacionActual =
      map(
        adc0,
        ADC_SECO,
        ADC_HUMEDO,
        0,
        100
      );

      hidratacionActual =
      constrain(
        hidratacionActual,
        0,
        100
      );

      // =================
      // ACTUALIZAR UI
      // =================
      actualizarUI(
        hidratacionActual
      );

      // =================
      // SERIAL
      // =================
      USBSerial.printf(
        "ADC: %d | Humedad: %d%%\n",
        adc0,
        hidratacionActual
      );
    }
  }

  ui_tick();

  delay(5);
}