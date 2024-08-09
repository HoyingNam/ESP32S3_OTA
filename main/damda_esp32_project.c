#include <stdio.h>

#include "device_rsc.h"
#include "damda_ESP32_Project.h"
#include "damda_nvs.c"
#include "nvs_flash.h"
#include "damdaMQTTClient.c"
#include "damdaSDKCore.c"
#include "damda_ota.c"


#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)
/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

static int s_retry_num = 0;

nvs_handle_t nvs;

tcpip_adapter_ip_info_t ipInfo;

typedef uint64_t UINT64;

char* server_addr_buffer = " ";
char* server_port_buffer = " ";
char* ap_ssid_buffer = " ";
char* ap_password_buffer = " ";
char* auth_key_buffer = " ";

char* ap_ssid_buffer_b = " ";
char* ap_password_buffer_b = " ";

unsigned int _mil_sec = 0;

int ucnotify_flag = 0;
char message_print_flag = 0;
char now_time[16] = "";

uint8_t base_mac_addr[6] = {0};	
static httpd_handle_t server = NULL;

int ap_state = 0;

bool ip_get_flag ;

bool init_sntp = false;
bool isForcedWifiStop = false;

//time_t obtain_time(void);
//void OTA_upgrade_fail(void);

static esp_netif_t *ap_netif = NULL; // AP 네트워크 인터페이스 포인터를 저장할 변수

void Notify_Func();

static const char *TAG = "APP_MAIN";
static esp_netif_t* sta_netif;

//=====================================================================================================

int esp_compare_bufferData();
void esp_nvs_set_backupData();
wifi_config_t esp_nvs_get_backupData();
char * esp_nvs_load_value_if_exist();

void test_init();
void print_receiveData(ResponseMessageInfo* responseInfo);

int getCntSplit(char* msg, char* split){
    int i=0;
    char *currPoint=NULL;
    char *prevPoint=msg;
    do{
        currPoint=strstr(prevPoint, split);
        printf("split currPoint : %s\n", currPoint==NULL?"NULL":currPoint);
        if(currPoint!=NULL){
            prevPoint=currPoint+strlen(split);
        }
    } while(currPoint!=NULL && ++i);
    if(i>0) i++;

    return i;
}

int split(char* msg, char* split, char*** result){
    int i=0;
    int charCount=3;
    int totalCount=0;
    char *prevPoint=msg;
    char *currPoint=NULL;
    char **array2d=NULL;
    int splitCount = getCntSplit(msg, split);

    array2d=(char**)malloc(sizeof(char*)*(splitCount));
    for(int i=0; i < splitCount; i++){
        currPoint=strstr(prevPoint, split);
        printf("split currPoint : %s\n", currPoint==NULL?"NULL":currPoint);
        if(i<(splitCount-1)){
            if(currPoint!=NULL){
                totalCount=currPoint-msg;
                if(prevPoint==msg) charCount=totalCount;
                else charCount=currPoint-prevPoint;                
                array2d[i]=(char*)malloc(sizeof(char)*30);
                strncpy(array2d[i], prevPoint, charCount);
                
                array2d[i][charCount]='\0';
                prevPoint=currPoint+strlen(split);
            }
        } else {
            charCount=strlen(msg)-totalCount; 
            array2d[i]=(char*)malloc(sizeof(char)*30);
            strncpy(array2d[i], prevPoint, charCount);
            array2d[i][charCount]='\0';
        }
    }
    *result = array2d;

    return splitCount;
}

void freeSplit(char** result, int count){
    //printf("size:%d\n", count);
    while(--count>-1){
        /*printf("%d. %s[%x]\n", count, result[count], result[count]);*/
        free(result[count]);
    }
    /*printf("%x\n", result);*/
    free(result);
}

void parseJsonRscData(){
    int i = 0;
    ESP_LOGE(TAG, "json_data : %s", deviceRscData);
    cJSON *jsonRscData = cJSON_Parse(deviceRscData);
    
    ESP_LOGE(TAG, "json_data size : %i", cJSON_GetArraySize(jsonRscData));
    for (i = 0 ; i < cJSON_GetArraySize(jsonRscData) ; i++)
    {
        cJSON * subitem = cJSON_GetArrayItem(jsonRscData, i);
        char * jsonOid = cJSON_GetStringValue(cJSON_GetObjectItem(subitem, RSC_OID));
        char * jsonRid = cJSON_GetStringValue(cJSON_GetObjectItem(subitem, RSC_RID));
        char * jsonCount = cJSON_GetStringValue(cJSON_GetObjectItem(subitem, RSC_COUNT));
        char * jsonType = cJSON_GetStringValue(cJSON_GetObjectItem(subitem, RSC_TYPE));
        char * jsonRange = cJSON_GetStringValue(cJSON_GetObjectItem(subitem, RSC_RANGE));
        ESP_LOGI(TAG, "oid %s", jsonOid);
        ESP_LOGI(TAG, "rid %s", jsonRid);
        ESP_LOGI(TAG, "count %s", jsonCount); 
        ESP_LOGI(TAG, "type %s", jsonType);
        ESP_LOGI(TAG, "range %s", jsonRange);
    }
}

int getNumberRangeType(char* range, char* sep){
    char *ptr = strtok(range, sep);
    int cnt = 0;
    while (ptr != NULL){
        printf("range %i : %s", cnt, ptr);
        ptr = strtok(NULL, sep);
        cnt++;
    }
    return cnt;
}

void getRscRandomValue(char* type, char* range, char* val){
    srand((unsigned int)time(NULL)); 
    int num = rand();   

    // printf("time for random : %u\n", num); 

    if(strcmp((const char *) type, "B") == 0 || strcmp((const char *) type, "b") == 0){
        if((num%2)==0){
            strcpy(val,"true");
            return;
        }else{
            strcpy(val,"false");
            return;
        }        
    }else if(strcmp((const char *) type, "S") == 0 || strcmp((const char *) type, "s") == 0){
        char** ptr = NULL;
        int cnt = split(range, ",", &ptr);
        
        for(int i=0; i<cnt; i++){
            if(num%cnt == i){
                strcpy(val,ptr[i]);
                freeSplit(ptr, cnt);
                return;
            }
        }        
    }else if(strcmp((const char *) type, "O") == 0 || strcmp((const char *) type, "o") == 0){
        strcpy(val,"Dummy Opaque");
        return;
    }else if(strcmp((const char *) type, "T") == 0 || strcmp((const char *) type, "t") == 0){
        int t = (unsigned int)time(NULL);
        itoa(t, val, 10);
        return;
    }else if(strcmp((const char *) type, "I") == 0 || strcmp((const char *) type, "i") == 0){
        char ** ptr = NULL;
        int cnt = split(range, "~", &ptr);
        if(cnt<=0){
            strcpy(val,"0");
            return;
        } 
        int fromTo[cnt];
        for(int i=0; i<cnt; i++){
            printf("ptr[%i] : %s\n", i, ptr[i]);
            fromTo[i] = atoi(ptr[i]);
            printf("fromTo[%i] : %i\n", i, fromTo[i]);
        }

        int res = num%fromTo[1] + fromTo[0];
        freeSplit(ptr, cnt);
        itoa(res, val, 10);
        printf("val : %s\n", val);
        return;
    }else if(strcmp((const char *) type, "F") == 0 || strcmp((const char *) type, "f") == 0){
        char ** ptr = NULL;
        int cnt = split(range, "~", &ptr);
        if(cnt<=0){
            strcpy(val,"0");
            return;
        } 
        float fromTo[cnt];
        for(int i=0; i<cnt; i++){
            fromTo[i] = atof(ptr[i]);
        }

        float res = num%(int)fromTo[1] + fromTo[0];
        freeSplit(ptr, cnt);
        sprintf(val, "%f", res);
        return;        
    }
}

static void time_now_us(char *now_time, size_t now_time_size) {
    struct timespec ts_now;
    char sec[12];

    // snprintf를 사용하여 버퍼 오버플로우 방지
    snprintf(sec, sizeof(sec), "%lu", (unsigned long)time(NULL)); 

    clock_gettime(CLOCK_REALTIME, &ts_now);
    
    // now_time_size가 충분히 큰지 확인하고, 버퍼 오버플로우를 방지합니다
    if (now_time_size >= (strlen(sec) + 4)) { // sec 문자열 길이 + 3자리 밀리초 + null terminator
        snprintf(now_time, now_time_size, "%s%03ld", sec, ts_now.tv_nsec / 1000000);
        ESP_LOGI(TAG, "The time_now_us: ======> %s", now_time);
    } else {
        ESP_LOGE(TAG, "Buffer size too small");
    }
}

void time_sync_notification_cb(struct timeval *tv)
{
    //ESP_LOGI(TAG, "Notification of a time synchronization event");
}

void initialize_sntp()
{
    if(init_sntp) return;
    // Time init  
      ESP_LOGI(TAG, "Initializing SNTP");
      sntp_setoperatingmode(SNTP_OPMODE_POLL);
      sntp_setservername(0, "pool.ntp.org");
      sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
      sntp_set_time_sync_notification_cb(time_sync_notification_cb);
      sntp_init();
      init_sntp = true;

}


time_t obtain_time(void)
{
    ESP_LOGI(TAG, "[obtain_time] start");

    // Set timezone to Korea Standard Time
    setenv("TZ", "KST-9", 1);
    tzset();

    // Initialize SNTP
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();

    // Wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 100;
    char strftime_buf[64];

    // 이미 시간 동기화가 된 경우를 대비해 초기화
    time(&now);
    localtime_r(&now, &timeinfo);

    while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count)
    {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }

    sntp_stop();

    if (retry == retry_count)
    {
        ESP_LOGI(TAG, "Failed to synchronize time");
        return -1; // Time synchronization failed
    }

    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in South Korea is: ======> %s", strftime_buf);

    return now;
}

