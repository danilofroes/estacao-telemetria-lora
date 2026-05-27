/// @brief Classe para o sensor de umidade e temperatura DHT11

#pragma once

#include <ctype.h>
#include <array>

#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_attr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "eletronica/pinagem.hpp"

/**
 * @note Sobre o funcionamento do DHT11:
 * Possui protocolo próprio (single-bus), seguiremos as orietações do datasheet para seu funcionamento.
 * Será conectado com sensor PULL-UP externo para garantia de funcionamento ideal.
 * 
 * Recebe uma chamada LOW do ESP por 18ms (output), após isso, funcionará como input, com o DHT puxando para LOW por 80us.
 * Logo em seguida envia os 40 bits em sequência, todo bit começará com pulso LOW de 50us e ficará em HIGH após isso.
 * Se ficar em HIGH por cerca de 28us é bit 0, se ficar por 70us é bit 1.
 * 
 * Os 40 bits (5 bytes) são:
 * Byte 0: Umidade (parte inteira)
 * Byte 1: Umidade (parte decimal)
 * Byte 2: Temperatura (parte inteira)
 * Byte 3: Temperatura (parte decimal)
 * Byte 4: Checksum
 */

class DHT {
    private:
        static constexpr char* TAG = "DHT11"; // Tag para logs

        static constexpr uint8_t TIMEOUT_RESPOSTA_US = 100; // Tempo máximo para aguardar as respostas do sensor (em microsegundos)
        static constexpr uint8_t TIMEOUT_PULSO_US = 60;     // Tempo máximo para aguardar os pulsos do sensor (em microsegundos)
        static constexpr uint8_t LIMITE_BIT_1_US = 40;      // Limite de tempo para diferenciar bit 0 de bit 1 (em microsegundos)
        
        gpio_num_t pino; // Pino GPIO conectado ao sensor DHT11

        std::array<uint8_t, 5> dados; // Array de 5 bytes (40 bits) para armazenar os dados lidos

        float umidade = 0.0f;     // Variável para armazenar a umidade lida
        float temperatura = 0.0f; // Variável para armazenar a temperatura lida

        portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED; // Mutex para proteger a leitura dos dados

        /**
         * @brief Função para aguardar o sensor mudar para o estado esperado (HIGH ou LOW) com timeout
         * 
         * @param estadoEsperado O estado que estamos aguardando (0 para LOW, 1 para HIGH)
         * @param timeoutUs O tempo máximo para aguardar o estado esperado (em microsegundos)
         * 
         * @return O tempo em microsegundos que levou para o sensor atingir o estado esperado, ou -1 se ocorreu um timeout
         * 
         * @note A flag IRAM_ATTR é usada para garantir que essa função seja executada na RAM, evitando atrasos causados 
         * por acesso à memória flash, sendo crucial para a precisão na leitura dos pulsos do sensor.
         */
        IRAM_ATTR int esperarEstado(int estadoEsperado, int timeoutUs) {
            int usEsperados = 0;

            while (gpio_get_level(pino) != estadoEsperado) {
                if (usEsperados > timeoutUs) {
                    return -1; // Retorna um timeout caso o sensor não responda a tempo
                }

                esp_rom_delay_us(1); // Aguarda 1us (essa função trava a CPU para obter precisão na leitura)
                usEsperados++;
            }

            return usEsperados;
        }

        /**
         * @brief Função para realizar o handshake inicial com o sensor DHT11, seguindo o protocolo descrito no datasheet
         * 
         * @return ESP_OK se o handshake foi bem-sucedido, ou um código de erro específico caso tenha falhado
         */
        esp_err_t handshake() {
            gpio_set_direction(pino, GPIO_MODE_OUTPUT); // Definindo o GPIO como saída
            gpio_set_level(pino, 0); // Pino como LOW
            vTaskDelay(pdMS_TO_TICKS(18)); // Aguarda 18ms

            gpio_set_direction(pino, GPIO_MODE_INPUT); // Define o pino como entrada
            
            if (esperarEstado(0, TIMEOUT_PULSO_US) == -1) {
                ESP_LOGE(TAG, "Timeout: Sensor não iniciou o pulso de LOW na resposta");
                return ESP_ERR_TIMEOUT;
            }

            if (esperarEstado(1, TIMEOUT_RESPOSTA_US) == -1) {
                ESP_LOGE(TAG, "Timeout: Sensor não finalizou o pulso LOW de resposta.");
                return ESP_ERR_TIMEOUT;
            }

            if (esperarEstado(0, TIMEOUT_RESPOSTA_US) == -1) {
                ESP_LOGE(TAG, "Timeout: Sensor não iniciou a transmissão de dados.");
                return ESP_ERR_TIMEOUT;
            }

            return ESP_OK;
        }

