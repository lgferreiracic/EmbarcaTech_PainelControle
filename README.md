# BusCounter+: Controle de Lotação Inteligente com Feedback Visual e Auditivo
<p align="center">
  <img src="Group 658.png" alt="EmbarcaTech" width="300">
</p>

![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-%23008FBA.svg?style=for-the-badge&logo=cmake&logoColor=white)
![Raspberry Pi](https://img.shields.io/badge/-Raspberry_Pi-C51A4A?style=for-the-badge&logo=Raspberry-Pi)
![GitHub](https://img.shields.io/badge/github-%23121011.svg?style=for-the-badge&logo=github&logoColor=white)
![Windows 11](https://img.shields.io/badge/Windows%2011-%230079d5.svg?style=for-the-badge&logo=Windows%2011&logoColor=white)

## Descrição do Projeto

O **BusCounter+** é um sistema embarcado para controle de lotação em tempo real de veículos de transporte coletivo, como ônibus. Utilizando o microcontrolador **Raspberry Pi Pico W**, o sistema monitora entradas e saídas de passageiros por meio de botões físicos, fornecendo **feedback visual e sonoro** com diversos periféricos.

Ao atingir a lotação máxima, o sistema bloqueia novas entradas utilizando um **semáforo contador do FreeRTOS**, garantindo controle eficaz da capacidade. Um modo especial de **"Última Parada"** zera a contagem rapidamente via interrupção, ideal para reiniciar o sistema no fim da linha.

## Componentes Utilizados

- **Botão A**: Simula entrada de passageiro.
- **Botão B**: Simula saída de passageiro.
- **Botão do Joystick (Click)**: Ativa o modo "Última Parada".
- **Microcontrolador Raspberry Pi Pico W (RP2040)**: Executa FreeRTOS e gerencia periféricos.
- **LED RGB**: Indicação visual do nível de ocupação.
- **Matriz de LEDs WS2812B**: Representação gráfica do número de passageiros.
- **Display OLED SSD1306 (I2C)**: Mostra mensagens e status do sistema.
- **Buzzer (PWM)**: Alertas sonoros para eventos como “ônibus cheio” e “última parada”.

## Ambiente de Desenvolvimento

- **VS Code** com extensão da Raspberry Pi
- **C (Pico SDK)**
- **FreeRTOS**

## Guia de Instalação

1. Clone este repositório.
2. Abra no VS Code com o ambiente do Pico SDK corretamente configurado.
3. Compile o projeto (CMake + SDK).
4. Conecte o RP2040 via **BOOTSEL** e arraste o arquivo `.uf2`.
5. O sistema inicia automaticamente após a gravação.

## Modos de Operação

### Incremento e Decremento de Passageiros
- O botão A incrementa o número de passageiros.
- O botão B decrementa o número de passageiros.
- A matriz de LEDs acende proporcionalmente à ocupação.
- O display mostra mensagens como "Passageiros ++" ou "Passageiros --".
- O LED RGB indica:
  - 🔵 Azul: Vazio
  - 🟢 Verde: Há vagas
  - 🟡 Amarelo: Quase cheio
  - 🔴 Vermelho: Lotado
- Quando o número máximo de passageiros é atingido:
  - O sistema bloqueia novas entradas via `xContadorSem`.
  - O display exibe "Ônibus cheio".
  - O buzzer emite um alerta sonoro.

### Reset
- Ativado com o clique do joystick.
- Zera o contador e reinicia o sistema.
- O display exibe "Última Parada".
- O buzzer emite dois sinais curtos.
- Liberadas todas as permissões de entrada (semáforo resetado).

## Destaques Técnicos

- **Debounce por software** nos botões físicos para leituras confiáveis.
- **Interrupção externa** no botão do joystick, com entrega de semáforo pela ISR.
- **Mutex (`xDisplayMutex`)** para controle seguro de acesso ao OLED.
- **Semáforo contador (`xContadorSem`)** para controle de acesso baseado em limite de passageiros.
- **Modularidade no código** com bibliotecas separadas para cada periférico.
- **Atualização contínua da matriz WS2812B via PIO**, sem bloqueios.

## Testes Realizados

1. Validação de entradas e saídas com controle de capacidade.
2. Alterações visuais em tempo real no display OLED.
3. Teste dos níveis de LED RGB com base no número de passageiros.
4. Alertas sonoros corretos via buzzer.
5. Comportamento de bloqueio/desbloqueio com semáforos.
6. Reset de sistema com o modo "Última Parada" via interrupção.

## Vídeo da Solução

[Assista no YouTube](https://www.youtube.com/watch?v=lw1H96zziUk)
