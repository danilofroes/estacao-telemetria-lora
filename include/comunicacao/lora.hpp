/// @brief Classe base para comunicação com o módulo LoRa SX1276 via SPI no ESP-IDF

#pragma once

#include <cstring>
#include <array>

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "eletronica.hpp"

class LoRa {
    private:
        static constexpr const char* TAG = "SX1276";

        spi_device_handle_t spi;

        gpio_num_t pinoMISO = ESP::PINO_MISO;
        gpio_num_t pinoMOSI = ESP::PINO_MOSI;
        gpio_num_t pinoSCK  = ESP::PINO_SCK;
        gpio_num_t pinoSS   = ESP::PINO_SS;
        gpio_num_t pinoRST  = ESP::PINO_RST;

        /**
         * @brief Função para escrever um valor em um registrador do módulo SX1276 via SPI
         * 
         * @param reg O endereço do registrador que se deseja escrever
         * @param valor O valor que se deseja escrever no registrador
         */
        void writeRegister(uint8_t reg, uint8_t valor) {
            // Alterando o bit 7 do registrador para 1 indicando que é operação de escrita e armazenando junto com o valor no array
            std::array<uint8_t, 2> txData = {static_cast<uint8_t>(reg | 0x80), valor};

            spi_transaction_t transacao = {
                .length = 16,               // 1 byte para o registrador e 1 byte para o valor
                .tx_buffer = txData.data(), // Enviando o array de 2 bytes (registrador + valor)
                .rx_buffer = nullptr        // Sem buffer de rx
            };

            ESP_ERROR_CHECK(spi_device_polling_transmit(spi, &transacao)); // Transmitindo a transação via SPI
        }

        /**
         * @brief Função para ler o valor de um registrador do módulo SX1276 via SPI
         * 
         * @param reg O endereço do registrador que se deseja ler
         * @return O valor lido do registrador especificado
         */
        uint8_t readRegister(uint8_t reg) {
            // Alterando o bit 7 do registrador para 0 indicando que é operação de leitura e armazenando no array
            std::array<uint8_t, 2> txData = {static_cast<uint8_t>(reg & 0x7F), 0x00};
            std::array<uint8_t, 2> rxData = {0x00, 0x00}; // Array para armazenar os dados recebidos (registrador + valor)

            spi_transaction_t transacao = {
                .length = 16,               // 1 byte para o registrador e 1 byte para o valor
                .tx_buffer = txData.data(), // Enviando o array de 2 bytes
                .rx_buffer = rxData.data()  // Recebendo os dados no array de 2 bytes
            };

            ESP_ERROR_CHECK(spi_device_polling_transmit(spi, &transacao)); // Transmitindo a transação via SPI

            return rxData[1]; // Retornando o valor lido do registrador
        }

        /// @brief Função para inicializar o barramento SPI
        void initSPI() {
            spi_bus_config_t busConfig = {
                .mosi_io_num = pinoMOSI,
                .miso_io_num = pinoMISO,
                .sclk_io_num = pinoSCK,
                .quadwp_io_num = -1,
                .quadhd_io_num = -1,
                .max_transfer_sz = 0
            };

            ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &busConfig, SPI_DMA_CH_AUTO)); // Usando no SPI2 (HSPI)
        }

        /// @brief Função para inicializar o dispositivo SPI para o módulo SX1276
        void initSX() {
            // Configurando o dispositivo SPI para o módulo SX1276
            spi_device_interface_config_t devConfig = {
                .mode = 0, // Modo SPI 0
                .clock_source = SPI_CLK_SRC_DEFAULT,
                .clock_speed_hz = 5'000'000, // 5 MHz
                .spics_io_num = pinoSS,
                .queue_size = 1,
            };

            ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devConfig, &spi));
        }
        
    public:
        LoRa() {}

        /**
         * @brief Função para inicializar corretamente toda configuração do módulo SX e do barramento SPI
         * 
         * @return true se a inicialização for feita corretamente, false se contrário
         */
        bool begin() {
            // Configuração do pino de reset do módulo LoRa
            gpio_set_direction(pinoRST, GPIO_MODE_OUTPUT);
            gpio_set_level(pinoRST, 0);
            vTaskDelay(pdMS_TO_TICKS(10));
            gpio_set_level(pinoRST, 1);
            vTaskDelay(pdMS_TO_TICKS(10));

            initSPI();
            initSX();

            ESP_LOGI(TAG, "SPI inicializado!");

            // Verificando se o módulo está conectado corretamente (deve retornar a versão 0x12 se sim)
            uint8_t versao = readRegister(0x42);
            if (versao != 0x12) {
                ESP_LOGE(TAG, "Falha ao encontrar SX1276. Versão lida: 0x%02X", versao);
                return false;
            }
            ESP_LOGI(TAG, "SX1276 detectado com sucesso!");

            // Configurando o modo de operação do módulo
            writeRegister(SX::REG_OP_MODE, SX::MODE_SLEEP); // Módulo no modo sleep para configuração inicial
            writeRegister(SX::REG_OP_MODE, SX::MODE_LONG_RANGE_MODE | SX::MODE_SLEEP); // Habilita o modo de longo alcance (LoRa) e mantém no modo sleep
            writeRegister(SX::REG_OP_MODE, SX::MODE_LONG_RANGE_MODE | SX::MODE_STDBY); // Vai para o modo standby

            // Configurando RF com frequência de 915MHz (padrão Brasil)
            // FRF = 915MHz / (32MHz / 2^19) = 0xE4C000
            writeRegister(SX::REG_FRF_MSB, 0xE4);
            writeRegister(SX::REG_FRF_MID, 0xC0);
            writeRegister(SX::REG_FRF_LSB, 0x00);

            writeRegister(SX::REG_PA_CONFIG, 0x8F); // Configuração de potência máxima para transmissão

            return true;
        }
        
        /**
         * @brief Função para transmitir dados através do módulo LoRa
         * 
         * @param buffer O buffer contendo os dados a serem transmitidos
         * @param tamanho O tamanho dos dados a serem transmitidos
         */
        void transmitirDados(uint8_t* buffer, size_t tamanho) {
            writeRegister(SX::REG_OP_MODE, SX::MODE_LONG_RANGE_MODE | SX::MODE_STDBY); // Garantindo que o módulo esteja no modo standby

            writeRegister(SX::REG_FIFO_ADDR_PTR, readRegister(SX::REG_FIFO_TX_BASE_ADDR)); // Apontando o ponteiro do FIFO para o endereço base de transmissão
            writeRegister(SX::REG_PAYLOAD_LENGTH, tamanho); // Configurando o comprimento do payload a ser transmitido

            // Escrevendo os dados no FIFO
            for (size_t i = 0; i < tamanho; i++) {
                writeRegister(SX::REG_FIFO, buffer[i]);
            }

            writeRegister(SX::REG_OP_MODE, SX::MODE_LONG_RANGE_MODE | SX::MODE_TX); // Iniciando a transmissão

            // Polling aguardando TX_DONE para garantir que a transmissão foi concluída antes de retornar
            while ((readRegister(SX::REG_IRQ_FLAGS) & SX::IRQ_TX_DONE_MASK) == 0) {
                vTaskDelay(pdMS_TO_TICKS(2)); // Aguardando a transmissão ser concluída
            }

            // Limpando as flags de interrupção
            writeRegister(SX::REG_IRQ_FLAGS, SX::IRQ_TX_DONE_MASK);
            ESP_LOGI(TAG, "Dados transmitidos com sucesso!");
        }
};