        /**
         * @brief Função para ler os 40 bits de dados do sensor DHT11
         * 
         * @return ESP_OK se a leitura foi bem-sucedida, ou um código de erro específico caso tenha falhado
         * 
         * @note A flag IRAM_ATTR também é usada aqui para garantir que a função seja executada na RAM
         */
        IRAM_ATTR esp_err_t lerDados() {
            dados.fill(0); // Limpando o array de dados para evitar lixo de memória e inicializando com 0s

            // Passando por todos 40 bits
            for (uint8_t i = 0; i < LIMITE_BIT_1_US; i++) {

                // Aguarda o pulso LOW inicial de aproximadamente 50us acabar
                if (esperarEstado(1, TIMEOUT_PULSO_US) == -1) {
                    ESP_LOGE(TAG, "Timeout: Falha ao aguardar inicio do bit %d", i);
                    return ESP_ERR_TIMEOUT;
                }

                // Contará o tempo que a linha ficará em HIGH (são 70us para bit 1 e 28us para bit 0)
                int duracaoHigh = esperarEstado(0, 100);

                if (duracaoHigh == -1) {
                    ESP_LOGE(TAG, "Timeout: Falha ao ler a duracao do bit %d", i);
                    return ESP_ERR_TIMEOUT;
                }

                // Se o pulso HIGH tiver tido mais de 40us, consideraremos bit 1 (Tendo em vista que são 28us para 0 e 70us para 1)
                if (duracaoHigh > LIMITE_BIT_1_US) {
                    uint8_t indiceByte = i / 8;
                    uint8_t posicaoBit = 7 - (i % 8);
                    dados[indiceByte] |= (1 << posicaoBit);
                }
                // Caso contrário, não fazemos nada, pois inicializamos o array preenchido com 0s
            }

            return ESP_OK;
        }

        /**
         * @brief Função para decodificar os dados lidos do sensor DHT11, verificando o checksum e convertendo os valores para temperatura e umidade
         * 
         * @return ESP_OK se a decodificação foi bem-sucedida, ou um código de erro específico caso tenha falhado (como erro de checksum)
         */
        esp_err_t decodificador() {
            // Cálculo de verificação do checksum com máscara de bits para limitar a 8 bits
            uint8_t checksum = (dados[0] + dados[1] + dados[2] + dados[3]) & 0xFF;

            if (checksum != dados[4]) {
                ESP_LOGE(TAG, "Erro de Checksum! Lidos: %d, Esperado: %d", dados[4], checksum);
                return ESP_ERR_INVALID_CRC;
            }

            // Para decodificar os valores, usa-se a fórmula padrão, somando as partes inteiras e decimais e dividindo por 10
            umidade = dados[0] + (dados[1] / 10.0f);
            temperatura = dados[2] + (dados[3] / 10.0f);

            ESP_LOGI(TAG, "Leitura OK! Temp: %.1f C, Umi: %.1f %%", temperatura, umidade);
    
            return ESP_OK;
        }
    
    public:
        explicit DHT(uint8_t pino) : pino(static_cast<gpio_num_t>(pino)) {}

        ~DHT() {}

        /// @brief Função para inicializar o sensor DHT11, configurando o pino GPIO corretamente
        void begin() {
            gpio_config_t configPino = {
                .pin_bit_mask = 1ULL << pino,
                .mode         = GPIO_MODE_INPUT_OUTPUT,
                .pull_up_en   = GPIO_PULLUP_DISABLE,
                .pull_down_en = GPIO_PULLDOWN_DISABLE,
                .intr_type    = GPIO_INTR_DISABLE
            };

            ESP_ERROR_CHECK(gpio_config(&configPino));
        }

        /**
         * @brief Função para realizar a leitura completa do sensor DHT11, incluindo o handshake, leitura dos dados e decodificação,
         * protegendo a seção crítica com mutex para garantir a integridade dos dados lidos
         * 
         * @return ESP_OK se a leitura e decodificação foram bem-sucedidas, ou um código de erro específico
         */
        esp_err_t leitura() {
            esp_err_t err;

            err = handshake();
            if (err != ESP_OK) return err;

            portENTER_CRITICAL(&mux); // Entrando na seção crítica para proteger a leitura dos dados
            err = lerDados();
            portEXIT_CRITICAL(&mux); // Saindo da seção crítica
            if (err != ESP_OK) return err;

            return decodificador();
        }

        /**
         * @brief Função para obter a temperatura lida do sensor DHT11
         * @return A temperatura em graus Celsius, ou 0.0f se a leitura ainda não foi realizada ou falhou
         */
        float getTemperatura() const { return temperatura; }

        /**
         * @brief Função para obter a umidade lida do sensor DHT11
         * @return A umidade em porcentagem, ou 0.0f se a leitura ainda não foi realizada ou falhou
         */
        float getUmidade() const { return umidade; }
};