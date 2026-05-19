#include <lvgl.h>
#include <SPI.h>

#include "Arduino_GFX_Library.h"
#include "HWCDC.h"
#include "TouchDrvGT911.hpp"

#include "ui.h"
#include "screens.h"
#include "vars.h"

#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <MAX30105.h>

extern objects_t objects;

#define I2C_SDA 15
#define I2C_SCL 7
#define PIN_RST_TOUCH 16

#define GSR_REPOSO   960  // subir un poco para absorber la deriva natural
#define GSR_MAXIMO   4000
#define UMBRAL_CONTACTO 300  // vuelve a 300, ya no necesitas 1800

Adafruit_ADS1115 ads;
MAX30105 max30105;
TouchDrvGT911 GT911;
HWCDC USBSerial;

int16_t x[5], y[5];

bool adsOK = false;
bool maxOK = false;
bool midiendo = false;

long irValue = 0;

int contacto = 0;
int contactoBase = 0;
int delta = 0;
int deltaFiltrado = 0;

bool dedo = false;
bool contactoOK = false;

int resultadoFinal = 0;
int resultadoFiltrado = 0;

int bpm = 0;

unsigned long tiempoInicio = 0;
unsigned long ultimoSensor = 0;

Arduino_DataBus *bus = new Arduino_SWSPI(
  GFX_NOT_DEFINED, 42, 2, 1, GFX_NOT_DEFINED
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
  480, 480, rgbpanel, 2, true,
  bus, GFX_NOT_DEFINED,
  st7701_type1_init_operations,
  sizeof(st7701_type1_init_operations)
);

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
  lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  uint8_t touched = GT911.getPoint(x, y, 1);
  if (touched > 0) {
    data->state = LV_INDEV_STATE_PR;
    data->point.x = gfx->width() - x[0];
    data->point.y = gfx->height() - y[0];
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

int leerMonedas() {
  long suma = 0;
  for (int i = 0; i < 10; i++) {
    suma += ads.readADC_SingleEnded(0);
    delay(2);
  }
  return suma / 10;
}

void actualizarUI(int porcentaje, int bpmValor, int gsr, const char *estado) {
  lv_arc_set_value(objects.arc_estres, porcentaje);

  char txtPct[20];
  sprintf(txtPct, "%d%%", porcentaje);
  lv_label_set_text(objects.label_porcentaje, txtPct);

  char txtBPM[30];
  sprintf(txtBPM, "%d BPM", bpmValor);
  lv_label_set_text(objects.label_bpm, txtBPM);

  char txtGSR[30];
  sprintf(txtGSR, "%d", gsr);
  lv_label_set_text(objects.label_gsr, txtGSR);

  lv_label_set_text(objects.label_estado, estado);

  lv_obj_invalidate(objects.arc_estres);
  lv_obj_invalidate(objects.label_porcentaje);
  lv_obj_invalidate(objects.label_bpm);
  lv_obj_invalidate(objects.label_gsr);
  lv_obj_invalidate(objects.label_estado);
}

void setup() {
  Serial.begin(115200);
  USBSerial.begin(115200);

  Wire.begin(I2C_SDA, I2C_SCL);
  delay(100);

  Wire.beginTransmission(0x20);
  Wire.write(0x01);
  Wire.write(0xFF);
  Wire.endTransmission();

  pinMode(PIN_RST_TOUCH, OUTPUT);
  digitalWrite(PIN_RST_TOUCH, LOW);
  delay(50);
  digitalWrite(PIN_RST_TOUCH, HIGH);
  delay(200);

  if (!GT911.begin(Wire, 0x5D, I2C_SDA, I2C_SCL)) {
    GT911.begin(Wire, 0x14, I2C_SDA, I2C_SCL);
  }

  if (ads.begin(0x48)) {
    adsOK = true;
    ads.setGain(GAIN_ONE);
    Serial.println("ADS1115 OK");
  }

  if (max30105.begin(Wire, I2C_SPEED_STANDARD)) {
    maxOK = true;
    max30105.setup();
    max30105.setPulseAmplitudeRed(0x0A);
    max30105.setPulseAmplitudeGreen(0);
    Serial.println("MAX30105 OK");
  }

  long suma = 0;
  for (int i = 0; i < 50; i++) {
    suma += leerMonedas();
    delay(10);
  }
  contactoBase = suma / 50;
  Serial.print("BASE: ");
  Serial.println(contactoBase);

  gfx->begin();
  lv_init();

  size_t buf_size = gfx->width() * gfx->height() / 4;
  lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(buf_size * sizeof(lv_color_t), MALLOC_CAP_DMA);

  static lv_disp_draw_buf_t draw_buf;
  lv_disp_draw_buf_init(&draw_buf, buf1, NULL, buf_size);

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = gfx->width();
  disp_drv.ver_res = gfx->height();
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  ui_init();
  actualizarUI(0, 0, 0, "ESPERA");
}

void loop() {
  lv_timer_handler();
  lv_tick_inc(5);

  if (lv_obj_has_state(objects.button_medir, LV_STATE_PRESSED)) {
    if (!midiendo) {
      midiendo = true;
      tiempoInicio = millis();
      deltaFiltrado = 0;
      resultadoFiltrado = 0;
      bpm = 0;
      Serial.println("MEDICION INICIADA");
    }
  }

  if (midiendo && millis() - ultimoSensor > 200) {
    ultimoSensor = millis();

    irValue = max30105.getIR();
    dedo = irValue > 50000;

    if (dedo) {
      bpm = map(irValue, 50000, 120000, 60, 110);
      bpm = constrain(bpm, 60, 110);
    } else {
      bpm = 0;
    }

    contacto = leerMonedas();
    delta = abs(contacto - contactoBase);
    deltaFiltrado = (deltaFiltrado * 9 + delta) / 10;
    contactoOK = deltaFiltrado > UMBRAL_CONTACTO;

    if (dedo && contactoOK) {
      int gsrRelativo = deltaFiltrado - GSR_REPOSO;
      if (gsrRelativo < 0) gsrRelativo = 0;
      resultadoFinal = map(gsrRelativo, 0, GSR_MAXIMO - GSR_REPOSO, 0, 100);
      resultadoFinal = constrain(resultadoFinal, 0, 100);
    } else {
      resultadoFinal = 0;
    }

    resultadoFiltrado = (resultadoFiltrado * 8 + resultadoFinal * 2) / 10;

    int gsrMostrar = (dedo && contactoOK) ? deltaFiltrado : 0;

    const char *estado;
    if (resultadoFiltrado < 30) {
      estado = "NORMAL";
    } else if (resultadoFiltrado < 70) {
      estado = "MEDIO";
    } else {
      estado = "ALTO";
    }

    actualizarUI(resultadoFiltrado, bpm, gsrMostrar, estado);

    Serial.print("IR: ");     Serial.print(irValue);
    Serial.print(" | BPM: "); Serial.print(bpm);
    Serial.print(" | GSR: "); Serial.print(deltaFiltrado);
    Serial.print(" | Rel: "); Serial.print(deltaFiltrado - GSR_REPOSO);
    Serial.print(" | %: ");   Serial.println(resultadoFiltrado);
  }

  if (midiendo && millis() - tiempoInicio > 20000) {
    midiendo = false;
    Serial.println("MEDICION FINALIZADA");
  }

  ui_tick();
  delay(5);
}