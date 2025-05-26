// Bibliotecas padrao do Pico SDK e do C
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Bibliotecas do projeto
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "lib/matrix.h"
#include "lib/buzzer.h"
#include "lib/button.h"
#include "lib/leds.h"
#include "lib/joystick.h"
#include "ws2812.pio.h"

// Bibliotecas de hardware do Pico SDK
#include "hardware/i2c.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/pio.h"

//Bibliotecas do FreeRTOS
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "semphr.h"

#define MAX_COUNT 25 // Definição do número máximo de passageiros

ssd1306_t ssd; // Variável para o display OLED SSD1306
SemaphoreHandle_t xDisplayMutex; // Mutex para o display OLED
SemaphoreHandle_t xButtonSem; // Semáforo para o botão do joystick
SemaphoreHandle_t xContadorSem; // Semáforo para o contador de passageiros
uint16_t counter = 0; // Contador de passageiros
uint16_t last_time_joystick = 0; // Variável para o debounce do joystick

// Função responsável por atualizar o display OLED com auxilio de um mutex para evitar conflitos de acesso
void display_update(const char *message) {
    if (xSemaphoreTake(xDisplayMutex, portMAX_DELAY) == pdTRUE) { // Tenta pegar o mutex do display
        ssd1306_fill(&ssd, false);
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "Lotacao: %d", counter);
        ssd1306_draw_string(&ssd, "CEPEDI   TIC37", 8, 10); // Desenha uma string
        ssd1306_draw_string(&ssd, buffer, 16, 30);
        ssd1306_draw_string(&ssd, message, 3, 48); // Desenha uma string      
        ssd1306_send_data(&ssd);
        xSemaphoreGive(xDisplayMutex); // Libera o mutex do display
    }
}

// Tarefa responsável por gerenciar a entrada de passageiros
void vTaskEntrada(void *params) {
    bool last_state = true;
    uint16_t last_time = 0;
    while (true) {
        bool current_state = gpio_get(BUTTON_A);
        if ((!current_state && last_state) && debounce(&last_time)) { // Verifica se o botão foi pressionado
            if (xSemaphoreTake(xContadorSem, 0) == pdTRUE && counter < MAX_COUNT) { // Tenta pegar o semáforo do contador
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

// Tarefa responsável por gerenciar a saída de passageiros
void vTaskSaida(void *params) {
    bool last_state = true;
    uint16_t last_time = 0;
    while (true) {
        bool current_state = gpio_get(BUTTON_B);
        if ((!current_state && last_state) && debounce(&last_time)) { // Verifica se o botão foi pressionado
            if (counter > 0) {
                counter--;
                xSemaphoreGive(xContadorSem); // Libera o semáforo do contador
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

// Tarefa responsável por gerenciar o reset do contador
void vTaskReset(void *params) {
    while (true){
        if(xSemaphoreTake(xButtonSem, portMAX_DELAY) == pdTRUE){ // Espera pelo semáforo do botão
            while(counter > 0){ // Enquanto o contador for maior que zero
                xSemaphoreGive(xContadorSem); // Libera o semáforo do contador
                counter--; // Decrementa o contador
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

// Tarefa responsável por gerenciar o LED RGB
void vLEDsTask(void *params) {
    while (true) {
        led_rgb_off();
        if(counter == 0) // Se o contador for zero, apaga todos os LEDs
            led_rgb_blue_color(); 
        else if(counter < MAX_COUNT - 1) // Se o contador for menor que o máximo - 1, acende o LED verde
            led_rgb_green_color(); 
        else if(counter == MAX_COUNT - 1) // Se o contador for igual ao máximo - 1, acende o LED amarelo
            led_rgb_yellow_color(); 
        else // Se o contador for igual ao máximo, acende o LED vermelho
            led_rgb_red_color();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Tarefa responsável por atualizar a matriz de LEDs RGB
void vMatrixTask(void *params) {
    while (true) {
        matrix_update(counter);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Função para o tratamento de interrupção do botão do joystick
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
    hardware_init_all(); // Inicializa todo o hardware

    xDisplayMutex = xSemaphoreCreateMutex(); // Cria um mutex para o display OLED
    xButtonSem = xSemaphoreCreateBinary(); // Cria um semáforo binário para o botão do joystick
    xContadorSem = xSemaphoreCreateCounting(MAX_COUNT, MAX_COUNT); // Cria um semáforo de contagem para o contador de passageiros

    // Tarefas do FreeRTOS
    xTaskCreate(vTaskEntrada, "vTaskEntrada", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(vTaskSaida, "vTaskSaida", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(vTaskReset, "vTaskReset", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(vLEDsTask, "vLEDsTask", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(vMatrixTask, "vMatrixTask", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    vTaskStartScheduler(); // Inicia o escalonador do FreeRTOS
    panic_unsupported(); 
}
