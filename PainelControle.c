#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/ssd1306.h"
#include "lib/font.h"
#include "lib/matrix.h"
#include "lib/buzzer.h"
#include "lib/button.h"
#include "lib/leds.h"
#include "lib/joystick.h"

#include "hardware/i2c.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "semphr.h"

#include <cyw43_ll.h>
#include "pico/cyw43_arch.h"       
#include "lwip/pbuf.h"           
#include "lwip/tcp.h"            
#include "lwip/netif.h" 

#define WIFI_SSID "A35 de Lucas"
#define WIFI_PASSWORD "lucaslucas"

char html[2048]; 
ssd1306_t ssd; 
SemaphoreHandle_t xDisplayMutex;
SemaphoreHandle_t xButtonSem;
SemaphoreHandle_t xContadorSem;
uint16_t eventosProcessados = 0;

void vTaskEntrada(void *params) {

}

void vTaskSaida(void *params) {

}

void vTaskReset(void *params) {

}

void vDisplayTask(void *params){
    i2c_init(I2C_PORT, 400000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ADRESS, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    while(true){
        vTaskDelay(pdMS_TO_TICKS(50)); 
    }   
}

void gpio_callback(uint gpio, uint32_t events) {

}

void gpio_irq_handler(uint gpio, uint32_t events) {

}


int main() {
    stdio_init_all();

    xDisplayMutex = xSemaphoreCreateMutex();
    xButtonSem = xSemaphoreCreateBinary();
    xContadorSem = xSemaphoreCreateCounting(44, 0);

    xTaskCreate(vTaskEntrada, "vTaskEntrada", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);
    xTaskCreate(vTaskSaida, "vTaskSaida", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);
    xTaskCreate(vTaskReset, "vTaskReset", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);
    xTaskCreate(vDisplayTask, "vDisplayTask", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);

    vTaskStartScheduler();
    panic_unsupported();
}