/* Function to free context */
static void adder_free_func(void *ctx)
{
    #ifdef DEBUG
    	ESP_LOGI(TAG, "/adder Free Context function called");
    #endif
    free(ctx);
}

/* This handler gets the present value of the accumulator */
static esp_err_t adder_get_handler(httpd_req_t *req)
{
    char resp_str[256];

#ifdef DEBUG
   	ESP_LOGI(TAG, "adder_get_handler-----> GO GO GO !!!");
    ESP_LOGI(TAG,"\n => adder_get_handler DEVICE_MANUFACTURER : %s\n",DEVICE_MANUFACTURER);
    ESP_LOGI(TAG,"\n => adder_get_handler DEVICE_MODEL : %s\n",DEVICE_MODEL);
	ESP_LOGI(TAG,"\n => adder_get_handler device_serial_str : %s\n",DEVICE_ETH_MAC);
    ESP_LOGI(TAG,"\n => adder_get_handler DEVICE_FIRMWAREVERSION : %s\n",DEVICE_FIRMWAREVERSION);
#endif

    sprintf(resp_str,"{\"manufacturer\":\"%s\",\"model\":\"%s\",\"serial\":\"%s\",\"support_type\":\"wifi\"}",DEVICE_MANUFACTURER,DEVICE_MODEL, DEVICE_ETH_MAC);
	//httpd_resp_set_status(req,HTTPD_TYPE_JSON);
	httpd_resp_set_status(req, HTTPD_200);
	httpd_resp_set_type(req, HTTPD_TYPE_JSON);
  	httpd_resp_send(req, resp_str, strlen(resp_str));
  	
    /* Respond with empty body */
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

/* This handler keeps accumulating data that is posted to it into a per
 * socket/session context. And returns the result.
 */
static esp_err_t adder_post_handler(httpd_req_t *req)
{
	//char resp_str[256];
	//const char* resp_str = "200";
    ESP_LOGI(TAG, "adder_post_handler-----> GO GO GO !!!");
	httpd_resp_set_status(req, HTTPD_200);
	httpd_resp_set_type(req, HTTPD_TYPE_JSON);
  	httpd_resp_send(req, "{}", 2);
	vTaskDelay(5); 
		
    char buf[256];
    char test[256];
    char* server_addr;
    char* server_port;
    char* ap_ssid;
    char* ap_password;
    char* auth_key;
    int ret, remaining = req->content_len;
	//printf("data_send : %d \n",remaining);
    while (remaining > 0) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf,
                        MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }

        /* Send back the same data */
        httpd_resp_send_chunk(req, buf, ret);
        remaining -= ret;
		
        /* Log data received */
        #ifdef DEBUG
	        ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
	        ESP_LOGI(TAG, "%.*s", ret, buf);
	        ESP_LOGI(TAG, "====================================");
        #endif
        sprintf(test,"%.*s",ret,buf);
        //printf("test buff is %s\n",test);
                
        cJSON *root = cJSON_Parse(test);
        server_addr = cJSON_GetStringValue(cJSON_GetObjectItem(root,"server_addr"));
        server_port = cJSON_GetStringValue(cJSON_GetObjectItem(root,"server_port"));
        ap_ssid = cJSON_GetStringValue(cJSON_GetObjectItem(root,"ap_ssid"));
    	ap_password = cJSON_GetStringValue(cJSON_GetObjectItem(root,"ap_password"));
    	auth_key = cJSON_GetStringValue(cJSON_GetObjectItem(root,"auth_key"));

        #ifdef DEBUG
	    	printf("server_addr is : %s\n",server_addr);
	    	printf("server_port is : %s\n",server_port);
	    	printf("ap_ssid is : %s\n",ap_ssid);
	    	printf("ap_password is : %s\n",ap_password);
	    	printf("auth_key is : %s\n",auth_key);
        #endif

        flash_info.booting_mode = NORMAL_MODE;
        write_flash(&flash_info);
        
    	printf("NVS Save Starting \n");
    	nvs_open("nvs",NVS_READWRITE,&nvs);
   		nvs_set_str(nvs,"server_addr",server_addr);
   		nvs_set_str(nvs,"server_port",server_port);
   		nvs_set_str(nvs,"ap_ssid",ap_ssid);
   		nvs_set_str(nvs,"ap_password",ap_password);
   		nvs_set_str(nvs,"auth_key",auth_key);
   		nvs_commit(nvs);
    	printf("NVS Save ok \n");
    	nvs_close(nvs);
    	printf("NVS Close ok \n");
    	esp_restart();
    }
    
    return ESP_OK;
}

static esp_err_t adder_set_post_handler(httpd_req_t *req)
{
	//char resp_str[256];
	//const char* resp_str = "200";
	httpd_resp_set_status(req, HTTPD_200);
	httpd_resp_set_type(req, HTTPD_TYPE_JSON);
  	httpd_resp_send(req, "{}", 2);
	vTaskDelay(5); 
		
    char buf[256];
    char test[256];
    char* ap_ssid;
    char* ap_password;

    int ret, remaining = req->content_len;
	//printf("data_send : %d \n",remaining);
    while (remaining > 0) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf,
                        MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }

        /* Send back the same data */
        httpd_resp_send_chunk(req, buf, ret);
        remaining -= ret;
		
        /* Log data received */
        #ifdef DEBUG
	        ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
	        ESP_LOGI(TAG, "%.*s", ret, buf);
	        ESP_LOGI(TAG, "====================================");
        #endif
        sprintf(test,"%.*s",ret,buf);
        //printf("test buff is %s\n",test);
                
        cJSON *root = cJSON_Parse(test);

        ap_ssid = cJSON_GetStringValue(cJSON_GetObjectItem(root,"ap_ssid"));
    	ap_password = cJSON_GetStringValue(cJSON_GetObjectItem(root,"ap_password"));

        #ifdef DEBUG
	    	printf("ap_ssid is : %s\n",ap_ssid);
	    	printf("ap_password is : %s\n",ap_password);
        #endif

        flash_info.booting_mode = NORMAL_MODE;
        write_flash(&flash_info);
        
    	printf("NVS Save Starting \n");
    	nvs_open("nvs",NVS_READWRITE,&nvs);
   		nvs_set_str(nvs,"ap_ssid",ap_ssid);
   		nvs_set_str(nvs,"ap_password",ap_password);

   		nvs_commit(nvs);

    	printf("NVS Save ok \n");
    	nvs_close(nvs);
    	printf("NVS Close ok \n");
    	esp_restart();
    }
    
    return ESP_OK;
}


#ifdef Code_Test32
static esp_err_t adder_post2_handler(httpd_req_t *req)
{

}
#endif
/* Maintain a variable which stores the number of times
 * the "/adder" URI has been visited */
static unsigned visitors = 0;

static const httpd_uri_t adder_get = {
    .uri      = "/api/v1/wifi/info/get",
    .method   = HTTP_GET,
    .handler  = adder_get_handler,
    .user_ctx = NULL
};

static const httpd_uri_t adder_post = {
    .uri      = "/api/v1/wifi/link/start",
    .method   = HTTP_POST,
    .handler  = adder_post_handler,
    .user_ctx = NULL
};

