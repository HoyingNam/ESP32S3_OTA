#ifndef DAMDA_CORE_H
#define DAMDA_CORE_H

#include <string.h>
#include <stdlib.h>
#include "mqtt_client.h"
#include "cJson.h"
#include "damda_extern.h"
#include "esp_log.h"

#define DEBUG
#define TOPIC_VERSION "1.0"
#define PROFILE_VERSION "1.0"
#define SERVICE_PROTOCOL_VERSION "1.0"
#define MAX_SEQUENCE 10000000

#define BUF_LEN 1024
#define QOS 1
#define MESSAGE_TYPE_RESPONSE 1
#define MESSAGE_TYPE_READ 2
#define MESSAGE_TYPE_WRITE 3
#define MESSAGE_TYPE_EXECUTE 4
#define MESSAGE_TYPE_DELETE_SYNC 5
#define MESSAGE_TYPE_DELETE_ASYNC 6
#define MESSAGE_TYPE_FIRMWARE_DOWNLOAD 7
#define MESSAGE_TYPE_READ_ASYNC 8
#define MESSAGE_TYPE_WRITE_ASYNC 9
#define MESSAGE_TYPE_EXECUTE_ASYNC 10

#define MAX_RESOURCE_SIZE 10

#define SID_BUFFER_LEN 100
#define ERROR_CODE_BUFFER_LEN 10
#define RETURN_CODE_BUFFER_LEN 10
#define TIME_BUFFER_LEN 15

#define MQTT_RECON_DEFAULT_MS       (10*1000)  //default value
typedef struct notifyParamter{
	char* resourceUri;
	char* stringValue;
	char* time; 
} NotifyParameter;

char* time_execute;

#ifdef Operation_Mode_Certification
// pem code for Operation platform(AWS/DAMDA) 
static const char ca_pem[] = "-----BEGIN CERTIFICATE-----\r\n"
"MIIEkjCCA3qgAwIBAgITBn+USionzfP6wq4rAfkI7rnExjANBgkqhkiG9w0BAQsF\r\n"
"ADCBmDELMAkGA1UEBhMCVVMxEDAOBgNVBAgTB0FyaXpvbmExEzARBgNVBAcTClNj\r\n"
"b3R0c2RhbGUxJTAjBgNVBAoTHFN0YXJmaWVsZCBUZWNobm9sb2dpZXMsIEluYy4x\r\n"
"OzA5BgNVBAMTMlN0YXJmaWVsZCBTZXJ2aWNlcyBSb290IENlcnRpZmljYXRlIEF1\r\n"
"dGhvcml0eSAtIEcyMB4XDTE1MDUyNTEyMDAwMFoXDTM3MTIzMTAxMDAwMFowOTEL\r\n"
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\r\n"
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\r\n"
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\r\n"
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\r\n"
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\r\n"
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\r\n"
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\r\n"
"jgSubJrIqg0CAwEAAaOCATEwggEtMA8GA1UdEwEB/wQFMAMBAf8wDgYDVR0PAQH/\r\n"
"BAQDAgGGMB0GA1UdDgQWBBSEGMyFNOy8DJSULghZnMeyEE4KCDAfBgNVHSMEGDAW\r\n"
"gBScXwDfqgHXMCs4iKK4bUqc8hGRgzB4BggrBgEFBQcBAQRsMGowLgYIKwYBBQUH\r\n"
"MAGGImh0dHA6Ly9vY3NwLnJvb3RnMi5hbWF6b250cnVzdC5jb20wOAYIKwYBBQUH\r\n"
"MAKGLGh0dHA6Ly9jcnQucm9vdGcyLmFtYXpvbnRydXN0LmNvbS9yb290ZzIuY2Vy\r\n"
"MD0GA1UdHwQ2MDQwMqAwoC6GLGh0dHA6Ly9jcmwucm9vdGcyLmFtYXpvbnRydXN0\r\n"
"LmNvbS9yb290ZzIuY3JsMBEGA1UdIAQKMAgwBgYEVR0gADANBgkqhkiG9w0BAQsF\r\n"
"AAOCAQEAYjdCXLwQtT6LLOkMm2xF4gcAevnFWAu5CIw+7bMlPLVvUOTNNWqnkzSW\r\n"
"MiGpSESrnO09tKpzbeR/FoCJbM8oAxiDR3mjEH4wW6w7sGDgd9QIpuEdfF7Au/ma\r\n"
"eyKdpwAJfqxGF4PcnCZXmTA5YpaP7dreqsXMGz7KQ2hsVxa81Q4gLv7/wmpdLqBK\r\n"
"bRRYh5TmOTFffHPLkIhqhBGWJ6bt2YFGpn6jcgAKUj6DiAdjd4lpFw85hdKrCEVN\r\n"
"0FE6/V1dN2RMfjCyVSRCnTawXZwXgWHxyvkQAiSr6w10kY17RSlQOYiypok1JR4U\r\n"
"akcjMS9cmvqtmg5iUaQqqcT5NJ0hGA==\r\n"
"-----END CERTIFICATE-----";

