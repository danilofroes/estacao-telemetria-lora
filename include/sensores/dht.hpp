/// @brief Classe para o sensor de umidade e temperatura DHT11

#pragma once

#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "eletronica/pinagem.hpp"

class DHT {
    private:
        static constexpr char* TAG = "DHT11";
        
        gpio_num_t pino = static_cast<gpio_num_t>(Pinagem::PINO_DHT);

        void initGPIO() {
            gpio_config_t configPino = {
                .pin_bit_mask = 1ULL << pino,
                .mode         = GPIO_MODE_INPUT_OUTPUT,
                .pull_up_en   = GPIO_PULLUP_DISABLE,
                .pull_down_en = GPIO_PULLDOWN_DISABLE,
                .intr_type    = GPIO_INTR_DISABLE
            };

            ESP_ERROR_CHECK(gpio_config(&configPino));
        }

        void lerDados() {
            gpio_set_direction(pino, GPIO_MODE_OUTPUT); // Definindo o GPIO como saída
            gpio_set_level(pino, 0); // Pino como LOW
            vTaskDelay(pdMS_TO_TICKS(18)); // Aguarda 18ms

            gpio_set_direction(pino, GPIO_MODE_INPUT); // Define o pino como entrada
            int nivelAtual = gpio_get_level(pino);
        }
    
    public:
        DHT() {}

        ~DHT() {}
};