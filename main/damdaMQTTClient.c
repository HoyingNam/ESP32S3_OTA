#include "damdaCore.h"

//int noti_index = 0;

#include <stdbool.h>
int messageArrivedCallbackStatus;
int damdaConnectedStatus = 0;
int damdaConnectedErrorStatus = 0;
int mqtt_err_count = 0;
bool mqtt_start_flag  = false;


//char subscribeTopic[100];
MSG_ARRIVED_CALLBACK_FUNCTION callbackFunc = NULL; //messageArrivedCallback

void damda_setMQTTMsgReceiveCallback(MSG_ARRIVED_CALLBACK_FUNCTION func){
	callbackFunc = func;
	messageArrivedCallbackStatus = 1;
}

void messageArrived(char* topic, int topic_len, char* data, int data_len) {
    const int cTopic_len = topic_len;
    const int cData_len = data_len;
    char receivedTopic[cTopic_len + 1];
    char receivedData[cData_len + 1];
    char* name[10] = {0}; // Initialize to NULL
    char* sv[10] = {0};   // Initialize to NULL
    char* ti[10] = {0};   // Initialize to NULL
    int nameIdx = 0;
    int svIdx = 0;
    int tiIdx = 0;
    int freeIndex = 0;
    int nSize;
    int svSize;
    int tiSize;
    int urllen;
    int firmwareVersionlen;
    char downloadSid[100];
    char temp_firmwareversion[10];
    char temp_firmwareurl[100];

    // Copy topic and data with null-termination
    strncpy(receivedTopic, topic, topic_len);
    strncpy(receivedData, data, data_len);
    receivedData[data_len] = '\0';
    receivedTopic[topic_len] = '\0';

#ifdef DEBUG
    printf("receivedTopic is %s\n", receivedTopic);
    printf("receivedData is %s\n", receivedData);
#endif

    ResponseMessageInfo resMsgInfo;
    char sid[40];
    getSid(receivedData, sid);
    int seq = getSeq(receivedData);
    int returnCode = getReturnCode(receivedData);
    int errorCode = getErrorCode(receivedData);
    int messageType = getMessageType(receivedTopic, receivedData);
    int arraySize = getNArraySize(receivedData);
    char topicOprId[10];
    getTopicOprId(receivedTopic, topicOprId);

    resMsgInfo.responseArraySize = arraySize;
    resMsgInfo.sequence = seq;
    resMsgInfo.errorCode = errorCode;
    resMsgInfo.returnCode = returnCode;
    resMsgInfo.responseType = messageType;
    resMsgInfo.firmwareVersion = NULL;
    resMsgInfo.firmwareDownloadUrl = NULL;
    if (sid != NULL) {
        strcpy(resMsgInfo.sid, sid);
    }

    if (arraySize > 0) {
        for (nameIdx = 0; nameIdx < arraySize; nameIdx++) {
            nSize = getResourceUriLength(receivedData, nameIdx);
            name[nameIdx] = (char*)malloc(sizeof(char) * (nSize + 1));
            if (name[nameIdx] == NULL) {
                printf("Failed to allocate memory for name[%d]\n", nameIdx);
                return;
            }
            getResourceUri(receivedData, nameIdx, name[nameIdx]);

#ifdef DEBUG
            printf("\nparsing resourceURI name in loop : %s", name[nameIdx]);
#endif

            resMsgInfo.resourceUri[nameIdx] = (char*)malloc(sizeof(char) * (nSize + 1));
            if (resMsgInfo.resourceUri[nameIdx] == NULL) {
                printf("Failed to allocate memory for resMsgInfo.resourceUri[%d]\n", nameIdx);
                return;
            }
            strcpy(resMsgInfo.resourceUri[nameIdx], name[nameIdx]);

#ifdef DEBUG
            printf("\nparsing resourceURI in loop : %s", resMsgInfo.resourceUri[nameIdx]);
#endif
        }

        if (getStringValueLength(receivedData, 0) != 0) {
            for (svIdx = 0; svIdx < arraySize; svIdx++) {
                svSize = getStringValueLength(receivedData, svIdx);
                sv[svIdx] = (char*)malloc(sizeof(char) * (svSize + 1));
                if (sv[svIdx] == NULL) {
                    printf("Failed to allocate memory for sv[%d]\n", svIdx);
                    return;
                }
                getStringValue(receivedData, svIdx, sv[svIdx], svSize + 1);

#ifdef DEBUG
                printf("\nparsing stringValue value in loop : %s", sv[svIdx]);
#endif

                resMsgInfo.stringValue[svIdx] = (char*)malloc(sizeof(char) * (svSize + 1));
                if (resMsgInfo.stringValue[svIdx] == NULL) {
                    printf("Failed to allocate memory for resMsgInfo.stringValue[%d]\n", svIdx);
                    return;
                }
                strcpy(resMsgInfo.stringValue[svIdx], sv[svIdx]);

#ifdef DEBUG
                printf("parsing stringValue in loop : %s", resMsgInfo.stringValue[svIdx]);
#endif
            }
        }

        if (getTimeLength(receivedData, 0) != 0) {
            for (tiIdx = 0; tiIdx < arraySize; tiIdx++) {
                tiSize = getTimeLength(receivedData, tiIdx);
                ti[tiIdx] = (char*)malloc(sizeof(char) * (tiSize + 1));
                if (ti[tiIdx] == NULL) {
                    printf("Failed to allocate memory for ti[%d]\n", tiIdx);
                    return;
                }
                getTime(receivedData, tiIdx, ti[tiIdx]);

                resMsgInfo.time[tiIdx] = (char*)malloc(sizeof(char) * (tiSize + 1));
                if (resMsgInfo.time[tiIdx] == NULL) {
                    printf("Failed to allocate memory for resMsgInfo.time[%d]\n", tiIdx);
                    return;
                }
                strcpy(resMsgInfo.time[tiIdx], ti[tiIdx]);
            }
        }
    }

    if (messageType == MESSAGE_TYPE_DELETE_SYNC) {
        damda_Rsp_RemoteDelDevice(resMsgInfo.sid, "200");
        damda_Req_DeleteDevice();
    } else if (messageType == MESSAGE_TYPE_DELETE_ASYNC) {
        damda_Req_DeleteDevice();
    }

    if (messageType == MESSAGE_TYPE_FIRMWARE_DOWNLOAD) {
        if (strcmp(topicOprId, "write") == 0) {
            strcpy(downloadSid, sid);
            damda_Rsp_RemoteControl_noEntity(MESSAGE_TYPE_WRITE, downloadSid, "200");
        }

        getStringValueByResourceUri(receivedData, "/5/0/1", temp_firmwareurl);
        printf("\n getStringValueByResourceUri => temp_firmwareurl  : %s \n", temp_firmwareurl);
        urllen = strlen(temp_firmwareurl);
        resMsgInfo.firmwareDownloadUrl = (char*)malloc(urllen + 1);
        if (resMsgInfo.firmwareDownloadUrl == NULL) {
            printf("Failed to allocate memory for firmwareDownloadUrl\n");
            return;
        }
        strcpy(resMsgInfo.firmwareDownloadUrl, temp_firmwareurl);

        getStringValueByResourceUri(receivedData, "/3/0/3", temp_firmwareversion);
        printf("\n getStringValueByResourceUri => temp_firmwareversion  : %s \n", temp_firmwareversion);
        firmwareVersionlen = strlen(temp_firmwareversion);
        resMsgInfo.firmwareVersion = (char*)malloc(firmwareVersionlen + 1);
        if (resMsgInfo.firmwareVersion == NULL) {
            printf("Failed to allocate memory for firmwareVersion\n");
            return;
        }
        strcpy(resMsgInfo.firmwareVersion, temp_firmwareversion);
    }

    if (callbackFunc != NULL) {
        callbackFunc(&resMsgInfo);
    }

    // Free allocated memory
    for (freeIndex = 0; freeIndex < arraySize; freeIndex++) {
        if (name[freeIndex]) free(name[freeIndex]);
        if (resMsgInfo.resourceUri[freeIndex]) free(resMsgInfo.resourceUri[freeIndex]);

        if (getStringValueLength(receivedData, 0) != 0) {
            if (sv[freeIndex]) free(sv[freeIndex]);
            if (resMsgInfo.stringValue[freeIndex]) free(resMsgInfo.stringValue[freeIndex]);
        }

        if (getTimeLength(receivedData, 0) != 0) {
            if (ti[freeIndex]) free(ti[freeIndex]);
            if (resMsgInfo.time[freeIndex]) free(resMsgInfo.time[freeIndex]);
        }
    }

    if (messageType == MESSAGE_TYPE_FIRMWARE_DOWNLOAD) {
        if (resMsgInfo.firmwareDownloadUrl) free(resMsgInfo.firmwareDownloadUrl);
        if (resMsgInfo.firmwareVersion) free(resMsgInfo.firmwareVersion);
    }
}


