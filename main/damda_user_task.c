
#include "damda_ESP32_Project.h"

static const char *TAG = "user_events";
//==============================================================================================//

//unsigned int user_task_count = 0;

void user_task(void *arg)
{
//    ESP_LOGI(TAG, "user_task started");
    while (1) {
        vTaskDelay(100 / portTICK_RATE_MS); //태스크를 틱(Tick) 단위로 지연. 이 시간동안 태스크를 블록(BLOCK) 상태로 유지
         
//		user_task_count++;
//          ESP_LOGI(TAG, "user_task_count0: %d ", user_task_count);	 
    }
    vTaskDelete(NULL);
}

void user_task1(void *arg)
{
//    ESP_LOGI(TAG, "user_task1 started");
    while (1) {
        vTaskDelay(100 / portTICK_RATE_MS);
         
//		user_task_count++;
//          ESP_LOGI(TAG, "user_task_count1: %d ", user_task_count);	 
    }
    vTaskDelete(NULL);
}

void user_task_init(void) 
{
   
    xTaskCreatePinnedToCore(user_task, "user_task", 2048, NULL, 5, NULL,tskNO_AFFINITY);
    xTaskCreatePinnedToCore(user_task1, "user_task1", 2048, NULL, 5, NULL,tskNO_AFFINITY);
}

