/// @brief Arquivo de definição dos pinos utilizados no projeto

#pragma once

#include <cstdint>

#include "hal/adc_types.h"
#include "soc/gpio_num.h"

namespace ESP {
    // Pinos para o módulo LoRa (SPI)
    constexpr gpio_num_t PINO_MOSI = GPIO_NUM_13; // Master Out Slave In
    constexpr gpio_num_t PINO_MISO = GPIO_NUM_12; // Master In Slave Out
    constexpr gpio_num_t PINO_SCK  = GPIO_NUM_14; // Serial Clock
    constexpr gpio_num_t PINO_SS   = GPIO_NUM_15;  // Slave Select
    constexpr gpio_num_t PINO_RST  = GPIO_NUM_27; // Reser

    #ifdef TRANSMISSOR

        constexpr uint8_t PINO_LED_TRANSMISSOR = 2; // LED para indicar status do transmissor

        constexpr gpio_num_t PINO_DHT = GPIO_NUM_33; // Pino para sensor de temperatura e umidade DHT

        constexpr adc_channel_t CANAL_LDR = ADC_CHANNEL_0; // Canal 0, localizado no pino 36

    #elifdef RECEPTOR

        constexpr uint8_t PINO_LED_RECEPTOR = 2; // LED para indicar status do receptor

    #endif
}

namespace SX {
    // Registradores do módulo SX1276
    constexpr uint8_t REG_FIFO              = 0x00; // Endereço do FIFO para leitura e escrita de dados
    constexpr uint8_t REG_OP_MODE           = 0x01; // Modo de operação do módulo (standby, transmit, receive, etc.)
    constexpr uint8_t REG_FRF_MSB           = 0x06; // Frequência de operação - parte mais significativa
    constexpr uint8_t REG_FRF_MID           = 0x07; // Frequência de operação - parte intermediária
    constexpr uint8_t REG_FRF_LSB           = 0x08; // Frequência de operação - parte menos significativa
    constexpr uint8_t REG_PA_CONFIG         = 0x09; // Configuração do amplificador de potência para transmissão
    constexpr uint8_t REG_FIFO_ADDR_PTR     = 0x0D; // Ponteiro para o endereço do FIFO, usado para leitura e escrita de dados
    constexpr uint8_t REG_FIFO_TX_BASE_ADDR = 0x0E; // Endereço base para escrita de dados a serem transmitidos
    constexpr uint8_t REG_FIFO_RX_BASE_ADDR = 0x0F; // Endereço base para leitura de dados recebidos
    constexpr uint8_t REG_IRQ_FLAGS         = 0x12; // Flags de interrupção para eventos
    constexpr uint8_t REG_PAYLOAD_LENGTH    = 0x22; // Comprimento do payload a ser transmitido ou recebido

    // Modos de operação
    constexpr uint8_t MODE_SLEEP            = 0x00; // Modo de baixo consumo, módulo inativo
    constexpr uint8_t MODE_STDBY            = 0x01; // Modo de espera, pronto para transmitir ou receber
    constexpr uint8_t MODE_TX               = 0x03; // Modo de transmissão
    constexpr uint8_t MODE_RX_CONT          = 0x05; // Modo de recepção contínua
    constexpr uint8_t MODE_LONG_RANGE_MODE  = 0x80; // Habilita o modo de longo alcance (LoRa)

    // Flags de interrupção
    constexpr uint8_t IRQ_TX_DONE_MASK      = 0x08; // Flag de transmissão concluída
    constexpr uint8_t IRQ_RX_DONE_MASK      = 0x40; // Flag de recepção concluída
}