static const httpd_uri_t adder_set_post = {
    .uri      = "/api/v1/wifi/link/set",
    .method   = HTTP_POST,
    .handler  = adder_set_post_handler,
    .user_ctx = NULL
};

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server");
    
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 18080;
	config.max_resp_headers = 20;
    
	// Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);

    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &adder_get);
        httpd_register_uri_handler(server, &adder_post);
        httpd_register_uri_handler(server, &adder_set_post);

        // httpd_register_err_handler(server, HTTPD_400_BAD_REQUEST, adder_err);
        #ifdef adder_post2
        httpd_register_uri_handler(server, &adder_post2);
        #endif
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    char now_time[25];
    httpd_handle_t *server = (httpd_handle_t *) arg;
    
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) 
    {
        ESP_LOGI(TAG, "WIFI_EVENT_AP_STACONNECTED");
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;   // mac[6] : ESP32 soft-AP에 연결된 스테이션의 MAC 주소
                                                                                            // aid : ESP32 soft-AP가 연결된 스테이션에 제공하는 지원
                                                                                            // is_mesh_child : 메쉬 자식을 식별하는 플래그
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",MAC2STR(event->mac), event->aid);
        if (*server == NULL) 
        {
            *server = start_webserver();    
        }
        ap_state = 1;
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) 
    {
        ESP_LOGI(TAG, "WIFI_EVENT_AP_STADISCONNECTED");
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;     // mac[6] : 스테이션의 MAC 주소는 ESP32 소프트 AP에 연결 해제
                                                                                                    // aid : ESP32 soft-AP가 스테이션에 제공한 지원
                                                                                                    // is_mesh_child : 메쉬 자식을 식별하는 플래그
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
	{
        ESP_LOGI(TAG, "WIFI_EVENT_STA_START");
        isForcedWifiStop = false;

        esp_wifi_connect();
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
       	ap_state = 0;
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) 
	{  
        isForcedWifiStop = false;
        //TODO flag stop 후 중간에 타는지. 
        ESP_LOGI(TAG, "##### WIFI_EVENT_STA_CONNECTED");

         // NULL 포인터 확인 추가
        if (ap_ssid_buffer == NULL || ap_ssid_buffer_b == NULL || ap_password_buffer == NULL || ap_password_buffer_b == NULL) {
            ESP_LOGE(TAG, "One or more buffers are NULL");
            return;
        }
    
        if(esp_compare_bufferData() == 1){
            esp_nvs_set_backupData();
        }

        // ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, &server));
        // ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, &event_handler, &server));
    }
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
	{               
        //TODO 
        wifi_event_sta_disconnected_t* event = (wifi_event_sta_disconnected_t*) event_data;

        ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED");
        ESP_LOGI(TAG, "Disconnected reason : %d", event->reason);
        if(isForcedWifiStop) return;

        switch(event->reason){
            case WIFI_REASON_AUTH_EXPIRE:
            case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
            case WIFI_REASON_AUTH_FAIL:
            case WIFI_REASON_ASSOC_EXPIRE:
            case WIFI_REASON_HANDSHAKE_TIMEOUT:
                ESP_LOGI(TAG, "STA Auth Error");
                if(esp_compare_bufferData() == 1 && s_retry_num == 0){

                    isForcedWifiStop = true;
                    s_retry_num++;

                    esp_err_t err = esp_wifi_disconnect();

                    if (err == ESP_OK){
                    ESP_ERROR_CHECK ( esp_wifi_stop ());
                    ESP_ERROR_CHECK ( esp_wifi_deinit ());
                    }

                    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
                    ESP_ERROR_CHECK(esp_wifi_init(&cfg));


                    wifi_config_t wifi_config =  esp_nvs_get_backupData();
                    if (strlen(&ap_ssid_buffer) > 0)
                    {
                        
                        strcpy((char*)wifi_config.sta.ssid, (const char*)ap_ssid_buffer_b);
                        strcpy((char*)wifi_config.sta.password, (const char*)ap_password_buffer_b);
                        printf("ssid : %s \n",(char*)wifi_config.sta.ssid);
                        printf("password : %s \n",(char*)wifi_config.sta.password);
                    }
                    else
                    {

                        wifi_config_t wifi_config = {
                            .sta = {
                                .ssid = EXAMPLE_ESP_WIFI_SSID,
                                .password = EXAMPLE_ESP_WIFI_PASS
                            },
                        };
                    }

                    
                    ESP_LOGI(TAG, "wifi_init_sta finished33333.");
                    isForcedWifiStop = false;

                    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
                    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
                    ESP_ERROR_CHECK(esp_wifi_start() );

                    return;
                }else{
                    isForcedWifiStop = false;
                }
                break;
            case WIFI_REASON_NO_AP_FOUND:
                ESP_LOGI(TAG, "STA AP Not found");
                break;
            default:
                ESP_LOGI(TAG, "STA default Error");

        }
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) 
		{
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the STA");
            if (ip_get_flag)
            {
                ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTTING~~~~~");
                   if (s_retry_num ==( EXAMPLE_ESP_MAXIMUM_RETRY-1)) 
                   {
                         esp_restart();
                   }
            }
        } 
		else 
		{
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            
            printf("Soft AP ON \n");
            printf("Wifi Disconnect \n");
            printf("NVS format \n");
            //timer1_disable();
            
            nvs_flash_init();
            ESP_ERROR_CHECK(nvs_flash_erase());
            
            vTaskDelay(5); 
                
            printf("Soft AP MODE START \n");
            vTaskDelay(5); 
            esp_restart();
        }
        ESP_LOGI(TAG,"connect to the STA fail");
    } 
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
	{        

        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "IP_EVENT_STA_GOT_IP");

         // NULL 포인터 체크 추가
        if(event == NULL) {
            ESP_LOGE(TAG, "event_data is NULL");
             return;
         }

            // 추가적인 NULL 포인터 체크
        if (&event->ip_info.ip == NULL) {
            ESP_LOGE(TAG, "event->ip_info.ip is NULL");
            return;
        }

        initialize_sntp();
        obtain_time();
        time_now_us(now_time, sizeof(now_time));

        
        ESP_LOGI(TAG, "IP_EVENT_STA_GOT_IP");
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        ip_get_flag = 1;
		
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        if (server == NULL) 
        {
            server = start_webserver();           
            printf("Web Server Started ok.\n");
        }
		
    }
}



void esp_nvs_read()
{
	printf("NVS Reading Starting \n");
    nvs_open("nvs",NVS_READWRITE,&nvs);
    size_t len;

	nvs_get_str(nvs, "server_addr", NULL, &len);
	server_addr_buffer = malloc(len);
	nvs_get_str(nvs, "server_addr", server_addr_buffer, &len);    

	nvs_get_str(nvs, "server_port", NULL, &len);
	server_port_buffer = malloc(len);
	nvs_get_str(nvs, "server_port", server_port_buffer, &len);
		
	nvs_get_str(nvs, "ap_ssid", NULL, &len);
	ap_ssid_buffer = malloc(len);
	nvs_get_str(nvs, "ap_ssid", ap_ssid_buffer, &len);
		
	nvs_get_str(nvs, "ap_password", NULL, &len);
	ap_password_buffer = malloc(len);
	nvs_get_str(nvs, "ap_password", ap_password_buffer, &len);
		
	nvs_get_str(nvs, "auth_key", NULL, &len);
	auth_key_buffer = malloc(len);
	nvs_get_str(nvs, "auth_key", auth_key_buffer, &len);

    nvs_get_str(nvs, "ap_ssid_b", NULL, &len);
	ap_ssid_buffer_b = malloc(len);
	nvs_get_str(nvs, "ap_ssid_b", ap_ssid_buffer_b, &len);
		
	nvs_get_str(nvs, "ap_password_b", NULL, &len);
	ap_password_buffer_b = malloc(len);
	nvs_get_str(nvs, "ap_password_b", ap_password_buffer_b, &len);
  
	nvs_close(nvs);

	printf("NVS Reading Ending \n");
}


void wifi_init_softap(void)
{
    s_wifi_event_group = xEventGroupCreate();
    
    ESP_ERROR_CHECK(esp_netif_init()); // 기본 TCP/IP 스택을 초기화합니다.
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // 기본 이벤트 루프를 만듭니다.
     esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap(); // 기본 WIFI AP를 생성 기본 WiFi 액세스 포인트 구성으로 esp_netif 개체를 생성하고 netif를 wifi에 연결하고 기본 Wi-Fi 핸들러를 등록

    ESP_ERROR_CHECK(esp_netif_dhcps_stop(ap_netif)); //DHCP 서버 중지(인터페이스 개체에서 활성화된 경우에만)

    esp_netif_ip_info_t ip_info;
    memset(&ip_info, 0, sizeof(ip_info));   
    IP4_ADDR(&ip_info.ip, 10, 10, 10, 10);
    IP4_ADDR(&ip_info.gw, 10, 10, 10, 1);
    IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
    ESP_ERROR_CHECK(esp_netif_set_ip_info(ap_netif, &ip_info));
    ESP_ERROR_CHECK(esp_netif_dhcps_start(ap_netif));
    esp_efuse_mac_get_default(base_mac_addr);
    sprintf(DEVICE_ETH_MAC, "%02x%02x%02x%02x%02x%02x",base_mac_addr[0], base_mac_addr[1],
             base_mac_addr[2], base_mac_addr[3], base_mac_addr[4], base_mac_addr[5]);

    for(int i = 0;i<12;i++)
    {
        if(DEVICE_ETH_MAC[i]>='a'&& DEVICE_ETH_MAC[i] <='z')
        {
            DEVICE_ETH_MAC[i]= DEVICE_ETH_MAC[i]-32;
        }
    }
	
    #ifdef DEBUG
    ESP_LOGI(TAG, "Base MAC Address ==> wifi_init_softap MAC Address => %02x%02x%02x%02x%02x%02x", base_mac_addr[0], base_mac_addr[1],
             base_mac_addr[2], base_mac_addr[3], base_mac_addr[4], base_mac_addr[5]);   
    ESP_LOGI(TAG, "Device MAC Address =======================>>>>>>>> %s\n",DEVICE_ETH_MAC);
	#endif
    memcpy(flash_info.esp_mac, DEVICE_ETH_MAC, strlen(DEVICE_ETH_MAC));
   
    write_flash(&flash_info);
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, &event_handler, &server));
     
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg)); //WiFi 드라이버에 대한 WiFi 할당 리소스를 초기화합니다. 이 WiFi도 WiFi 작업을 시작

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
	
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) 
	{
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "[wifi_init_softap] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[wifi_init_softap] Minimum_Free memory: %d bytes", esp_get_minimum_free_heap_size());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
}

