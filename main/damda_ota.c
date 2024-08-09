/* OTA example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "damda_ESP32_Project.h"
#include "damdaCore.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
//#include "protocol_examples_common.h"
#include "string.h"

#include "nvs.h"
#include "nvs_flash.h"
//#include "protocol_examples_common.h"

#if CONFIG_EXAMPLE_CONNECT_WIFI
#include "esp_wifi.h"
#endif

static const char *OTAG = "damda_ota";

#define OTA_URL_SIZE 256

extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(OTAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(OTAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(OTAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(OTAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(OTAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(OTAG, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGD(OTAG, "HTTP_EVENT_DISCONNECTED");
        break;
    }
    return ESP_OK;
}

void simple_ota_example_task(void *pvParameter)
{
    esp_log_level_set("*", ESP_LOG_DEBUG);
    esp_log_level_set(OTAG, ESP_LOG_DEBUG);

    ESP_LOGI(OTAG, "Starting OTA example");

    ESP_LOGD(OTAG, "OTA URL: %s", ota_url);
    printf("1111111111 OTA URL: %s\n", ota_url);

    esp_http_client_config_t config = {
       
        //.url = "http://iot.godamda.kr:15050/api/fota/v1/WINIAELECTRONICS/WN_ULTFREEZER/0.03",    //ota_url,        
        
        .url = ota_url,
        .cert_pem = ca_pem,
        .event_handler = _http_event_handler,
        .timeout_ms = 5000, // 타임아웃 설정 
        .skip_cert_common_name_check = true, // 인증서 공통 이름 검증 건너뛰기
        //.cert_pem = NULL,
    };

    // 디버그 로그 추가
    // ESP_LOGD(OTAG, "OTA cert_pem: %s", config.cert_pem);
    printf("2222222222 cert_pem URL: %s\n", config.cert_pem);

#ifdef CONFIG_EXAMPLE_FIRMWARE_UPGRADE_URL_FROM_STDIN
    char url_buf[OTA_URL_SIZE];
    if (strcmp(config.url, "FROM_STDIN") == 0) {
        example_configure_stdin_stdout();
        fgets(url_buf, OTA_URL_SIZE, stdin);
        int len = strlen(url_buf);
        url_buf[len - 1] = '\0';
        config.url = url_buf;
    } else {
        ESP_LOGE(OTAG, "Configuration mismatch: wrong firmware upgrade image url");
        abort();
    }
#endif

#ifdef CONFIG_EXAMPLE_SKIP_COMMON_NAME_CHECK
    config.skip_cert_common_name_check = true;
    ESP_LOGD(OTAG, "Skipping certificate common name check");
#endif

    ESP_LOGI(OTAG, "Starting OTA process");

    int retry_count = 0;
    const int max_retries = 5;
    esp_err_t ret;

    printf("333333333 확인\n");
    while (retry_count < max_retries) {
        ret = esp_https_ota(&config);
        if (ret == ESP_OK) {
            ESP_LOGI(OTAG, "OTA Succeed, Rebooting...");
            extern int ucnotify_flag;
            strcpy(flash_info.ver_string, ota_version);
            write_flash(&flash_info);

            ESP_LOGI(OTAG, "FOTA update done, Send device update information \r\n");
            ucnotify_flag = 0;
            damda_Req_UpdateDeviceInfo(); // Send Updated device information after successful FOTA
            vTaskDelay(1000 / portTICK_PERIOD_MS);

            ESP_LOGI(OTAG, "Restarting device after successful FOTA update \r\n");
            esp_restart();
        } else {
            ESP_LOGE(OTAG, "OTA failed, attempt %d of %d", retry_count + 1, max_retries);
            ESP_LOGE(OTAG, "Error: %s", esp_err_to_name(ret));  // 오류 메시지 추가
            if (ret == ESP_ERR_OTA_PARTITION_CONFLICT) {
                ESP_LOGE(OTAG, "Partition conflict, please check your partition table.");
                break; // 파티션 충돌 시 재시도 하지 않음
            }
            retry_count++;
            vTaskDelay(5000 / portTICK_PERIOD_MS); // 5초 대기 후 재시도
        }
    }

    ESP_LOGE(OTAG, "OTA failed after %d attempts", max_retries);
    flash_info.ota_mode_fail = true;
    write_flash(&flash_info);
    esp_restart();

    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void ota_task_init(void)
{
    
   // ESP_ERROR_CHECK(esp_netif_init());
    //ESP_ERROR_CHECK(esp_event_loop_create_default());
       xTaskCreate(&simple_ota_example_task, "ota_example_task", 8192, NULL, 5, NULL);
}
