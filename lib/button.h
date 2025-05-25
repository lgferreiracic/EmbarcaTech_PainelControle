#ifndef BUTTON_H
#define BUTTON_H

#include "stdio.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h"

#define BUTTON_A 5
#define BUTTON_B 6
#define JOYSTICK_BUTTON 22

void button_init(uint gpio); // Inicializa o botão
void button_init_all(); // Inicializa todos os botões
bool debounce(volatile uint16_t *last_time); // Função para debounce dos botões

#endif