// MQTT 이벤트 핸들러
static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            damdaConnectedStatus = 1;
            printf("MQTT Connected.\n");
            esp_mqtt_client_subscribe(client, subscribeTopic, 0);
            mqtt_start_flag = true;
            break;
        case MQTT_EVENT_DISCONNECTED:
            damdaConnectedStatus = 0;
            printf("MQTT Disconnected.\n");
			esp_mqtt_client_reconnect(client);
            break;
        case MQTT_EVENT_SUBSCRIBED:
            printf("Subscribe Done\n");
            break;
        case MQTT_EVENT_DATA:
            messageArrived(event->topic, event->topic_len, event->data, event->data_len);
            #ifdef DEBUG
            printf(" ==> TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf(" ==> DATA=%.*s\r\n", event->data_len, event->data);
            printf("DATA Arrive\n");
            #endif
            break;
        case MQTT_EVENT_ERROR:
            printf("MQTT_EVENT_ERROR\n");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_ESP_TLS) {
                printf("Last error code reported from esp-tls: 0x%x\n", event->error_handle->esp_tls_last_esp_err);
                printf("Last tls stack error number: 0x%x\n", event->error_handle->esp_tls_stack_err);
                damdaConnectedErrorStatus = 0;
            } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
                printf("Connection refused error: 0x%x\n", event->error_handle->connect_return_code);
                mqtt_err_count++;
                printf("Connection refused error %d\n", mqtt_err_count);
                if (mqtt_err_count > 5) {
                    mqtt_err_count = 0;
                    damdaConnectedErrorStatus = 1;
                    printf("Connection refused error damdaConnectedStatus: %d\n", damdaConnectedErrorStatus);
                }
            } else {
                printf("Unknown error type: 0x%x\n", event->error_handle->error_type);
            }
            break;
        default:
            break;
    }
    return ESP_OK;
}

