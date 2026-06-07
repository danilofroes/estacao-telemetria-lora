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
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "../include/configuracoes/configTransmissor.hpp"

extern "C" void app_main(void) {
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

   filaDados = xQueueCreate(5, sizeof(DadosTelemetria)); // Criando a fila de comunicação entre tarefas para armazenar até 5 pacotes pendentes

   if (filaDados == NULL) {
      ESP_LOGE(TAG, "Falha ao criar a fila de dados");
      return; // Encerrando a aplicação para evitar a criação das tasks sem a fila
   }

   // Criando a thread para leitura de sensores no Core 1 (APP_CPU)
   xTaskCreatePinnedToCore(
      taskLeituraSensores, // Função que irá rodar na task
      "LeituraSensores",   // Nome para a task
      4096,                // Alocando 4kb na Stack para task
      NULL,                // Sem parâmetros
      5,                   // Prioridade da task
      NULL,                // Sem handle
      1                    // Task executada no núcleo 1
   );

   xTaskCreatePinnedToCore(
      taskEnvioLora, // Função que rodará na task
      "EnvioLoRa",   // Nome para task
      4096,          // 4kb alocados na stack
      NULL,          // Sem parâmetros
      5,             // Prioridade
      NULL,          // Sem handle
      0              // Executada no núcleo 0
   );
}