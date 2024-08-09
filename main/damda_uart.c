
#include "damda_ESP32_Project.h"
#include "freertos/FreeRTOS.h"

static const char *TAG = "damda_uart_events";

extern void UART_Command();
extern void Test_Command();

static const int BUF_SIZE = 1024;
static QueueHandle_t uart0_queue;

unsigned int _10mil_sec_rx = 0;

static void rx_task(void *arg)
{
    ESP_LOGI(TAG, "rx_task started");
    
    while (true)
    {
        _10mil_sec_rx++;
         if(_10mil_sec_rx >=50)
        {
             _10mil_sec_rx = 0;
             
             Test_Command();
             UART_Command();
             //comand_action();
         }
         
         vTaskDelay(10 / portTICK_RATE_MS); // portTICK_RATE_MS = 1000
    }
	vTaskDelete(NULL);
}

void damda_uart_init(void) 
{
    esp_log_level_set(TAG, ESP_LOG_INFO);
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_0, BUF_SIZE,  BUF_SIZE, 20, &uart0_queue, 0);
    uart_param_config(UART_NUM_0, &uart_config);
    
    xTaskCreatePinnedToCore(rx_task, "uart_rx_task", BUF_SIZE*8, NULL, PRIORITY_UART_RX, NULL,APP_CPU_NUM);
}