#define WIFI_MAXIMUM_RETRY 2

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t* event = (wifi_event_sta_disconnected_t*) event_data;
        ESP_LOGI(TAG, "Wi-Fi disconnected, reason: %d", event->reason);
        
        if (s_retry_num < WIFI_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            ESP_LOGI(TAG, "Failed to connect after %d attempts, restarting...", WIFI_MAXIMUM_RETRY);
            esp_restart();
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0; // Reset retry count on successful connection
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    ESP_LOGI(TAG, "wifi_init_sta starting...");

    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    //  ESP_ERROR_CHECK(esp_event_loop_create_default());

    // 이벤트 루프 생성 전에 이미 생성되었는지 확인
    esp_err_t err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_ERROR_CHECK(err);
    } else if (err == ESP_ERR_INVALID_STATE) {
        ESP_LOGW(TAG, "Event loop already created");
    }

    // 기존에 생성된 기본 Wi-Fi STA 인터페이스가 있는 경우 삭제
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif != NULL) {
        ESP_LOGI(TAG, "Destroying existing Wi-Fi STA interface");
        esp_netif_destroy(netif);  // void 함수이므로 반환값을 무시하고 호출만 합니다.
    }
    // sta_netif를 초기화
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();  

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_wifi_get_mac(ESP_IF_WIFI_STA, eth_mac);
    ESP_LOGI(TAG, "wifi_init_sta MAC Address => %02x%02x%02x%02x%02x%02x", eth_mac[0], eth_mac[1], eth_mac[2], eth_mac[3], eth_mac[4], eth_mac[5]);

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip));

	wifi_config_t wifi_config = {0};

    // NVS 데이터 읽기 추가
    esp_nvs_read(); // Read NVS data

	if (strlen(ap_ssid_buffer) > 0)
	{
		
		strcpy((char*)wifi_config.sta.ssid, (const char*)ap_ssid_buffer);
		strcpy((char*)wifi_config.sta.password, (const char*)ap_password_buffer);
		printf("ssid : %s \n",(char*)wifi_config.sta.ssid);
    	printf("password : %s \n",(char*)wifi_config.sta.password);
	}
    else
	{

        strcpy((char*)wifi_config.sta.ssid, EXAMPLE_ESP_WIFI_SSID);
        strcpy((char*)wifi_config.sta.password, EXAMPLE_ESP_WIFI_PASS);
    }

/*
	    wifi_config_t wifi_config = {
	        .sta = {
	            .ssid = EXAMPLE_ESP_WIFI_SSID,
	            .password = EXAMPLE_ESP_WIFI_PASS
	        },
	    };
    }
        */
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished11111.");

    // Wi-Fi 연결 대기
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", wifi_config.sta.ssid, wifi_config.sta.password);

        // 연결 성공 후에 DNS 정보 가져오기
        esp_netif_dns_info_t dns;
        if (esp_netif_get_dns_info(sta_netif, ESP_NETIF_DNS_MAIN, &dns) == ESP_OK) {
            ESP_LOGI(TAG, "DNS: " IPSTR, IP2STR(&dns.ip.u_addr.ip4));
        } else {
            ESP_LOGE(TAG, "Failed to get DNS info");
        }

    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", wifi_config.sta.ssid, wifi_config.sta.password);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));

    // 이벤트 그룹 삭제
    vEventGroupDelete(s_wifi_event_group);

    ESP_LOGI(TAG, "[wifi_init_sta] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[wifi_init_sta] Minimum_Free memory: %d bytes", esp_get_minimum_free_heap_size());

    ESP_LOGI(TAG, "[wifi_init_sta] init sntp");
    // initialize_sntp();
    // obtain_time();
    // time_now_us(now_time, sizeof(now_time));
}

/*
void deinit_softap() {
    if (ap_netif != NULL) { // AP 네트워크 인터페이스가 존재하는 경우에만 해제
        esp_netif_destroy(ap_netif);
        ap_netif = NULL; // 포인터를 NULL로 초기화하여 중복 해제를 방지
    }
}

void handle_data(const char *data) {
    if (strcmp(data, "a") == 0) {
        init_softap();
    } else {
        deinit_softap(); // 다른 경우 필요 시 AP를 해제할 수 있음
    }
}
*/

void UART_Command()
{
    char now_time[25];
	uint8_t *data = (uint8_t *) malloc(RD_BUF_SIZE);
	NotifyParameter firmwareNotifyParam[2];
	//char* ti;
	//int i = 0;
	bzero(data, RD_BUF_SIZE);
	int len = uart_read_bytes(UART_NUM_0, data, 2, 20 / portTICK_RATE_MS);
	//Write data back to UART
	uart_write_bytes(UART_NUM_0, (const char*) data, len);
	
	
  if(strcmp((const char *) data,"a") == 0)
	{
		printf(": Soft AP ON\r\n");
		if(damda_IsMQTTConnected() == 1)damdaMQTTDisConnect();
        ESP_ERROR_CHECK(esp_netif_init());
        //ESP_ERROR_CHECK(esp_event_loop_create_default());

        if (ap_netif == NULL) { // AP 네트워크 인터페이스가 생성되지 않은 경우에만 생성
        ap_netif = esp_netif_create_default_wifi_ap();
        ESP_ERROR_CHECK(esp_netif_dhcps_stop(ap_netif));
        
        esp_netif_ip_info_t ip_info;
        memset(&ip_info, 0, sizeof(ip_info));   
        IP4_ADDR(&ip_info.ip, 10, 10, 10, 10);
        IP4_ADDR(&ip_info.gw, 10, 10, 10, 1);
        IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
        ESP_ERROR_CHECK(esp_netif_set_ip_info(ap_netif, &ip_info));
        ESP_ERROR_CHECK(esp_netif_dhcps_start(ap_netif));
        
        }

        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, &server));
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, &event_handler, &server));
         
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        
        wifi_config_t wifi_config = {
            .ap = {
                .ssid = EXAMPLE_ESP_WIFI_SSID,
                .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
                .channel = EXAMPLE_ESP_WIFI_CHANNEL,
                .password = EXAMPLE_ESP_WIFI_PASS,
                .max_connection = EXAMPLE_MAX_STA_CONN,
                .authmode = WIFI_AUTH_WPA_WPA2_PSK
            },
        };
        
        if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) 
        {
            wifi_config.ap.authmode = WIFI_AUTH_OPEN;
        }
        
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());
        
        ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);

		
	}
	else if(strcmp((const char *) data,"b") == 0){
		printf(": Soft AP OFF\r\n");

		if(damda_IsMQTTConnected() == 1)
        damdaMQTTDisConnect();	

        // Soft AP 모드 관련 처리
        if (ap_state == 1) {
        // Soft AP 모드가 활성화 중인 경우, STA 모드로 전환
        esp_err_t ret = ESP_OK;

    // 이벤트 그룹 생성
        if (s_wifi_event_group == NULL) {
            s_wifi_event_group = xEventGroupCreate();
            if (s_wifi_event_group == NULL) {
                ESP_LOGE(TAG, "Failed to create WiFi event group");
                return;
            }
        }

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ret = esp_wifi_init(&cfg);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize WiFi: %d", ret);
            return;
        }

    //    s_wifi_event_group = xEventGroupCreate();     


        ESP_ERROR_CHECK(esp_netif_init());        
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        esp_netif_create_default_wifi_sta();
        
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        
    
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, &server));

	wifi_config_t wifi_config = {0};
	if (strlen(&ap_ssid_buffer) > 0)
	{
		
		strcpy((char*)wifi_config.sta.ssid, (const char*)ap_ssid_buffer);
		strcpy((char*)wifi_config.sta.password, (const char*)ap_password_buffer);
		printf("ssid : %s \n",(char*)wifi_config.sta.ssid);
    	printf("password : %s \n",(char*)wifi_config.sta.password);
	}
    else
	{

	    wifi_config_t wifi_config = {
	        .sta = {
	            .ssid = EXAMPLE_ESP_WIFI_SSID,
	            .password = EXAMPLE_ESP_WIFI_PASS
	        },
	    };
    }
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
        ESP_ERROR_CHECK(esp_wifi_start() );
        
        ESP_LOGI(TAG, "wifi_init_sta finished22222.");
        
        /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
         * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                pdFALSE,
                pdFALSE,
                portMAX_DELAY);
        
        /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
         * happened. */
        if (bits & WIFI_CONNECTED_BIT) {
            ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                     EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
        } else if (bits & WIFI_FAIL_BIT) {
            ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                     EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
        } else {
            ESP_LOGE(TAG, "UNEXPECTED EVENT");
        }
        
#ifdef Code_Test32

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
#else
    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));

