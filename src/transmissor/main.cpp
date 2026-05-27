/**
 * @file main.cpp
 * 
 * @brief Arquivo principal do ESP32 transmissor de telemetria LoRa
 * Esse transmissor será responsável por processar, formatar e enviar os dados de telemetria dos sensores coletados
 * para o receptor via LoRa
 * 
 * @author Danilo Fróes
 * @date 2026
 */

#include <iostream>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "../include/sensores/ldr.hpp"

static const char* TAG = "Transmissor";

extern "C" {
   void app_main(void);
}

void setup();
void loop();

void app_main(void) {
   setup();

   while (true) {
      loop();
      vTaskDelay(pdMS_TO_TICKS(10));
   }
}

void setup() {
   // Inicializando a non-volatile storage (NVS)
   esp_err_t ret = nvs_flash_init();

   // Verificando se a inicialização da NVS falhou
   if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
         ESP_LOGW(TAG, "Problema na NVS, corrigindo...");

         ESP_ERROR_CHECK(nvs_flash_erase()); // Limpando a NVS
         ret = nvs_flash_init(); // Tentando inicializar novamente
   }
   
   ESP_ERROR_CHECK(ret); // Verificando se a inicialização da NVS foi bem-sucedida

   ESP_LOGI(TAG, "Iniciando Estação de Telemetria LoRa...");
}

void loop() {
   ESP_LOGI(TAG, "Lendo sensores...");
}