
//###########################################################################
//
// FILE:   damda_ESP32_Project.h
//
// TITLE:  damda_ESP32_Project Headerfile and Examples Include File
//
//###########################################################################

#ifndef DAMDA_ESP32_PROJECT_H
#define DAMDA_ESP32_PROJECT_H

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"


#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_netif_defaults.h"
#include <esp_http_server.h>
#include <sys/param.h>
#include "mqtt_client.h"

#include "esp_sntp.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <unistd.h>


///////////////////////////
#include "damdaCore.h"
#include "damda_extern.h"



#endif //DAMDA_ESP32_PROJECT_H