#endif    
    vEventGroupDelete(s_wifi_event_group);		
    } else {
        printf("WiFi STA mode is already active\n");
    }
		
	}

	else if(strcmp((const char *) data,"c") == 0){
        if(ap_state == 0){

			if(damda_IsMQTTConnected() == 1){
            damdaMQTTDisConnect();
        }
			
			printf(": Mqtt connect11 \r\n");
			
			// damdaSetDeviceInfo(DEVICE_MANUFACTURER,DEVICE_MODEL,DEVICE_ETH_MAC,flash_info.ver_string,DEVICE_TYPE);

			//damdaMQTTConnect((const char*)server_addr_buffer, atoi(server_port_buffer),(const char*)auth_key_buffer);

            if(damdaMQTTConnect((const char*) server_addr_buffer, atoi(server_port_buffer), (const char*) auth_key_buffer) != 1) {
            ESP_LOGE(TAG, "MQTT client initialization failed");
            return;  // MQTT 연결 실패 시 함수 종료
            
        }

            printf("MQTT22 : %d",damda_IsMQTTConnected());

            obtain_time();
			test_init();

            printf("Mqtt Callback maked \n");
		}
		else
		{
			printf("wifi not connected \n");
		}
		printf("MQTT : %d",damda_IsMQTTConnected());
        
        ESP_LOGI(TAG, "[damdaMQTTDisConnect] Free memory: %d bytes", esp_get_free_heap_size());
	
	}
	else if(strcmp((const char *) data,"d") == 0)
	{
		printf(": Register Device \r\n");
		damda_Req_RegisterDevice();
	}
    else if(strcmp((const char *) data,"e") == 0)
    {
		printf(": Update Device Information\r\n");
		damda_Req_UpdateDeviceInfo();
	}
	else if(strcmp((const char *) data,"f") == 0)
    {
		printf(": Timesync ON\r\n");
		damda_Req_ServerTimeInfo();
	}
	else if(strcmp((const char *) data,"g") == 0)
    {
		printf(": Report periodic information(notify)\r\n");
		//damdaMQTTDisConnect();
		ucnotify_flag = 1;
	}
	else if(strcmp((const char *) data,"h") == 0)
    {
		printf(": Report periodic information(notify) STOP\r\n");
		ucnotify_flag = 0;
	}
	else if(strcmp((const char *) data,"i") == 0)
	{
		printf(": Report event notification(notify)\r\n");
        ESP_LOGI(TAG, "[Report event notification] Free memory: %d bytes", esp_get_free_heap_size());
		
		obtain_time();
        time_now_us(now_time, sizeof(now_time));
	}
	
	else if(strcmp((const char *) data,"j") == 0){
		printf(": Unregister Device\r\n");
		// damda_Req_DeregisterDevice();
        // it makes db error
	}
	else if(strcmp((const char *) data,"k") == 0){
		printf(": Firmware Download init\r\n");
/*
        const char* ssid = "damda.dev_2.4";
        const char* password = "iot1q2w3e4r@";

         // ap_ssid_buffer 및 ap_password_buffer에 메모리 할당
    if (ap_ssid_buffer == NULL) {
        ap_ssid_buffer = (char*)malloc(strlen(ssid) + 1);
        if (ap_ssid_buffer == NULL) {
            ESP_LOGE(TAG, "Failed to allocate memory for ap_ssid_buffer");
            return;
        }
    }

    if (ap_password_buffer == NULL) {
        ap_password_buffer = (char*)malloc(strlen(password) + 1);
        if (ap_password_buffer == NULL) {
            ESP_LOGE(TAG, "Failed to allocate memory for ap_password_buffer");
            free(ap_ssid_buffer);  // 할당된 메모리 해제
            return;
        }
    }
    printf("11111 확인");
    // Wi-Fi 연결 시도
        strcpy((char*)ap_ssid_buffer, ssid);
        strcpy((char*)ap_password_buffer, password);

        wifi_init_sta();
    
    printf("22222 확인");
    // WiFi 연결 대기
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT,
            pdFALSE,
            pdTRUE,
            portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        printf("WiFi connected\n");

            printf("33333 확인");
      if (ap_state == 0) {
        if (damda_IsMQTTConnected() != 1) {
            printf(": Mqtt connect\r\n");

             // 임시로 값을 설정
            //const char* server_addr = "dev-iot.godamda.kr";
            const char* server_addr = "iot.godamda.kr";
            const char* server_port = "11884";
            //운영
            const char* auth_key = "6ff0b8bb-9a0e-4917-b94b-e767f87a61d0";
            //개발
            //const char* auth_key = "5cf3d1ba-20f1-4b1b-97aa-5faf135f33dd";
            
            // 값 설정
            server_addr_buffer = server_addr;
            server_port_buffer = server_port;
            auth_key_buffer = auth_key;
            
            // NULL 포인터 확인
            if (server_addr_buffer == NULL || server_port_buffer == NULL || auth_key_buffer == NULL) {
                ESP_LOGE(TAG, "MQTT parameters are NULL");
                return;
            }

            // 포인터 내용 출력 (디버깅 목적)
            printf("Server address: %s\n", server_addr_buffer);
            printf("Server port: %s\n", server_port_buffer);
            printf("Auth key: %s\n", auth_key_buffer);

            // MQTT 연결 시도
                int mqtt_connect_attempts = 0;
                while (mqtt_connect_attempts < 5) {
                    if (damdaMQTTConnect((const char*) server_addr_buffer, atoi(server_port_buffer), (const char*) auth_key_buffer) == 1) {
                        printf("MQTT connected successfully\n");
                        break;
                    } else {
                        ESP_LOGE(TAG, "MQTT client initialization failed, attempt %d", mqtt_connect_attempts + 1);
                        mqtt_connect_attempts++;
                        vTaskDelay(5000 / portTICK_PERIOD_MS); // 5초 대기 후 재시도
                    }
                }
                
                if (mqtt_connect_attempts == 5) {
                    ESP_LOGE(TAG, "MQTT connection failed after multiple attempts");
                    return;
                }
            }



	//	obtain_time();

        if (obtain_time() != ESP_OK) {
            ESP_LOGE(TAG, "Failed to obtain time");
            return;
        }
   */     
        obtain_time();
        time_now_us(now_time, sizeof(now_time));

		damda_Req_UpdateDeviceInfo();
		firmwareNotifyParam[0].resourceUri = "/5/0/3";
		firmwareNotifyParam[0].stringValue = "0";
		firmwareNotifyParam[0].time = now_time;
		firmwareNotifyParam[1].resourceUri = "/5/0/5";
		firmwareNotifyParam[1].stringValue = "0";
		firmwareNotifyParam[1].time = now_time;
		damda_Notify_DeviceStatus("2",firmwareNotifyParam,2);// /5/0/3 /5/0/5 and string Value 0,0 send (download init)

		firmwareNotifyParam[0].resourceUri = "/5/0/3";
		firmwareNotifyParam[0].stringValue="1";
		firmwareNotifyParam[0].time = now_time;
		damda_Notify_DeviceStatus("2",firmwareNotifyParam,1);// /5/0/3 and string Value 1 send (downloading)
        printf("44444 확인");
         
        //test_init();
/*
         }  else {
        printf("wifi not connected\r\n");
          }
     } else {
        printf("Failed to connect to WiFi\n");
    } */
}
	else if(strcmp((const char *) data,"l") == 0){
		printf(": Firmware Download\r\n");
        obtain_time();
        time_now_us(now_time, sizeof(now_time));
        firmwareNotifyParam[0].resourceUri = "/5/0/3";
        firmwareNotifyParam[0].stringValue="2";
		firmwareNotifyParam[0].time = now_time;
		damda_Notify_DeviceStatus("2",firmwareNotifyParam,1);// /5/0/3 and string Value 1 send (downloading)
	}
	else if(strcmp((const char *) data,"m") == 0){
		printf(": Firmware info update\r\n");
		        obtain_time();
        time_now_us(now_time, sizeof(now_time));
	    firmwareNotifyParam[0].resourceUri = "/5/0/3";
        firmwareNotifyParam[0].stringValue="3";
        firmwareNotifyParam[0].time = now_time;
        damda_Notify_DeviceStatus("2",firmwareNotifyParam,1);// /5/0/3 and string Value 3 send (Updating)
	}
	else if(strcmp((const char *) data,"n") == 0){
		printf(": Firmware update\r\n");
		        obtain_time();
        time_now_us(now_time, sizeof(now_time));
        firmwareNotifyParam[0].resourceUri = "/5/0/3";
        firmwareNotifyParam[0].stringValue="0";
        firmwareNotifyParam[0].time = now_time;
		firmwareNotifyParam[1].resourceUri = "/5/0/5";
		firmwareNotifyParam[1].stringValue="1";
		firmwareNotifyParam[1].time = now_time;
		damda_Notify_DeviceStatus("2",firmwareNotifyParam,2);// /5/0/5 and string Value 1 send (Successing)
		read_flash(&flash_info);
        strcpy(DEVICE_FIRMWAREVERSION,flash_info.ver_string);
	}


    else if(strcmp((const char *) data,"o") == 0)
	{
		printf(": EEPROM format\r\n");
		
		printf("Soft AP ON \n");
		printf("Wifi Disconnect \n");
		printf("NVS format \n");
		//timer1_disable();
		
		nvs_flash_init();
		ESP_ERROR_CHECK(nvs_flash_erase());
		
		vTaskDelay(5); 
			
		printf("Soft AP MODE START \n");
		vTaskDelay(5); 
 		esp_restart();
	}
    else if(strcmp((const char *) data,"q") == 0)  // Delete device 
    {
		printf(": Delete registered device\r\n");
		damda_Req_DeleteDevice();
	}
    else if(strcmp((const char *) data,"x") == 0){
        printf(": notification\r\n");
        NotifyParameter notifyParams[10];
        
                obtain_time();
        time_now_us(now_time, sizeof(now_time));
    
        notifyParams[0].resourceUri = "/3406/1/3419";
        notifyParams[0].stringValue="3";
        notifyParams[0].time = now_time;
        notifyParams[1].resourceUri = "/3406/1/4627";
        notifyParams[1].stringValue="1";
        notifyParams[1].time = now_time;
       notifyParams[2].resourceUri = "/1202/1/5200";
        notifyParams[2].stringValue="OFF";
        notifyParams[2].time = now_time;
        damda_Notify_DeviceStatus("2",notifyParams,3);// /5/0/3 and string Value 3 send (Updating)
    }
    else if(strcmp((const char *) data,"y") == 0){
            printf(": notification\r\n");
            NotifyParameter notifyParams[10];
            
                    obtain_time();
            time_now_us(now_time, sizeof(now_time));

            notifyParams[0].resourceUri = "/3406/1/3419";
            notifyParams[0].stringValue="6";
            notifyParams[0].time = now_time;
            notifyParams[1].resourceUri = "/3406/1/4627";
            notifyParams[1].stringValue="3";
            notifyParams[1].time = now_time;
           notifyParams[2].resourceUri = "/1202/1/5200";
            notifyParams[2].stringValue="ON";
            notifyParams[2].time = now_time;
            damda_Notify_DeviceStatus("2",notifyParams,3);// /5/0/3 and string Value 3 send (Updating)
        }
	if(strcmp((const char *) data,"\r") == 0){
		message_print_flag = 1;
		printf("Command : NULL\r\n");
	}
	
	free(data);
}