#else
// pem code for developmemt platform(KEA/DAMDA)
static const char ca_pem[] = "-----BEGIN CERTIFICATE-----\r\n"
"MIIDSjCCAjKgAwIBAgIQRK+wgNajJ7qJMDmGLvhAazANBgkqhkiG9w0BAQUFADA/\r\n"
"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\r\n"
"DkRTVCBSb290IENBIFgzMB4XDTAwMDkzMDIxMTIxOVoXDTIxMDkzMDE0MDExNVow\r\n"
"PzEkMCIGA1UEChMbRGlnaXRhbCBTaWduYXR1cmUgVHJ1c3QgQ28uMRcwFQYDVQQD\r\n"
"Ew5EU1QgUm9vdCBDQSBYMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\r\n"
"AN+v6ZdQCINXtMxiZfaQguzH0yxrMMpb7NnDfcdAwRgUi+DoM3ZJKuM/IUmTrE4O\r\n"
"rz5Iy2Xu/NMhD2XSKtkyj4zl93ewEnu1lcCJo6m67XMuegwGMoOifooUMM0RoOEq\r\n"
"OLl5CjH9UL2AZd+3UWODyOKIYepLYYHsUmu5ouJLGiifSKOeDNoJjj4XLh7dIN9b\r\n"
"xiqKqy69cK3FCxolkHRyxXtqqzTWMIn/5WgTe1QLyNau7Fqckh49ZLOMxt+/yUFw\r\n"
"7BZy1SbsOFU5Q9D8/RhcQPGX69Wam40dutolucbY38EVAjqr2m7xPi71XAicPNaD\r\n"
"aeQQmxkqtilX4+U9m5/wAl0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNV\r\n"
"HQ8BAf8EBAMCAQYwHQYDVR0OBBYEFMSnsaR7LHH62+FLkHX/xBVghYkQMA0GCSqG\r\n"
"SIb3DQEBBQUAA4IBAQCjGiybFwBcqR7uKGY3Or+Dxz9LwwmglSBd49lZRNI+DT69\r\n"
"ikugdB/OEIKcdBodfpga3csTS7MgROSR6cz8faXbauX+5v3gTt23ADq1cEmv8uXr\r\n"
"AvHRAosZy5Q6XkjEGB5YGV8eAlrwDPGxrancWYaLbumR9YbK+rlmM6pZW87ipxZz\r\n"
"R8srzJmwN0jP41ZL9c8PDHIyh8bwRLtTcm1D9SZImlJnt1ir/md2cXjbDaJWFBM5\r\n"
"JDGFoqgCWjBH4d1QB7wCCZAA62RjYJsWvIjJEubSfZGL+T0yjWW06XyxV3bqxbYo\r\n"
"Ob8VZRzI9neWagqNdwvYkQsEjgfbKbYK7p2CNTUQ\r\n"
"-----END CERTIFICATE-----";

#endif

typedef struct responseMessageInfo{
	char sid[SID_BUFFER_LEN];
	char* resourceUri[MAX_RESOURCE_SIZE];
	char* stringValue[MAX_RESOURCE_SIZE];
	int returnCode;
	int errorCode;
	char* time[MAX_RESOURCE_SIZE];
	int responseType;
	int responseArraySize;
	char* firmwareDownloadUrl;
	char* firmwareVersion;
	int sequence;
} ResponseMessageInfo;

static esp_mqtt_client_handle_t client = NULL; // 전역 변수로 클라이언트 핸들을 선언

// esp_mqtt_client_handle_t client;
char device_id[80];  // char device_id[40]; //WINIADIMCHAE-WD_REFRIGERATOR2-A4CF12561204 42
char *device_type;
typedef void (*MSG_ARRIVED_CALLBACK_FUNCTION)(ResponseMessageInfo*);

char subscribeTopic[200];

/*---damdaSDKCore Function Start---*/

int damdaSetDeviceInfo(char* manufacturer,char* model,char* serial,char* firmwareVersion,char* device_type);

int  damda_Req_RegisterDevice();

int damda_Req_DeregisterDevice();

int damda_Req_UpdateDeviceInfo();

int damda_Req_ServerTimeInfo();

int damda_Notify_DeviceStatus(char* notificationType,NotifyParameter notifyParams[],int arrayCount);

int damda_Notify_DeviceStatuArr(char* notificationType,NotifyParameter notifyParams[],int arrayCount);

int damda_Rsp_RemoteControl(int controlType,const char* sid,const char* returnCode,char* resourceUri[],char* stringValue[],int arrayCount);

int damda_Rsp_RemoteDelDevice(const char* sid,const char* returnCode);

void damda_setMQTTMsgReceiveCallback(MSG_ARRIVED_CALLBACK_FUNCTION func);

int damda_Rsp_Client_Busy (int controlType,const char* sid,const char* returnCode,int arrayCount);

void getSid(char* message,char* returnValue);

int getSeq(char* message);

void getOperation(char* message,char* returnValue);

int getReturnCode(char* message);

int getErrorCode(char* message);

void getResourceUri(char* message,int index,char* returnValue);

void getStringValue(const char* message, int index, char* returnValue, int returnValueSize);

void getTime(char* message,int index,char* returnValue);

int getResourceUriLength(char* message,int index);

int getStringValueLength(char* message,int index);

int getTimeLength(const char* message,int index);

void getServiceId(char* topic,char* returnValue);

void getMsgType(char* topic,char* returnValue);

int getMessageType(char*topic,char *message);

void getOriginId(char* topic,char* returnValue);

void getTargetId(char* topic,char* returnValue);

void getTopicOprId(char* topic,char* returnValue);

int isFirmwareUpdateMessage(char* message);

void  getStringValueByResourceUri(char* message,char* n, char* returnValue);

int getNArraySize(char* message);

int damda_Rsp_RemoteControl_noEntity(int controlType,const char* sid,const char* returnCode);

int damda_Req_DeleteDevice();


/*---damdaSDKCore Function End---*/



int damdaMQTTConnect(const char* host, int port,const char* password);

void damdaMQTTDisConnect();

int damda_IsMQTTConnected();

int damda_IsMQTTConnected_Error(); 

#endif /* DAMDA_CORE_H */