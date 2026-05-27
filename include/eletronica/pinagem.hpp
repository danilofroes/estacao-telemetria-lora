/// @brief Arquivo de definição dos pinos utilizados no projeto

#pragma once

#include <cstdint>

#include "hal/adc_types.h"

namespace Pinagem {

    // Pinos para o módulo LoRa (SPI)
    constexpr uint8_t PINO_MOSI = 23; // Master Out Slave In
    constexpr uint8_t PINO_MISO = 19; // Master In Slave Out
    constexpr uint8_t PINO_SCK  = 18;  // Serial Clock
    constexpr uint8_t PINO_SS   = 5;    // Slave Select

    #ifdef TRANSMISSOR

    constexpr uint8_t PINO_LED_TRANSMISSOR = 2; // LED para indicar status do transmissor

    constexpr uint8_t PINO_DHT = 33; // Pino para sensor de temperatura e umidade DHT

    constexpr adc_channel_t CANAL_LDR = ADC_CHANNEL_0; // Canal 0, localizado no pino 36

    #elifdef RECEPTOR

    constexpr uint8_t PINO_LED_RECEPTOR = 2; // LED para indicar status do receptor

    #endif
}