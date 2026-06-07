/**
 * @file config.hpp
 * 
 * @brief Arquivo de configuração global para o transmissor de telemetria LoRa
 */

#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#include "eletronica.hpp"
#include "sensores/ldr.hpp"
#include "sensores/dht.hpp"

static const char* TAG = "Transmissor"; // Tag para logs

/// @brief Estrutura para armazenar os dados de telemetria dos sensores
struct DadosTelemetria {
   float temperatura;
   float umidade;
   float luminosidade;
};

QueueHandle_t filaDados; // Handle da fila de comunicação entre tarefas

DHT sensorDHT(Pinagem::PINO_DHT); // Instância do sensor DHT11
LDR sensorLDR; // Instância do sensor LDR

/**
 * @brief Task responsável por fazer a leitura dos sensores DHT11 e LDR, armazenando os dados no struct e enviando para a fila de comunicação
 */
void taskLeituraSensores(void *pvParameters) {
   ESP_LOGI(TAG, "Task de Leitura iniciada no Core %d", xPortGetCoreID());

   sensorDHT.begin(); // Inicializando o sensor DHT11
   sensorLDR.begin(); // Inicializando o sensor LDR

   DadosTelemetria dados; // Instância para armazenar os dados lidos

   // Loop infinito para leitura dos sensores
   for (;;) {
      dados.luminosidade = sensorLDR.lerSensor(); // Lendo o valor do LDR

      // Iniciando leitura do DHT11 e verificando se foi bem-sucedida
      if (sensorDHT.leitura() == ESP_OK) {
         dados.temperatura = sensorDHT.getTemperatura(); // Lendo a temperatura do DHT11
         dados.umidade = sensorDHT.getUmidade();         // Lendo a umidade do DHT11
      } 
      
      else {
         ESP_LOGE(TAG, "Erro na leitura do DHT11");
         dados.temperatura = -273.15f; // Indicando erro na leitura da temperatura com valor absurdo do zero absoluto
         dados.umidade = -1.0f;        // Indicando erro na leitura da umidade
      }

      // Log com todos os dados obtidos
      ESP_LOGI(TAG, "[Core %d] Sensores Lidos - Temp: %.1fC | Umi: %.1f%% | Lum: %.1f%%", 
               xPortGetCoreID(), dados.temperatura, dados.umidade, dados.luminosidade);

      // Verifica se os dados foram enviados para a fila com sucesso
      if (xQueueSend(filaDados, &dados, pdMS_TO_TICKS(100)) != pdTRUE) {
         ESP_LOGW(TAG, "Fila cheia, dados de telemetria descartados");
      }

      vTaskDelay(pdMS_TO_TICKS(5000)); // Aguardando 5 segundos antes da próxima leitura
   }
}

void taskEnvioLora(void *pvParameters) {
   ESP_LOGI(TAG, "Task de envio via LoRa iniciada no Core %d", xPortGetCoreID());

   DadosTelemetria dadosRecebidos; // Struct com os dados recebidos

   for (;;) {
      // Recebendo dados da fila, a task vai ficar bloqueada até o momento que haja dados na fila
      if (xQueueReceive(filaDados, &dadosRecebidos, portMAX_DELAY) == pdTRUE) {
         ESP_LOGI(TAG, "[Core %d] Preparando para transmitir dados. Temp base: %.1fC...", 
                  xPortGetCoreID(), dadosRecebidos.temperatura);
      }

      vTaskDelay(pdMS_TO_TICKS(100));
   }
}