void Test_Command()
{
	if(message_print_flag == 1)
	{	
		printf("\r\n");
   		printf("\r\n");
		printf("a : Soft AP ON\r\n");
		printf("b : Soft AP OFF\r\n");
		printf("c : Mqtt connect\r\n");
		printf("d : Register Device\r\n");
		printf("e : Update Device Information\r\n");
		printf("f : Timesync ON\r\n");
		printf("g : Report periodic information(notify)\r\n");
		printf("h : Report periodic information(notify) STOP\r\n");
		printf("i : Report event notification(notify)\r\n");
		// printf("j : Unregister Device\r\n"); it makes db error.
        printf("k : Firmware update\r\n");
		//printf("k : Firmware Download init\r\n");
		//printf("l : Firmware Download\r\n");
		//printf("m : Firmware info update\r\n");
		//printf("n : Firmware update\r\n");
        printf("o : EEPROM format\r\n");
        printf("p : SDK Firmware Version : %s\n",DEVICE_FIRMWAREVERSION);
        printf("q : Delete Device - don't use this menu as possible \r\n");
		
		message_print_flag = 0;
	}
}

void customMQTTMsgReceive(ResponseMessageInfo* responseInfo)
{
    char now_time[25];
	char receivedResourceUri[15];
	char receivedStringvalue[15];

    print_receiveData(responseInfo);

    #ifdef DEBUG
		printf("custom Callback sid is %s\n",responseInfo->sid);
		printf("custom Callback returnCode is %d\n",responseInfo->returnCode);
		printf("custom Callback errorCode is %d\n",responseInfo->errorCode);
		printf("custom Callback responseType is %d\n",responseInfo->responseType);
		printf("custom Callback responseArraySize is %d\n",responseInfo->responseArraySize);
	
		//printf("custom Callback FirmwareDownloadUrl is %s\n",responseInfo->resourceUri[0]);
		//printf("custom Callback FirmwareDownloadUrl is %s\n",responseInfo->stringValue[0]);
	
		printf("custom Callback sequence is %d\n",responseInfo->sequence);
		//printf("custom Callback FirmwareVersion is %s\n",responseInfo->firmwareVersion);
		//printf("custom Callback FirmwareDownloadUrl is %s\n",responseInfo->firmwareDownloadUrl);
	#endif
    
	if(responseInfo->responseType == MESSAGE_TYPE_READ || responseInfo->responseType == MESSAGE_TYPE_WRITE || responseInfo->responseType == MESSAGE_TYPE_EXECUTE){
		strcpy(receivedResourceUri,responseInfo->resourceUri[0]);
		strcpy(receivedStringvalue,responseInfo->stringValue[0]);
        printf("busy_flag is %d\n",busy_flag);
        if (busy_flag )
        {
         	//damda_Rsp_RemoteControl(responseInfo->responseType,responseInfo->sid,"200",responseInfo->resourceUri,responseInfo->stringValue,responseInfo->responseArraySize);
       	   printf("\ndamda_Rsp_RemoteControl =========> damda_Rsp_Client_Busy function \n");       
           damda_Rsp_Client_Busy(responseInfo->responseType,responseInfo->sid,"600",responseInfo->responseArraySize);
                            
        }
                  busy_flag  = true;

    	obtain_time();
      	time_now_us(now_time, sizeof(now_time));
      	time_execute = now_time;

	}

    
    
	if(responseInfo->responseType == MESSAGE_TYPE_FIRMWARE_DOWNLOAD){

        printf("MESSAGE_TYPE_FIRMWARE_DOWNLOAD !!!!!!!!!!!!\n");
		strcpy(ota_version,responseInfo->firmwareVersion);
		strcpy(ota_url,responseInfo->firmwareDownloadUrl);
		
        ota_task_init(); 
    }

   if ((responseInfo->returnCode == 404 )&& (responseInfo->errorCode == 40443))
   {
	   printf("Not Found Device \n");
	   printf("404 && 40443 error Code \n");
	   printf("Wifi Disconnect \n");
	   printf("NVS format \n");
       
	   nvs_flash_init();
	   ESP_ERROR_CHECK(nvs_flash_erase());
		
	   vTaskDelay(5); 
			
	   printf("Soft AP MODE START \n");
	   vTaskDelay(5); 
	   esp_restart();
   }
	if(responseInfo->responseType == MESSAGE_TYPE_DELETE_SYNC || responseInfo->responseType == MESSAGE_TYPE_DELETE_ASYNC){
			
		printf("Soft AP ON \n");
		printf("Wifi Disconnect \n");
		printf("NVS format \n");
		//timer1_disable();
		
		nvs_flash_init();
		ESP_ERROR_CHECK(nvs_flash_erase());
		
		vTaskDelay(5); 
			
		printf("Soft AP MODE START \n");
		vTaskDelay(5); 
		esp_restart();
	}
	
	message_print_flag = 1;
	
}

void test_init()
{
    printf("test_init START \n");
	damda_setMQTTMsgReceiveCallback(customMQTTMsgReceive);
}


int esp_compare_bufferData() {

    ESP_LOGI(TAG,"Compare bufferData Start\n"); 

    if (ap_ssid_buffer == NULL || ap_ssid_buffer_b == NULL || ap_password_buffer == NULL || ap_password_buffer_b == NULL) {
        ESP_LOGE(TAG, "One or more buffers are NULL");
        return -1;  // 오류 코드 반환
    }

    if((strcmp(ap_ssid_buffer, ap_ssid_buffer_b) == 0 )&& (strcmp(ap_password_buffer, ap_password_buffer_b) == 0) ) {
        return 0;
    }else{
        ESP_LOGI(TAG,"Compared values are different\n"); 
        return 1;
    }
}

//wifi 커넥트 성공 시 호출 될 함수 (nvs에 성공한 ssid,pw를 backup데이터와 비교 후 backup데이터 저장용.
void esp_nvs_set_backupData()
{
        ESP_LOGI(TAG,"NVS Backup Data Setting Start\n"); 
        ap_ssid_buffer_b = realloc(ap_ssid_buffer_b,strlen(ap_ssid_buffer)+1);
        strcpy(ap_ssid_buffer_b, ap_ssid_buffer);

        ap_password_buffer_b = realloc(ap_password_buffer_b,strlen(ap_password_buffer)+1);
        strcpy(ap_password_buffer_b,ap_password_buffer);

        ESP_LOGI(TAG," => ap_ssid_buffer : %s\n",ap_ssid_buffer);
        ESP_LOGI(TAG," => ap_ssid_buffer_b : %s\n",ap_ssid_buffer_b);
        ESP_LOGI(TAG," => ap_password_buffer : %s\n",ap_password_buffer);
        ESP_LOGI(TAG," => ap_password_buffer_b : %s\n",ap_password_buffer_b); 

        
        nvs_open("nvs",NVS_READWRITE,&nvs);
        nvs_set_str(nvs,"ap_ssid_b",ap_ssid_buffer_b);
        nvs_set_str(nvs,"ap_password_b",ap_password_buffer_b);
        nvs_commit(nvs);
        nvs_close(nvs);
        
        ESP_LOGI(TAG,"NVS Backup Data Setting Done\n");  
}

wifi_config_t esp_nvs_get_backupData()
{
    ESP_LOGI(TAG,"Get backup data from NVS\n");  

    wifi_config_t wifi_config = {0};
    if(strlen(ap_ssid_buffer_b) > 0 && strlen(ap_password_buffer_b) > 0){
        ap_ssid_buffer = realloc(ap_ssid_buffer,strlen(ap_ssid_buffer_b) +1);
        ap_password_buffer = realloc(ap_password_buffer,strlen(ap_password_buffer_b) +1);

        strcpy(ap_ssid_buffer, ap_ssid_buffer_b);
        strcpy(ap_password_buffer, ap_password_buffer_b);

        nvs_open("nvs",NVS_READWRITE,&nvs);
            nvs_set_str(nvs,"ap_ssid",ap_ssid_buffer);
            nvs_set_str(nvs,"ap_password",ap_password_buffer);
            nvs_commit(nvs);
        nvs_close(nvs);
    }else{
        nvs_open("nvs",NVS_READWRITE,&nvs);
        
        char * ap_ssid_b = esp_nvs_load_value_if_exist(nvs,"ap_ssid_b");
        char * ap_password_b = esp_nvs_load_value_if_exist(nvs, "ap_password_b");
        
        if(ap_ssid_b != NULL && ap_password_b != NULL) {

            strcpy(ap_ssid_buffer_b, ap_ssid_b);
            strcpy(ap_password_buffer_b,ap_password_b);
            strcpy(ap_ssid_buffer, ap_ssid_b);
            strcpy(ap_password_buffer,ap_password_b);


            nvs_set_str(nvs,"ap_ssid",ap_ssid_buffer);
            nvs_set_str(nvs,"ap_password",ap_password_buffer);
            nvs_commit(nvs);

            strcpy((char*)wifi_config.sta.ssid, (const char*)ap_ssid_buffer_b);
            strcpy((char*)wifi_config.sta.password,(const char*)ap_password_buffer_b);
        }
        nvs_close(nvs);
    }
    
    return wifi_config;
}
char * esp_nvs_load_value_if_exist(nvs_handle_t nvs, const char* key) { 

    ESP_LOGI(TAG,"NVS Load %s Value Starting\n",key);

    // Try to get the size of the item
    size_t len;
    if(nvs_get_str(nvs, key, NULL, &len) != ESP_OK){
        ESP_LOGI(TAG, "Failed to get size of key: %s", key);
        return NULL;
    }

    char* value = malloc(len);
    if(nvs_get_str(nvs, key, value, &len) != ESP_OK){
        ESP_LOGI(TAG, "Failed to load key: %s", key);
        return NULL;
    }
    ESP_LOGI(TAG, "NVS Load %s Value Ending", key);
    return value;
}

