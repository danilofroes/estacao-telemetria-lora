/// @brief Classe para o sensor de luminosidade (foto-resistor LDR)

#pragma once

#include "esp_adc/adc_oneshot.h"
#include "hal/adc_types.h"
#include "esp_err.h"
#include "esp_log.h"

#include "eletronica/pinagem.hpp"

class LDR {
    private:
        adc_oneshot_unit_handle_t adcHandle;
        adc_channel_t canal = Pinagem::CANAL_LDR;

        const char* TAG = "LDR";

        void initADC() {
            adc_oneshot_unit_init_cfg_t configADC = {
                .unit_id = ADC_UNIT_1,
                .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
                .ulp_mode = ADC_ULP_MODE_DISABLE
            };

            ESP_ERROR_CHECK(adc_oneshot_new_unit(&configADC, &adcHandle));

            adc_oneshot_chan_cfg_t configCanal = {
                .atten = ADC_ATTEN_DB_12,
                .bitwidth = ADC_BITWIDTH_DEFAULT
            };

            ESP_ERROR_CHECK(adc_oneshot_config_channel(adcHandle, canal, &configCanal));

            ESP_LOGI(TAG, "ADC inicializado no canal %d", canal);
        }

    public:
        LDR() {}

        ~LDR() {
            ESP_ERROR_CHECK(adc_oneshot_del_unit(adcHandle));
            ESP_LOGI(TAG, "Unidade ADC deletada");
        }

        void begin() {
            initADC();
        }

        int lerRaw() {
            int valorRaw = 0;

            esp_err_t err = adc_oneshot_read(adcHandle, canal, &valorRaw);

            if (err = ESP_OK) {
                return valorRaw;
            }

            else {
                ESP_LOGE(TAG, "Erro na leitura do LDR: %s", esp_err_to_name(err));
                return -1;
            }
        }

        float lerSensor() {
            int raw = lerRaw();

            if (raw < 0) return -1.0f;

            return (raw / 4095.0f) * 100.0f;
        }
};