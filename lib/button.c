#include "./lib/button.h"

// Inicialização de um botão
void button_init(uint gpio){
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_IN);
    gpio_pull_up(gpio);
}

// Inicialização de todos os botões
void button_init_all(){
    button_init(BUTTON_A);
    button_init(BUTTON_B);
    button_init(JOYSTICK_BUTTON);
}

// Função para debounce dos botões
bool debounce(volatile uint16_t *last_time){
    uint16_t current_time = to_ms_since_boot(get_absolute_time());
    if (current_time - *last_time > 250){ 
        *last_time = current_time;
        return true;
    }
    return false;
}