void OTA_upgrade_fail(void)
{
    char now_time[25];
	NotifyParameter firmwareNotifyParam[2];
            obtain_time();
    time_now_us(now_time, sizeof(now_time));
    firmwareNotifyParam[0].resourceUri = "/5/0/5";
    firmwareNotifyParam[0].stringValue="8";
    firmwareNotifyParam[0].time = now_time;
    damda_Notify_DeviceStatus("2",firmwareNotifyParam,1);// /5/0/5 and string Value 1 send (Successing)

}
void Notify_Func()
{
    char now_time[25];
    if(ucnotify_flag == 1 && damda_IsMQTTConnected() == 1)
    {
        NotifyParameter notifyParams[100];
        #ifdef DEBUG
            printf("app_main : \"Notify_Func\" is called \r\n");
        #endif
        obtain_time();
        time_now_us(now_time, sizeof(now_time));

        // --- Sending device resources data as notification ---
        // --- Need to modify below code if the device profile changed ---  
        char* resUri[11] = {POWER, TEMPERATURE, TEMPERATURESETTINGS, TEMPERATURESETTINGSMODE, SALESTARGET, LOCKSTATE, TEMPERUNIT, OPENSTATE, ERRORCODE, ALARMSTATUS,VERSION};
        char* strVal[11] = {power_value_0, temperature_value_0, temperaturesetting_value_0, temperaturesettingmode_value_0, salestarget_value_0, lockstate_value_0, temperunit_value_0, openstate_value_0, errorcode_value_0, alarmstaus_value_0, version_value_0};    
        for( int noti_index=0;noti_index<11;noti_index++)
		{
                notifyParams[noti_index].resourceUri = resUri[noti_index];
                notifyParams[noti_index].stringValue = strVal[noti_index];
            	notifyParams[noti_index].time = now_time;
    	}
    	damda_Notify_DeviceStatus("2", notifyParams, 11);


/*

#ifdef PN_manufacturer
         NotifyParameter notifyParams[100];
         char* resUri[4] = {POWER,STANDBY_CUTOFF_SETTING,STANDBY_CUTOFF,OVERHEAT_CUTOFF};
         char* strVal[4] = {power_value1,standby_cutoff_setting_value1,standby_cutoff_value1,overheat_cutoff_value1};
         for( int noti_index=0;noti_index<4;noti_index++)
		 {
                notifyParams[noti_index].resourceUri = resUri[noti_index];
                notifyParams[noti_index].stringValue = strVal[noti_index];
            	notifyParams[noti_index].time = now_time;
    	 }
    	damda_Notify_DeviceStatus("2", notifyParams, 4);
 #endif
 
#ifdef DIMCHAE_manufacturer
     NotifyParameter notifyParams[100];
     char* resUri[42] = {ROOM_OPERATION,
                                                MODE_MAINMODES,MODE_SUBMODES,MODE_REMAININGTIME,MODE_NOTIFY,MODE_COMPLETE,
                                                DOOR_OPENSTATE,
                                                COMPRESSOR_ONLEVEL,COMPRESSOR_OFFLEVEL,COMPRESSOR_REFRESHTIMER,COMPRESSOR_LEAKCOUNT,COMPRESSOR_STEPVALVEPOSITION,COMPRESSOR_COUNT,COMPRESSOR_STATUS ,
                                                HEATER_ONLEVEL,HEATER_OFFLEVEL,HEATER_TEMPERATURELEVEL, HEATER_DRAIN , HEATER_DEFROST,
                                                TEMPERATURE_TEMPERATURE, TEMPERATURE_CHANGESTEP, 
                                                FAN_OUTRPM, FAN_FEEDBACKRPM,
                                                LAMP_VALUE,
                                                OPERATIONSTATE_MACHINESTATES,OPERATIONSTATE_CURRENTMACHINESTATES,
                                                MOVEMENT_AUTOLIFTSTATUS, 
                                                HUMIDITYLEVEL,
                                                MOTION_SENSOR,
                                                ENERGYCONSUMPTION,
                                                ERROR_CODE_0, ERROR_CODE_1, ERROR_CODE_2,ERROR_CODE_3,ERROR_CODE_4,ERROR_CODE_5, 
                                                ERROR_CODE_6, ERROR_CODE_7, ERROR_CODE_8,ERROR_CODE_9,ERROR_CODE_10,ERROR_CODE_11};

     char* strVal[42] = {room_operation_value,
                                                mode_mainmodes_value,mode_submodes_value,mode_remainingtime_value,mode_notify_value,mode_complete_value,
                                                door_openstate_value,
                                                compressor_onlevel_value, compressor_offlevel_value,compressor_refreshtimer_value, compressor_leakcount_value,compressor_stepvalyepositon_value,compressor_count_value,compressor_status_value,
                                                heater_onlevel_vaule,heater_offlevel_value,heater_temperaturelevel_value,heater_drain_value, heater_defrost_value,
                                                temperature_temperature_value,temperature_changestep_value ,
                                                fan_outrpm_value,fan_feedbackrpm_value, 
                                                lamp_value1 ,
                                                operationstate_machinestates_value, operationstate_currentmachinestates_value,
                                                movement_autoliftstatus_value, 
                                                humiditylevel_value, 
                                                motion_sensor_value, 
                                                energyconsumption_value,
                                                error_code_0_value,error_code_1_value, error_code_2_value, error_code_3_value,error_code_4_value,error_code_5_value,
                                                error_code_6_value,error_code_7_value,error_code_8_value, error_code_9_value, error_code_10_value,error_code_11_value};
     
     for( int noti_index=0;noti_index<42;noti_index++)
     {
            notifyParams[noti_index].resourceUri = resUri[noti_index];
            notifyParams[noti_index].stringValue = strVal[noti_index];
            notifyParams[noti_index].time = now_time;
     }
    damda_Notify_DeviceStatus("2", notifyParams, 42);

#endif

#ifdef DUMMY_DATA
        printf("@@@@@ noti\n");
        cJSON *jsonRscData = cJSON_Parse(deviceRscData);
        int rscDataCnt = cJSON_GetArraySize(jsonRscData);
        int noti_cnt = 0;
        int noti_index=0;
        
        NotifyParameter notifyParams[rscDataCnt];
        char strUri[rscDataCnt][20];
        char strVal[rscDataCnt][30];
        int instanceCnt = 0;

        for( int jArr_index=0;jArr_index<rscDataCnt;jArr_index++)
        {
            cJSON * subitem = cJSON_GetArrayItem(jsonRscData, jArr_index);
            char * jsonOid = cJSON_GetStringValue(cJSON_GetObjectItem(subitem, RSC_OID));
            char * jsonRid = cJSON_GetStringValue(cJSON_GetObjectItem(subitem, RSC_RID));
            char * jsonCount = cJSON_GetStringValue(cJSON_GetObjectItem(subitem, RSC_COUNT));
            char * jsonType = cJSON_GetStringValue(cJSON_GetObjectItem(subitem, RSC_TYPE));
            char * jsonRange = cJSON_GetStringValue(cJSON_GetObjectItem(subitem, RSC_RANGE));


            instanceCnt = atoi(jsonCount);
            for(int i=0; i < instanceCnt; i++){
                char bufStr[30];
                
                ESP_LOGI(TAG, "type %s", jsonType);
                //sprintf(strUri[noti_index], "/%s/%s/%s", jsonOid, itoa(i, bufStr, 10), jsonRid);
                sprintf(strUri[noti_index], "/%s/0/%s", jsonOid, jsonRid);
                notifyParams[noti_index].resourceUri = strUri[noti_index];
                printf("noti uri : %s\n", notifyParams[noti_index].resourceUri);

                getRscRandomValue(jsonType, jsonRange, strVal[noti_index]);
                notifyParams[noti_index].stringValue = strVal[noti_index];
                printf("noti val : %s\n", notifyParams[noti_index].stringValue);
                notifyParams[noti_index].time = now_time;
                noti_index++;
            }
        }
         
        damda_Notify_DeviceStatus("2", notifyParams, noti_index);   

#endif
*/
        
	}
}

//========================================================================================//

