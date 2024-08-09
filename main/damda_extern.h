
//###########################################################################
//
// FILE:   damda_extern.h
//
// TITLE:  DAMDA_EXTERN Define File and Global !!!
//
//###########################################################################

#ifndef DAMDA_EXTERN_H
#define DAMDA_EXTERN_H

//#define VER_STRING                  "1.2"

//#define PN_manufacturer
//#define DIMCHAE_manufacturer
// #define DUMMY_DATA

#define DEBUG

/* ---Definition for Operation Platform Mode(AWS/DAMDA)--- */

// #define Operation_Mode_Certification 


// Task priority
#define PRIORITY_MAIN                   7
#define PRIORITY_USER                   8
#define PRIORITY_UART_TX          11
#define PRIORITY_UART_RX          12
//----------------------------- App_main -----------------------------//

/* ---ESP32 device AP Infomation--- */
#ifdef PN_manufacturer
    #define EXAMPLE_ESP_WIFI_SSID      "ESP32_PN2"
#endif

#ifdef DIMCHAE_manufacturer
    #define EXAMPLE_ESP_WIFI_SSID      "ESP32_DIMCHAE"
#endif

#ifdef DUMMY_DATA
    #define EXAMPLE_ESP_WIFI_SSID      DEVICE_MANUFACTURER
#endif


#define EXAMPLE_ESP_WIFI_SSID      "DAMDA_WIFI_TEST"
#define EXAMPLE_ESP_WIFI_PASS      "123456789"
#define EXAMPLE_ESP_WIFI_CHANNEL   0
#define EXAMPLE_MAX_STA_CONN       4
#define EXAMPLE_ESP_MAXIMUM_RETRY  10

/* ---device Infomation--- */
uint8_t eth_mac[6];
char DEVICE_ETH_MAC             [20];
char DEVICE_FIRMWAREVERSION             [10];

#ifdef PN_manufacturer
    #define DEVICE_MANUFACTURER 		"PN"
    #define DEVICE_MODEL 				"PN_PLUG"
    #define DEVICE_TYPE 				"PLUG"
#endif

#ifdef DIMCHAE_manufacturer
    #define DEVICE_MANUFACTURER 		"WINIADIMCHAE"
    #define DEVICE_MODEL 				"WD_REFRIGERATOR2"
    #define DEVICE_TYPE 				"REFRIGERATOR2"
#endif


/* Some commonly used content types */

#define HTTPD_TYPE_JSON   "application/json"            /*!< HTTP Content type JSON */
#define HTTPD_TYPE_TEXT   "text/html"                   /*!< HTTP Content type text/HTML */
#define HTTPD_TYPE_OCTET  "application/octet-stream"    /*!< HTTP Content type octext-stream */

/* Some commonly used status codes */

#define HTTPD_200      "200 OK"                     /*!< HTTP Response 200 */
#define HTTPD_204      "204 No Content"             /*!< HTTP Response 204 */
#define HTTPD_207      "207 Multi-Status"           /*!< HTTP Response 207 */
#define HTTPD_400      "400 Bad Request"            /*!< HTTP Response 400 */
#define HTTPD_404      "404 Not Found"              /*!< HTTP Response 404 */
#define HTTPD_408      "408 Request Timeout"        /*!< HTTP Response 408 */
#define HTTPD_500      "500 Internal Server Error"  /*!< HTTP Response 500 */

//---- App_main ----//

#define device_serial_address

/* wifi connected bit */
#define WIFI_CONNECTED_BIT      BIT0
#define WIFI_FAIL_BIT           BIT1

#define Q_NUM                   (10)

typedef void (*message_handler_t)(uint32_t arg);

typedef struct
{
    message_handler_t function;
    uint32_t arg;
} main_event_t;

time_t obtain_time(void);

bool mqtt_start_flag ;

//----------------------------- kea_uart -----------------------------//
void damda_uart_init(void); 

//----------------------------- kea_user -----------------------------//
void user_task_init(void);

//----------------------------- kea_nvs -----------------------------//

#define STORAGE_NAMESPACE           "storage"
#define MAX_KEY_NODE                6

typedef enum {
    INIT_MODE = 0x55,
    NORMAL_MODE = 0x00,
    FACTORY_MODE = 0xAA,
} booting_mode_type_t;

typedef struct{
    uint8_t         booting_mode;
    
   	char 			esp_mac [20] ;
    char            ver_string[30];
    bool            ota_mode_fail;
    uint32_t		igain;
    uint32_t		vgain;
    uint32_t		sgain;
} flash_info_t;

flash_info_t flash_info;

void damda_nvs_init(void);
void read_flash(flash_info_t *pdata);
void write_flash(flash_info_t *pdata);
void reset_flash(flash_info_t* pdata);

//----------------------------- kea_ota -----------------------------//

void ota_task_init(void); 
void OTA_upgrade_fail(void);

char            ota_url[200];
char            ota_version[10];

bool busy_flag;
#endif //DAMDA_EXTERN_H