int damdaMQTTConnect(const char* host, int port, const char* password)
{
    char realHost[30] = "mqtts://"; 
    strcat(realHost, host);
    
    #ifdef DEBUG
        printf("Connected Host : %s\n", realHost);
        printf("---- MQTT password : %s \n", password);
    #endif

    const esp_mqtt_client_config_t mqtt_cfg = {
        .host = host,
        .event_handle = mqtt_event_handler,
        .client_id = device_id,
        .username = device_id,
        .password = password,
        .port = port,
        .reconnect_timeout_ms = MQTT_RECON_DEFAULT_MS,
        .cert_pem = ca_pem,
        .transport = MQTT_TRANSPORT_OVER_SSL
    };

    // 디버그 출력
    printf("ca_pem %s\n", mqtt_cfg.cert_pem);
    printf("host %s\n", host);
    printf("password %s\n", mqtt_cfg.password);
    printf("port %d\n", port);
    printf("device_id %s\n", device_id);
    sprintf(subscribeTopic, "%s/+/iot-server/%s/#", device_type, device_id);
    printf("subscribeTopic %s\n", subscribeTopic);

    // 클라이언트 초기화 및 시작
    client = esp_mqtt_client_init(&mqtt_cfg);
    if (client == NULL) {
        printf("Failed to initialize MQTT client");
        return -1;
    }
	esp_mqtt_client_start(client);

    return 1;
}

void damdaMQTTDisConnect(){
	//esp_mqtt_client_stop(client);
	//esp_mqtt_client_destroy(client);
	if (client != NULL) {
        esp_mqtt_client_stop(client);  // MQTT 클라이언트 정지
        esp_mqtt_client_destroy(client);  // MQTT 클라이언트 파괴
        client = NULL;  // 클라이언트 변수를 NULL로 설정하여 이후 재사용을 방지합니다.
    }
}
/*void damdaMQTTConnect(){
	esp_mqtt_client_start(client);
}*/
int damda_IsMQTTConnected(){
	return damdaConnectedStatus;
}

int damda_IsMQTTConnected_Error(){
	return damdaConnectedErrorStatus;
}