void main_task(void *pvParameter)
{
    int task_count = 0;
   ESP_LOGI(TAG, "main_task started");
   

    while (true)
    {
             _mil_sec++;
            
             if(_mil_sec >= 20000)
             {
                 _mil_sec = 0;
                task_count++;
                 Notify_Func();
                  //ESP_LOGI(TAG,"\n => main_task started : %d, %d,%d\n",task_count,flash_info.ota_mode_fail, mqtt_start_flag);
                 if((mqtt_start_flag)&& (flash_info.ota_mode_fail == true)&&(task_count >= 20))
                 {
                         ESP_LOGI(TAG, "OTA_upgrade_fail start~~~");
                          OTA_upgrade_fail();
                        flash_info.ota_mode_fail = false;
                        write_flash(&flash_info);
                        mqtt_start_flag = false;
                        task_count = 0;
                 }
                  
                 if(damda_IsMQTTConnected_Error() == 1)
                 {
                    
         	   		printf("MQTT_EVENT_ERROR  -> Soft Ap mode Change damda_IsMQTTConnected_Error2 : %d \n", damda_IsMQTTConnected_Error());

                    printf("Soft AP ON \n");
                    printf("Wifi Disconnect \n");
                    printf("NVS format \n");
                     
                    nvs_flash_init();
                    ESP_ERROR_CHECK(nvs_flash_erase());
                     
                    vTaskDelay(5); 
                         
                    printf("MQTT_EVENT_ERROR ( Connection refused error )-> Soft AP MODE START \n");
                    vTaskDelay(5); 
                    esp_restart();
                    
                 }
             }
            
        vTaskDelay(10 / portTICK_PERIOD_MS); // portTICK_RATE_MS = 1000
    }
     vTaskDelete(NULL);
}


/*
    print format start

*/


void print_receiveData(ResponseMessageInfo* responseInfo){

    ESP_LOGI(TAG, "[print_receiveData] sid : %s",responseInfo->sid);
    ESP_LOGI(TAG, "[print_receiveData] sequence : %d",responseInfo->sequence);
    ESP_LOGI(TAG, "[print_receiveData] returnCode : %d",responseInfo->returnCode);
    ESP_LOGI(TAG, "[print_receiveData] errorCode : %d",responseInfo->errorCode);
    ESP_LOGI(TAG, "[print_receiveData] responseType : %d",responseInfo->responseType);
    ESP_LOGI(TAG, "[print_receiveData] responseArraySize : %d",responseInfo->responseArraySize);
    if(responseInfo->firmwareDownloadUrl != NULL){
        ESP_LOGI(TAG, "[print_receiveData] firmwareDownloadUrl :%s",responseInfo->firmwareDownloadUrl);
    }
    if(responseInfo->firmwareVersion != NULL){
        ESP_LOGI(TAG, "[print_receiveData] firmwareVersion :%s",responseInfo->firmwareVersion);
    }
    

    for (int i=0;i<responseInfo->responseArraySize;i++){
        ESP_LOGI(TAG, "[print_receiveData] resourceURI[%d] : %s",i,responseInfo->resourceUri[i]);
        //ESP_LOGI(TAG, "[print_receiveData] stringValue[%d] : %s",i,responseInfo->stringValue[i]);
       //ESP_LOGI(TAG, "[print_receiveData] time[%d] : %s",i,responseInfo->time[i]);
    }

}

void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] Minimum_Free memory: %d bytes", esp_get_minimum_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());   

    damda_nvs_init();
    esp_nvs_read();
    user_task_init();
    damda_uart_init(); 

    if (flash_info.booting_mode != NORMAL_MODE)
    {
        ESP_LOGI(TAG, "ESP_WIFI_MODE_AP\n");
        wifi_init_softap();        
    }
    else
    {
        ESP_LOGI(TAG, "ESP_WIFI_MODE_STA\n");
        wifi_init_sta();     
    }
    
    strcpy(DEVICE_FIRMWAREVERSION, flash_info.ver_string);
    strcpy(DEVICE_ETH_MAC, flash_info.esp_mac);
   
    #ifdef DEBUG
        ESP_LOGI(TAG,"\n => app_main DEVICE_MANUFACTURER : %s\n", DEVICE_MANUFACTURER); 
        ESP_LOGI(TAG,"\n => app_main DEVICE_MODEL : %s\n", DEVICE_MODEL); 
        ESP_LOGI(TAG,"\n => app_main DEVICE_MAC : %s\n", DEVICE_ETH_MAC); 
        ESP_LOGI(TAG,"\n => app_main DEVICE_TYPE : %s\n", DEVICE_TYPE); 
        ESP_LOGI(TAG,"\n => app_main DEVICE_FIRMWAREVERSION : %s\n", DEVICE_FIRMWAREVERSION); 
    #endif
    
    damdaSetDeviceInfo(DEVICE_MANUFACTURER, DEVICE_MODEL, DEVICE_ETH_MAC, flash_info.ver_string, DEVICE_TYPE);

    // xTaskCreatePinnedToCore(&main_task, "main_task", 1024*4, NULL, PRIORITY_MAIN, NULL, APP_CPU_NUM);
    message_print_flag = 1;

    if (flash_info.booting_mode == NORMAL_MODE)
    {
        printf("Normal Mode Start \r\n");
        if(damda_IsMQTTConnected() == 1) damdaMQTTDisConnect();
        
        printf(": Mqtt connect \r\n");
        printf("MQTT : %d", damda_IsMQTTConnected());
        damdaSetDeviceInfo(DEVICE_MANUFACTURER, DEVICE_MODEL, DEVICE_ETH_MAC, flash_info.ver_string, DEVICE_TYPE);

        // MQTT 클라이언트 초기화
        if (damdaMQTTConnect((const char*)server_addr_buffer, atoi(server_port_buffer), (const char*)auth_key_buffer) != 1)
        {
            ESP_LOGE(TAG, "MQTT client initialization failed");
            return;
        }

        obtain_time();
        test_init();

        printf("Mqtt Callback maked \n");        
        printf("MQTT : %d", damda_IsMQTTConnected());

        int i = 0;
        int max_attempts = 100; // 최대 시도 횟수 설정
        while(damda_IsMQTTConnected() != 1 && i < max_attempts)
        {
            i++;
            vTaskDelay(pdMS_TO_TICKS(1000)); // 1초 딜레이
        }

        if(damda_IsMQTTConnected() != 1)
        {
            ESP_LOGE(TAG, "MQTT connection failed after %d attempts", max_attempts);
            // 실패 처리 코드 추가
        }

        printf(": Register Device \r\n");
        damda_Req_RegisterDevice();
        
        ucnotify_flag = 1;
        vTaskDelay(100 / portTICK_PERIOD_MS);
        Notify_Func(); // Need to send a device resource data during device registration process. 
        
        ESP_LOGI(TAG, "[damdaMQTTDisConnect] Free memory: %d bytes", esp_get_free_heap_size());        

    }

    xTaskCreatePinnedToCore(&main_task, "main_task", 1024*4, NULL, PRIORITY_MAIN, NULL, APP_CPU_NUM);
    message_print_flag = 1;
}


/*
void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] Minimum_Free memory: %d bytes", esp_get_minimum_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());


    damda_nvs_init();
    esp_nvs_read();
   
    user_task_init();
    damda_uart_init(); 


    if (flash_info.booting_mode != NORMAL_MODE)
    {
        ESP_LOGI(TAG, "ESP_WIFI_MODE_AP\n");
        wifi_init_softap();        
    }
    else
    {
  		ESP_LOGI(TAG, "ESP_WIFI_MODE_STA\n");
    	wifi_init_sta();     
    }
    
   strcpy(DEVICE_FIRMWAREVERSION,flash_info.ver_string);

   strcpy(DEVICE_ETH_MAC,flash_info.esp_mac);
   
	#ifdef DEBUG
	 ESP_LOGI(TAG,"\n => app_main DEVICE_MANUFACTURER : %s\n",DEVICE_MANUFACTURER); 
	 ESP_LOGI(TAG,"\n => app_main DEVICE_MODEL : %s\n",DEVICE_MODEL); 
 
	 ESP_LOGI(TAG,"\n => app_main DEVICE_MAC : %s\n",DEVICE_ETH_MAC); 
	 ESP_LOGI(TAG,"\n => app_main DEVICE_TYPE : %s\n",DEVICE_TYPE); 

	 ESP_LOGI(TAG,"\n => app_main DEVICE_FIRMWAREVERSION : %s\n",DEVICE_FIRMWAREVERSION); 
	#endif
    
    xTaskCreatePinnedToCore(&main_task, "main_task", 1024*4, NULL, PRIORITY_MAIN, NULL,APP_CPU_NUM);
    message_print_flag = 1;

    if (flash_info.booting_mode == NORMAL_MODE)
    {
        if(damda_IsMQTTConnected() == 1)damdaMQTTDisConnect();
        
        printf(": Mqtt connect \r\n");
        printf("MQTT : %d",damda_IsMQTTConnected());
        damdaSetDeviceInfo(DEVICE_MANUFACTURER,DEVICE_MODEL,DEVICE_ETH_MAC,flash_info.ver_string,DEVICE_TYPE);

        RECONNECTING :
        damdaMQTTConnect((const char*)server_addr_buffer, atoi(server_port_buffer),(const char*)auth_key_buffer);        
        
        obtain_time();
        test_init();

        printf("Mqtt Callback maked \n");
		
        printf("MQTT : %d",damda_IsMQTTConnected());

        int i = 0;

        while(damda_IsMQTTConnected() != 1){
            //printf("##### Mqtt Callback wait %d \n",i);
            i++;
            if(damda_IsMQTTConnected_Error() == 1){
                free(auth_key_buffer);
                nvs_open("nvs",NVS_READWRITE,&nvs);
                size_t len;
                nvs_get_str(nvs, "auth_key", NULL, &len);
                auth_key_buffer = malloc(len);
                nvs_get_str(nvs, "auth_key", auth_key_buffer, &len);
            
                nvs_close(nvs);
                goto RECONNECTING;
            }
            obtain_time();
        }

        printf(": Register Device \r\n");
        damda_Req_RegisterDevice();
        
        ESP_LOGI(TAG, "[damdaMQTTDisConnect] Free memory: %d bytes", esp_get_free_heap_size());        

        ucnotify_flag = 1;
    }
} */