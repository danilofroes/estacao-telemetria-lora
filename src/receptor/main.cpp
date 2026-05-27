/**
 * @file main.cpp
 * 
 * @brief Arquivo principal do ESP32 receptor de telemetria LoRa
 * Esse receptor será responsável por receber os dados de telemetria enviados pelo transmissor via LoRa,
 * processar os dados recebidos e, enviá-los para um servidor remoto MQTT via Wi-Fi.
 * 
 * @author Danilo Fróes
 * @date 2026
 */

 #include "freertos/FreeRTOS.h"
 #include "freertos/task.h"

 extern "C" {
    void app_main(void);
 }

 void app_main(void) {

    while (true) {

        vTaskDelay(pdMS_TO_TICKS(10));
    }
 }