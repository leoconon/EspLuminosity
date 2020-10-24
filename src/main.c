#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <driver/gpio.h>
#include <stdio.h>
#include <driver/spi_master.h>
#include <driver/adc.h>
#include <u8g2_esp32_hal.h> 

#define loop while(true)
#define PIN_LUM ADC_WIDTH_BIT_12
#define PIN_SDA 5
#define PIN_SCL 4
#define MAX_WIDTH 128
#define MAX_HEIGHT 64
#define LUMINOSITY_MAX_VALUE 1000

static const char *TAG = "ESPLOG";
QueueHandle_t bufferLuminosity; 
int historic[MAX_WIDTH];
u8g2_t u8g2;

void delay(int millis);
void drawHistoric(int newValue);
void drawMoon();
void drawSun();
void animateSun(bool small);
void clearDraw();
void printValue(int value);
void taskReadLum(void *pvParameters);
void taskDisplay(void *pvParameters);

void app_main() {
    bufferLuminosity = xQueueCreate(1, sizeof(int));
    xTaskCreate(taskReadLum, "task_lum", 2048, NULL, 1, NULL);
    xTaskCreate(taskDisplay, "task_display", 2048, NULL, 1, NULL);
}

void taskReadLum(void *pvParameters) {
    adc1_config_width(PIN_LUM);
    adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_0);
    int val;
    loop {
        val = adc1_get_raw(PIN_LUM);
        xQueueSend(bufferLuminosity, &val, pdMS_TO_TICKS(0));
        ESP_LOGI(TAG, "%d", val);
        delay(500);
    }
}

void taskDisplay(void *pvParameters) {
    u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
	u8g2_esp32_hal.sda = PIN_SDA;
	u8g2_esp32_hal.scl = PIN_SCL;
	u8g2_esp32_hal_init(u8g2_esp32_hal);

    u8g2_Setup_ssd1306_i2c_128x64_noname_f(
		&u8g2,
		U8G2_R0,
		u8g2_esp32_i2c_byte_cb,
		u8g2_esp32_gpio_and_delay_cb);

    u8x8_SetI2CAddress(&u8g2.u8x8, 0x78); 
	u8g2_InitDisplay(&u8g2);
	u8g2_SetPowerSave(&u8g2, 0);

    u8g2_ClearBuffer(&u8g2);

    int lum;
    bool sunBigger = true;
    bool showSun = false;
    bool firstIteraction = true;

    loop {
        xQueueReceive(bufferLuminosity, &lum, pdMS_TO_TICKS(2000));
        drawHistoric(lum);
        bool actualState = lum > 100;
        if (actualState != showSun || firstIteraction) {
            showSun = actualState;
            clearDraw();
            if (showSun) {
                drawSun();
            } else {
                drawMoon();
            }
        }
        if (showSun) {
            animateSun(sunBigger);
        }
        sunBigger = !sunBigger;
        printValue(lum);
        u8g2_SendBuffer(&u8g2);
        firstIteraction = false;
    }
}

void delay(int millis) {
    vTaskDelay(millis / portTICK_RATE_MS);
}

void drawHistoric(int newValue) {
    if (newValue > LUMINOSITY_MAX_VALUE) {
        newValue = LUMINOSITY_MAX_VALUE;
    }
    int percent = (newValue / 1.0) / (LUMINOSITY_MAX_VALUE / 1.0) * 100.0;
    for (int i = 0; i < MAX_WIDTH - 1; i++) {
        historic[i] = historic[i + 1];
    }
    historic[MAX_WIDTH - 1] = percent;
    for (int i = 0; i < MAX_WIDTH; i++) {
        int size = historic[i] / 5.0;
        u8g2_SetDrawColor(&u8g2, 0);
        u8g2_DrawVLine(&u8g2, i, MAX_HEIGHT - 20, 20);
        u8g2_SetDrawColor(&u8g2, 1);
        u8g2_DrawVLine(&u8g2, i, MAX_HEIGHT - size, size);
    }
}

void printValue(int value) {
    char print[5];
    sprintf(print, "%d", value);
    u8g2_SetFont(&u8g2, u8g2_font_6x10_mf);
    u8g2_DrawUTF8(&u8g2, 64, 26, print);
}

