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

#define MAX_COUNT 25

char html[2048]; 
ssd1306_t ssd; 
SemaphoreHandle_t xDisplayMutex;
SemaphoreHandle_t xButtonSem;
SemaphoreHandle_t xContadorSem;
uint16_t counter = 0;
uint16_t last_time_joystick = 0;

void display_update(const char *message) {
    if (xSemaphoreTake(xDisplayMutex, portMAX_DELAY) == pdTRUE) {
        ssd1306_fill(&ssd, false);
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "Lotacao: %d", counter);
        ssd1306_draw_string(&ssd, "CEPEDI   TIC37", 8, 10); // Desenha uma string
        ssd1306_draw_string(&ssd, buffer, 16, 30);
        ssd1306_draw_string(&ssd, message, 3, 48); // Desenha uma string      
        ssd1306_send_data(&ssd);
        xSemaphoreGive(xDisplayMutex);
    }
}

void vTaskEntrada(void *params) {
    bool last_state = true;
    uint16_t last_time = 0;
    while (true) {
        bool current_state = gpio_get(BUTTON_A);
        if ((!current_state && last_state) && debounce(&last_time)) {
            if (xSemaphoreTake(xContadorSem, 0) == pdTRUE && counter < MAX_COUNT) {
                counter++;
                printf("Counter: %d\n", counter);
                display_update("Passageiros ++");
            } else {
                printf("Counter is full\n");
                display_update(" Onibus cheio");
                play_buzzers();
                vTaskDelay(pdMS_TO_TICKS(200));
                stop_buzzers();
            }
            
        }
        last_state = current_state;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void vTaskSaida(void *params) {
    bool last_state = true;
    uint16_t last_time = 0;
    while (true) {
        bool current_state = gpio_get(BUTTON_B);
        if ((!current_state && last_state) && debounce(&last_time)) {
            if (counter > 0) {
                counter--;
                xSemaphoreGive(xContadorSem);
                printf("Counter: %d\n", counter);
                display_update("Passageiros --");
            } else {
                printf("Counter is empty\n");
                display_update(" Onibus vazio");
            }
            
        }
        last_state = current_state;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void vTaskReset(void *params) {
    while (true){
        if(xSemaphoreTake(xButtonSem, portMAX_DELAY) == pdTRUE){
            while(counter > 0){
                xSemaphoreGive(xContadorSem);
                counter--;
            }
            printf("Counter reset\n");
            display_update(" Ultima Parada");
            play_buzzers();
            vTaskDelay(pdMS_TO_TICKS(100));
            stop_buzzers();
            vTaskDelay(pdMS_TO_TICKS(100));
            play_buzzers();
            vTaskDelay(pdMS_TO_TICKS(100));
            stop_buzzers();
        }
    }
}

void vLEDsTask(void *params) {
    while (true) {
        led_rgb_off();
        if(counter == 0)
            led_rgb_blue_color(); 
        else if(counter < MAX_COUNT - 1)
            led_rgb_green_color(); 
        else if(counter == MAX_COUNT - 1)
            led_rgb_yellow_color(); 
        else
            led_rgb_red_color();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void vMatrixTask(void *params) {
    while (true) {
        matrix_update(counter);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void gpio_irq_handler(uint gpio, uint32_t events) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;  //Nenhum contexto de tarefa foi despertado
    if(gpio == JOYSTICK_BUTTON && debounce(&last_time_joystick)){
        xSemaphoreGiveFromISR(xButtonSem, &xHigherPriorityTaskWoken);    //Libera o semáforo
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken); // Troca o contexto da tarefa
}

// Função para inicializar todo o hardware
void hardware_init_all(){
    stdio_init_all(); // Inicializa a comunicação padrão
    buzzer_init_all(); // Inicializa todos os buzzers
    matrix_init(); // Inicializa a matriz de LEDs RGB
    joystick_init(); // Inicializa o joystick
    led_init_all(); // Inicializa todos os LEDs
    button_init_all(); // Inicializa todos os botões
    display_init(&ssd); // Inicializa o display OLED SSD1306
    gpio_set_irq_enabled_with_callback(JOYSTICK_BUTTON, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
}

int main() {
    stdio_init_all();
    hardware_init_all(); 

    xDisplayMutex = xSemaphoreCreateMutex();
    xButtonSem = xSemaphoreCreateBinary();
    xContadorSem = xSemaphoreCreateCounting(MAX_COUNT, MAX_COUNT);

    xTaskCreate(vTaskEntrada, "vTaskEntrada", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(vTaskSaida, "vTaskSaida", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(vTaskReset, "vTaskReset", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(vLEDsTask, "vLEDsTask", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(vMatrixTask, "vMatrixTask", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    vTaskStartScheduler();
    panic_unsupported();
}
