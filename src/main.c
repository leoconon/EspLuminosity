#include <libs.h>

static const char *TAG = "ESPLOG";
QueueHandle_t bufferLuminosity; 
u8g2_t u8g2;

void delay(int millis);
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
        drawHistoric(&u8g2, lum);
        bool actualState = lum > 100;
        if (actualState != showSun || firstIteraction) {
            showSun = actualState;
            clearDraw(&u8g2);
            if (showSun) {
                drawSun(&u8g2);
            } else {
                drawMoon(&u8g2);
            }
        }
        if (showSun) {
            animateSun(&u8g2, sunBigger);
        }
        sunBigger = !sunBigger;
        printValue(&u8g2, lum);
        u8g2_SendBuffer(&u8g2);
        firstIteraction = false;
    }
}

void delay(int millis) {
    vTaskDelay(millis / portTICK_RATE_MS);
}