void drawMoon() {
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_DrawDisc(&u8g2, 23, 23, 18, U8G2_DRAW_ALL);
    u8g2_SetDrawColor(&u8g2, 0);
    u8g2_DrawDisc(&u8g2, 30, 20, 18, U8G2_DRAW_ALL);
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_SetFont(&u8g2, u8g2_font_unifont_t_symbols);
    u8g2_DrawGlyph(&u8g2, 78, 31, 0x2605);
    u8g2_DrawGlyph(&u8g2, 108, 38, 0x2605);
    u8g2_DrawGlyph(&u8g2, 39, 10, 0x2605);
    u8g2_DrawGlyph(&u8g2, 28, 26, 0x2605);
    u8g2_DrawGlyph(&u8g2, 60, 15, 0x2605);
    u8g2_DrawGlyph(&u8g2, 71, 41, 0x2605);
    u8g2_DrawGlyph(&u8g2, 95, 17, 0x2605);
    u8g2_SendBuffer(&u8g2);
}

void drawSun() {
    //Circulo Central
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_DrawDisc(&u8g2, 23, 23, 8, U8G2_DRAW_ALL);
}

void animateSun(bool small) {
    if (small) {
        u8g2_SetDrawColor(&u8g2, 0);
        //Vertical Superior
        u8g2_DrawLine(&u8g2, 23, 5, 23, 11);
        //Vertical Inferior
        u8g2_DrawLine(&u8g2, 23, 35, 23, 41);
        //Horizontal Esquerda
        u8g2_DrawLine(&u8g2, 11, 23, 5, 23);
        //Horizontal Direita
        u8g2_DrawLine(&u8g2, 35, 23, 41, 23);
        //Diagonal Norte Direita
        u8g2_DrawLine(&u8g2, 35, 11, 31, 15);
        //Diagonal Norte Esquerda
        u8g2_DrawLine(&u8g2, 11, 11, 15, 15);
        //Diagonal Sul Direita
        u8g2_DrawLine(&u8g2, 35, 35, 31, 31);
        //Diagonal Sul Esquerda
        u8g2_DrawLine(&u8g2, 11, 35, 15, 31);

        u8g2_SetDrawColor(&u8g2, 1);
        //Vertical Superior
        u8g2_DrawLine(&u8g2, 23, 7, 23, 11);
        //Vertical Inferior
        u8g2_DrawLine(&u8g2, 23, 35, 23, 39);
        //Horizontal Esquerda
        u8g2_DrawLine(&u8g2, 11, 23, 7, 23);
        //Horizontal Direita
        u8g2_DrawLine(&u8g2, 35, 23, 39, 23);
        //Diagonal Norte Direita
        u8g2_DrawLine(&u8g2, 33, 13, 31, 15);
        //Diagonal Norte Esquerda
        u8g2_DrawLine(&u8g2, 13, 13, 15, 15);
        //Diagonal Sul Direita
        u8g2_DrawLine(&u8g2, 33, 33, 31, 31);
        //Diagonal Sul Esquerda
        u8g2_DrawLine(&u8g2, 13, 33, 15, 31);
    } else {
        u8g2_SetDrawColor(&u8g2, 1);
        //Vertical Superior
        u8g2_DrawLine(&u8g2, 23, 5, 23, 11);
        //Vertical Inferior
        u8g2_DrawLine(&u8g2, 23, 35, 23, 41);
        //Horizontal Esquerda
        u8g2_DrawLine(&u8g2, 11, 23, 5, 23);
        //Horizontal Direita
        u8g2_DrawLine(&u8g2, 35, 23, 41, 23);
        //Diagonal Norte Direita
        u8g2_DrawLine(&u8g2, 35, 11, 31, 15);
        //Diagonal Norte Esquerda
        u8g2_DrawLine(&u8g2, 11, 11, 15, 15);
        //Diagonal Sul Direita
        u8g2_DrawLine(&u8g2, 35, 35, 31, 31);
        //Diagonal Sul Esquerda
        u8g2_DrawLine(&u8g2, 11, 35, 15, 31);
    }
}

void clearDraw() {
    u8g2_SetDrawColor(&u8g2, 0);
    u8g2_DrawBox(&u8g2, 0, 0, MAX_WIDTH, 44);
}
