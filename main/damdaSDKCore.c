
#include <unistd.h>
#include "damdaCore.h"

// static const char *TAG = "damdasdkcore";

char *firmware_version;
char *manufacturer;
char *model;
char *serial;

int sequence = 0 ; 

int damdaSetDeviceInfo(char* m_manufacturer,char* m_model,char* m_serial,char* m_firmwareVersion,char* m_deviceType){
	manufacturer = m_manufacturer;
	model = m_model;
	serial = m_serial;
//	serial = "20240422";
	firmware_version = m_firmwareVersion;
	device_type = m_deviceType;
	sprintf(device_id,"%s-%s-%s",manufacturer,model,serial);
    
	if (device_id == NULL)
	{
		printf("device_id가 없음");
    	goto end;
    }
    
    return 1;
end:
    return -2;
}

int damda_Req_RegisterDevice(){
	
	char *n[7] = {"/3/0/0","/3/0/1","/3/0/2","/3/0/3","/3/0/997","/3/0/998","/3/0/999"};
	char topic[200];
	char *sendMsg;
	char seqString[10];
	cJSON *root = cJSON_CreateObject();	
	cJSON *msg = cJSON_CreateObject();
	cJSON *eArray = cJSON_CreateArray();
	
	cJSON *manufacturerJson = cJSON_CreateObject();
	cJSON *modelJson = cJSON_CreateObject();
	cJSON *serialJson = cJSON_CreateObject();
	cJSON *firmwareJson = cJSON_CreateObject();
	cJSON *topicVersionJson = cJSON_CreateObject();
	cJSON *profileVersionJson = cJSON_CreateObject();
	cJSON *serviceProtocolVersionJson = cJSON_CreateObject();
	
	cJSON_AddStringToObject(root,"sid","0");
	sequence = (sequence + 1) % MAX_SEQUENCE;
	sprintf(seqString,"%d",sequence);
	cJSON_AddStringToObject(root,"seq",seqString);
	
	
	//cJSON_AddStringToObject(root,"did","DAMDA-DDSensor4_TestSensor-E465B80AFB80");
	cJSON_AddStringToObject(root,"did",device_id);
	
	cJSON_AddStringToObject(manufacturerJson,"n",n[0]);
	cJSON_AddStringToObject(modelJson,"n",n[1]);
	cJSON_AddStringToObject(serialJson,"n",n[2]);
	cJSON_AddStringToObject(firmwareJson,"n",n[3]);
	cJSON_AddStringToObject(topicVersionJson,"n",n[4]);
	cJSON_AddStringToObject(profileVersionJson,"n",n[5]);
	cJSON_AddStringToObject(serviceProtocolVersionJson,"n",n[6]);
	
	cJSON_AddStringToObject(manufacturerJson,"sv",manufacturer);
	cJSON_AddStringToObject(modelJson,"sv",model);
	cJSON_AddStringToObject(serialJson,"sv",serial);
	cJSON_AddStringToObject(firmwareJson,"sv",firmware_version);
	cJSON_AddStringToObject(topicVersionJson,"sv",TOPIC_VERSION);
	cJSON_AddStringToObject(profileVersionJson,"sv",PROFILE_VERSION);
	cJSON_AddStringToObject(serviceProtocolVersionJson,"sv",SERVICE_PROTOCOL_VERSION);
	
	cJSON_AddItemToArray(eArray,manufacturerJson);
	cJSON_AddItemToArray(eArray,modelJson);
	cJSON_AddItemToArray(eArray,serialJson);
	cJSON_AddItemToArray(eArray,firmwareJson);
	cJSON_AddItemToArray(eArray,topicVersionJson);
	cJSON_AddItemToArray(eArray,profileVersionJson);
	cJSON_AddItemToArray(eArray,serviceProtocolVersionJson);
	
	cJSON_AddStringToObject(msg,"o","rg");
	cJSON_AddItemToObject(msg,"e",eArray);
    
	cJSON_AddItemToObject(root,"msg",msg);
	
	sendMsg = cJSON_Print(root);
    if (sendMsg == NULL)
    {
        fprintf(stderr, "Failed to print monitor.\n");        
        goto end;
    }
	#ifdef DEBUG
		printf("sendMsg : \n%s\n",sendMsg);
	#endif
	sprintf(topic,"%s/sync/%s/iot-server/register/json",device_type,device_id);
	
	esp_mqtt_client_publish(client, topic, sendMsg, 0, 1, 0);
	
	free(sendMsg);
	cJSON_Delete(root);

    return 1;
end:
    cJSON_Delete(root);  
    return -3;
}

int damda_Req_DeregisterDevice(){
	
	char *n[3] = {"/3/0/0","/3/0/1","/3/0/2"};
	char topic[200];
	char *sendMsg;
	char seqString[10];
	cJSON *root = cJSON_CreateObject();	
	cJSON *msg = cJSON_CreateObject();
	cJSON *eArray = cJSON_CreateArray();
	
	cJSON *manufacturerJson = cJSON_CreateObject();
	cJSON *modelJson = cJSON_CreateObject();
	cJSON *serialJson = cJSON_CreateObject();
	
	cJSON_AddStringToObject(root,"sid","0");
	
	sequence = (sequence + 1) % MAX_SEQUENCE;
	sprintf(seqString,"%d",sequence);
	cJSON_AddStringToObject(root,"seq",seqString);
	
	cJSON_AddStringToObject(root,"did",device_id);
	
	cJSON_AddStringToObject(manufacturerJson,"n",n[0]);
	cJSON_AddStringToObject(modelJson,"n",n[1]);
	cJSON_AddStringToObject(serialJson,"n",n[2]);

	
	cJSON_AddStringToObject(manufacturerJson,"sv",manufacturer);
	cJSON_AddStringToObject(modelJson,"sv",model);
	cJSON_AddStringToObject(serialJson,"sv",serial);

	
	
	cJSON_AddItemToArray(eArray,manufacturerJson);
	cJSON_AddItemToArray(eArray,modelJson);
	cJSON_AddItemToArray(eArray,serialJson);
	
	cJSON_AddStringToObject(msg,"o","urg");
	cJSON_AddItemToObject(msg,"e",eArray);
	
	cJSON_AddItemToObject(root,"msg",msg);
	
	sendMsg = cJSON_Print(root);
    if (sendMsg == NULL)
    {
        sprintf(stderr, "Failed to print monitor.\n");        
        goto end;
    }
   
	#ifdef DEBUG
		printf("sendMsg : \n%s\n",sendMsg);
	#endif
	sprintf(topic,"%s/sync/%s/iot-server/unregister/json",device_type,device_id);
	
	esp_mqtt_client_publish(client, topic, sendMsg, 0, 1, 0);
	
	free(sendMsg);
	cJSON_Delete(root);

    return 1;
end:
    cJSON_Delete(root);  
    return -3;
}

int damda_Req_DeleteDevice(){
	
	char *n[3] = {"/3/0/0","/3/0/1","/3/0/2"};
	char topic[200];
	char *sendMsg;
	char seqString[10];

	cJSON *root = cJSON_CreateObject();	
	cJSON *msg = cJSON_CreateObject();
	cJSON *eArray = cJSON_CreateArray();
	
	cJSON *manufacturerJson = cJSON_CreateObject();
	cJSON *modelJson = cJSON_CreateObject();
	cJSON *serialJson = cJSON_CreateObject();
	
	cJSON_AddStringToObject(root,"sid","0");
	
	sequence = (sequence + 1) % MAX_SEQUENCE;
	sprintf(seqString,"%d",sequence);
	cJSON_AddStringToObject(root,"seq",seqString);
	
	cJSON_AddStringToObject(root,"did",device_id);
	
	cJSON_AddStringToObject(manufacturerJson,"n",n[0]);
	cJSON_AddStringToObject(modelJson,"n",n[1]);
	cJSON_AddStringToObject(serialJson,"n",n[2]);

	
	cJSON_AddStringToObject(manufacturerJson,"sv",manufacturer);
	cJSON_AddStringToObject(modelJson,"sv",model);
	cJSON_AddStringToObject(serialJson,"sv",serial);

	
	
	cJSON_AddItemToArray(eArray,manufacturerJson);
	cJSON_AddItemToArray(eArray,modelJson);
	cJSON_AddItemToArray(eArray,serialJson);
	
	cJSON_AddStringToObject(msg,"o","drg");
	cJSON_AddItemToObject(msg,"e",eArray);
	
	cJSON_AddItemToObject(root,"msg",msg);
	
	sendMsg = cJSON_Print(root);
    if (sendMsg == NULL)
    {
        fprintf(stderr, "Failed to print monitor.\n");        
        goto end;
    }
   
	#ifdef DEBUG
		printf("sendMsg : \n%s\n",sendMsg);
	#endif
	sprintf(topic,"%s/sync/%s/iot-server/deregister/json",device_type,device_id);
	
	esp_mqtt_client_publish(client, topic, sendMsg, 0, 1, 0);
	
	free(sendMsg);
	cJSON_Delete(root);

    return 1;
end:
    cJSON_Delete(root);  
    return -3;    
}

int damda_Req_UpdateDeviceInfo(){

	printf("damda_Req_UpdateDeviceInfo init\r\n");	
	// device_id가 초기화되었는지 확인
    if (device_id == NULL || strlen(device_id) == 0) {
        fprintf(stderr, "Error: device_id is not initialized.\n");
        return -1;
    }
	
	char *n[7] = {"/3/0/0","/3/0/1","/3/0/2","/3/0/3","/3/0/997","/3/0/998","/3/0/999"};
	char topic[200];
	char *sendMsg;
	char seqString[10];
	
	cJSON *root = cJSON_CreateObject();	
	cJSON *msg = cJSON_CreateObject();
	cJSON *eArray = cJSON_CreateArray();
	
	cJSON *manufacturerJson = cJSON_CreateObject();
	cJSON *modelJson = cJSON_CreateObject();
	cJSON *serialJson = cJSON_CreateObject();
	cJSON *firmwareJson = cJSON_CreateObject();
	cJSON *topicVersionJson = cJSON_CreateObject();
	cJSON *profileVersionJson = cJSON_CreateObject();
	cJSON *serviceProtocolVersionJson = cJSON_CreateObject();
	
	cJSON_AddStringToObject(root,"sid","3");
	
	sequence = (sequence + 1) % MAX_SEQUENCE;
	sprintf(seqString,"%d",sequence);
	cJSON_AddStringToObject(root,"seq",seqString);
	
	cJSON_AddStringToObject(root,"did",device_id);
	
	cJSON_AddStringToObject(manufacturerJson,"n",n[0]);
	cJSON_AddStringToObject(modelJson,"n",n[1]);
	cJSON_AddStringToObject(serialJson,"n",n[2]);
	cJSON_AddStringToObject(firmwareJson,"n",n[3]);
	cJSON_AddStringToObject(topicVersionJson,"n",n[4]);
	cJSON_AddStringToObject(profileVersionJson,"n",n[5]);
	cJSON_AddStringToObject(serviceProtocolVersionJson,"n",n[6]);
	
	cJSON_AddStringToObject(manufacturerJson,"sv",manufacturer);
	cJSON_AddStringToObject(modelJson,"sv",model);
	cJSON_AddStringToObject(serialJson,"sv",serial);
	cJSON_AddStringToObject(firmwareJson,"sv",firmware_version);
	cJSON_AddStringToObject(topicVersionJson,"sv",TOPIC_VERSION);
	cJSON_AddStringToObject(profileVersionJson,"sv",PROFILE_VERSION);
	cJSON_AddStringToObject(serviceProtocolVersionJson,"sv",SERVICE_PROTOCOL_VERSION);
	
	cJSON_AddItemToArray(eArray,manufacturerJson);
	cJSON_AddItemToArray(eArray,modelJson);
	cJSON_AddItemToArray(eArray,serialJson);
	cJSON_AddItemToArray(eArray,firmwareJson);
	cJSON_AddItemToArray(eArray,topicVersionJson);
	cJSON_AddItemToArray(eArray,profileVersionJson);
	cJSON_AddItemToArray(eArray,serviceProtocolVersionJson);
	
	cJSON_AddStringToObject(msg,"o","up");
	cJSON_AddItemToObject(msg,"e",eArray);
	
	cJSON_AddItemToObject(root,"msg",msg);
	
	sendMsg = cJSON_Print(root);
    if (sendMsg == NULL)
    {
        fprintf(stderr, "Failed to print monitor.\n");        
        goto end;
    }
   
	#ifdef DEBUG
	    printf("device_id : %s\n", device_id);
   		printf("device_type : %s\n", device_type);
		printf("sendMsg : \n%s\n",sendMsg);
	#endif
	sprintf(topic,"%s/sync/%s/iot-server/update/json",device_type,device_id);

	if (client == NULL) {
        fprintf(stderr, "Error: MQTT client is not initialized.\n");
        free(sendMsg);
        cJSON_Delete(root);
        return -2;
    }
	
	esp_mqtt_client_publish(client, topic, sendMsg, 0, 1, 0);

	free(sendMsg);
	cJSON_Delete(root);

    return 1;
end:
    cJSON_Delete(root);  
    return -3;    
}


int damda_Req_ServerTimeInfo(){
	
	char topic[200];
	char *sendMsg;
	char seqString[10];
	
	cJSON *root = cJSON_CreateObject();	
	cJSON *msg = cJSON_CreateObject();
	
	
	
	cJSON_AddStringToObject(root,"sid","0");
	
	sequence = (sequence + 1) % MAX_SEQUENCE;
	sprintf(seqString,"%d",sequence);
	cJSON_AddStringToObject(root,"seq",seqString);
	
	cJSON_AddStringToObject(root,"did",device_id);
	
	
	cJSON_AddStringToObject(msg,"o","tisyn");
		
	cJSON_AddItemToObject(root,"msg",msg);
	
	sendMsg = cJSON_Print(root);
    if (sendMsg == NULL)
    {
        fprintf(stderr, "Failed to print monitor.\n");        
        goto end;
    }
   
	#ifdef DEBUG
		printf("sendMsg : \n%s\n",sendMsg);
	#endif
	sprintf(topic,"%s/sync/%s/iot-server/timesync/json",device_type,device_id);
	
	esp_mqtt_client_publish(client, topic, sendMsg, 0, 1, 0);
	
	free(sendMsg);
	cJSON_Delete(root);

    return 1;

    end:
	    cJSON_Delete(root);  
         return -3;
        
}

int damda_Notify_DeviceStatus(char* notificationType,NotifyParameter notifyParams[],int arrayCount){
	
	char topic[200];
	char *sendMsg;
	char seqString[10];
	int i=0;
	int max_iter = 0; //Maximum number of iterations in a loop
	int j = 0;  // NotifyParam index	
	const int one_unit = 5; //Noti Maximum number of iterations
	int iter_count = arrayCount/one_unit; //Noti number of times to send with maximum number of iteration
	int remainder = arrayCount%one_unit; // Noti remaining iteration count
    
	sprintf(topic,"%s/sync/%s/iot-server/notify/json",device_type,device_id);
    
	cJSON *root ;    
    cJSON *msg ;
    cJSON *eArray;
    cJSON *paramsJson[50]; 

    while(iter_count >= 0){
		root = cJSON_CreateObject();
		msg = cJSON_CreateObject();
		eArray = cJSON_CreateArray();
	
		cJSON_AddStringToObject(root,"sid",notificationType);

		sequence = (sequence + 1) % MAX_SEQUENCE;
		sprintf(seqString,"%d",sequence);
		cJSON_AddStringToObject(root,"seq",seqString);
	
		cJSON_AddStringToObject(root,"did",device_id);
		max_iter  = iter_count > 0 ? one_unit : remainder ;

        for(i=0;i<max_iter;i++){
			paramsJson[j] = cJSON_CreateObject();
			cJSON_AddStringToObject(paramsJson[j],"n",notifyParams[j].resourceUri);
			cJSON_AddStringToObject(paramsJson[j],"sv",notifyParams[j].stringValue);
			cJSON_AddStringToObject(paramsJson[j],"ti",notifyParams[j].time);
			cJSON_AddItemToArray(eArray,paramsJson[j]);
	        
			j++;                         
        }

	    cJSON_AddStringToObject(msg,"o","n");
		cJSON_AddItemToObject(msg,"e",eArray);
	
		cJSON_AddItemToObject(root,"msg",msg);
    
		//sendMsg = cJSON_Print(root); Origin CJSON print https://github.com/DaveGamble/cJSON/issues/323
		sendMsg = cJSON_PrintUnformatted(root);

	    if (sendMsg == NULL)
	    {
	        fprintf(stderr, "Failed to print monitor.\n");        
	        goto end;
	    }
	
	#ifdef DEBUG
         if(sendMsg != NULL)
		 {
    		printf("sendMsg : \n%s\n",sendMsg);
                            
         }
         else 
		 {
         	printf("sendMSg is NULL \n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");    
         }
	#endif
		esp_mqtt_client_publish(client, topic, sendMsg, 0, 1, 0);

		free(sendMsg);
		cJSON_Delete(root);

        iter_count -- ;                  
    }    
        
	return 1;
end:
    cJSON_Delete(root);  
    return -3;
}

int damda_Notify_DeviceStatuArr(char* notificationType,NotifyParameter notifyParams[],int arrayCount){
	
	char topic[200];
	char *sendMsg;
	char seqString[10];
	int i=0;
	const int one_unit = 5; //Noti Maximum number of iterations
	
	sprintf(topic,"%s/sync/%s/iot-server/notify/json",device_type,device_id);
    
	cJSON *root ;    
    cJSON *msg ;
    cJSON *eArray;
    cJSON *paramsJson[arrayCount]; 

	root = cJSON_CreateObject();
	msg = cJSON_CreateObject();
	eArray = cJSON_CreateArray();

	cJSON_AddStringToObject(root,"sid",notificationType);

	sequence = (sequence + 1) % MAX_SEQUENCE;
	sprintf(seqString,"%d",sequence);
	cJSON_AddStringToObject(root,"seq",seqString);

	cJSON_AddStringToObject(root,"did",device_id);
	
	for(i=0;i<arrayCount;i++){
		paramsJson[i] = cJSON_CreateObject();
		cJSON_AddStringToObject(paramsJson[i],"n",notifyParams[i].resourceUri);
		cJSON_AddStringToObject(paramsJson[i],"sv",notifyParams[i].stringValue);
		cJSON_AddStringToObject(paramsJson[i],"ti",notifyParams[i].time);
		cJSON_AddItemToArray(eArray,paramsJson[i]);		
	}

	cJSON_AddStringToObject(msg,"o","n");
	cJSON_AddItemToObject(msg,"e",eArray);

	cJSON_AddItemToObject(root,"msg",msg);

	//sendMsg = cJSON_Print(root); Origin CJSON print https://github.com/DaveGamble/cJSON/issues/323
	sendMsg = cJSON_PrintUnformatted(root);

	if (sendMsg == NULL)
	{
		fprintf(stderr, "Failed to print monitor.\n");        
		goto end;
	}

#ifdef DEBUG
		if(sendMsg != NULL)
		{
		// printf("sendMsg : \n%s\n",sendMsg);
						
		}
		else 
		{
		printf("sendMSg is NULL \n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");    
		}
#endif
	esp_mqtt_client_publish(client, topic, sendMsg, 0, 1, 0);

	free(sendMsg);
	cJSON_Delete(root);
	return 1;
end:
    cJSON_Delete(root);  
    return -3;
}


int damda_Rsp_RemoteControl(int controlType,const char* sid,const char* returnCode,char* resourceUri[],char* stringValue[],int arrayCount){

	char topic[200];
	char *sendMsg;
    char * temp_ti;
	int i=0;
	cJSON *root = cJSON_CreateObject();	
	cJSON *msg = cJSON_CreateObject();
	cJSON *eArray = cJSON_CreateArray();
	
	cJSON *paramsJson[10];
	
	if(controlType == 2){
	    sprintf(topic,"%s/sync/%s/iot-server/read/json",device_type,device_id);
    } else if(controlType == 3){
	    sprintf(topic,"%s/sync/%s/iot-server/write/json",device_type,device_id);
    } else if(controlType == 4){
       	sprintf(topic,"%s/sync/%s/iot-server/execute/json",device_type,device_id);
    } else {
	printf("[damda] Error : invalid controlType \n");
	return;
    }
    
    cJSON_AddStringToObject(root,"sid",sid);
	
	cJSON_AddStringToObject(root,"did",device_id);

    temp_ti = time_execute;
	
	for(i=0;i<arrayCount;i++){
		paramsJson[i] = cJSON_CreateObject();
		cJSON_AddStringToObject(paramsJson[i],"n",resourceUri[i]);
		cJSON_AddStringToObject(paramsJson[i],"sv",stringValue[i]);
        cJSON_AddStringToObject(paramsJson[i],"ti",temp_ti);
		cJSON_AddItemToArray(eArray,paramsJson[i]);
	}
	
	cJSON_AddStringToObject(msg,"o","res");
	cJSON_AddStringToObject(msg,"rc",returnCode);
	cJSON_AddItemToObject(msg,"e",eArray);
	
	cJSON_AddItemToObject(root,"msg",msg);
	
	sendMsg = cJSON_Print(root);
    if (sendMsg == NULL)
    {
        fprintf(stderr, "Failed to print monitor.\n");        
        goto end;
    }

	#ifdef DEBUG
		printf("sendMsg : \n%s\n",sendMsg);
	#endif


	esp_mqtt_client_publish(client, topic, sendMsg, 0, 1, 0);
	
	free(sendMsg);
	cJSON_Delete(root);
    
    return 1;
end:
  	cJSON_Delete(root);  
   	return -3;

}

int damda_Rsp_RemoteControl_noEntity(int controlType,const char* sid,const char* returnCode){
	char topic[200];
	char *sendMsg;

	cJSON *root = cJSON_CreateObject();	
	cJSON *msg = cJSON_CreateObject();
	
	
	
	if(controlType == 2){
	    sprintf(topic,"%s/sync/%s/iot-server/read/json",device_type,device_id);
    } else if(controlType == 3){
	    sprintf(topic,"%s/sync/%s/iot-server/write/json",device_type,device_id);
    } else if(controlType == 4){
       	sprintf(topic,"%s/sync/%s/iot-server/execute/json",device_type,device_id);
    } else {
	printf("[damda] Error : invalid controlType \n");
	return;
    }
    
    cJSON_AddStringToObject(root,"sid",sid);
	
	cJSON_AddStringToObject(root,"did",device_id);
	

	
	cJSON_AddStringToObject(msg,"o","res");
	cJSON_AddStringToObject(msg,"rc",returnCode);
	
	cJSON_AddItemToObject(root,"msg",msg);
	
	sendMsg = cJSON_Print(root);
    if (sendMsg == NULL)
    {
        fprintf(stderr, "Failed to print monitor.\n");        
        goto end;
    }
	#ifdef DEBUG
	printf("sendMsg : \n%s\n",sendMsg);
	#endif
	
	esp_mqtt_client_publish(client, topic, sendMsg, 0, 1, 0);
	
	free(sendMsg);
	cJSON_Delete(root);
    
	return 1;
end:
    cJSON_Delete(root);  
    return -3;
}

int damda_Rsp_RemoteDelDevice(const char* sid,const char* returnCode){
	char topic[200];
	char *sendMsg;

	cJSON *root = cJSON_CreateObject();	
	cJSON *msg = cJSON_CreateObject();
	
	
	
	sprintf(topic,"%s/sync/%s/iot-server/delete/json",device_type,device_id);
    
    cJSON_AddStringToObject(root,"sid",sid);
	
	cJSON_AddStringToObject(root,"did",device_id);
	
	cJSON_AddStringToObject(msg,"o","res");
	cJSON_AddStringToObject(msg,"rc",returnCode);
	
	cJSON_AddItemToObject(root,"msg",msg);
	
	sendMsg = cJSON_Print(root);
    if (sendMsg == NULL)
    {
        fprintf(stderr, "Failed to print monitor.\n");        
        goto end;
    }
	#ifdef DEBUG
		printf("sendMsg : \n%s\n",sendMsg);
	#endif
	
	esp_mqtt_client_publish(client, topic, sendMsg, 0, 1, 0);
	
	free(sendMsg);
	cJSON_Delete(root);
    
    return 1;
end:
    cJSON_Delete(root);  
    return -3;
}

int damda_Rsp_Client_Busy (int controlType,const char* sid,const char* returnCode,int arrayCount)
{
	char topic[200];
	char *sendMsg;
	int i=0;
	cJSON *root = cJSON_CreateObject();	
	cJSON *msg = cJSON_CreateObject();
	cJSON *eArray = cJSON_CreateArray();
	
	cJSON *paramsJson[10];
	
	if(controlType == 2)
	{
	    sprintf(topic,"%s/sync/%s/iot-server/read/json",device_type,device_id);
    } 
	else if(controlType == 3)
	{
	    sprintf(topic,"%s/sync/%s/iot-server/write/json",device_type,device_id);
    } 
	else if(controlType == 4)
	{
       	    sprintf(topic,"%s/sync/%s/iot-server/execute/json",device_type,device_id);
    } 
	else 
	{
		printf("[damda] Error : invalid controlType \n");
	return;
    }
    
    cJSON_AddStringToObject(root,"sid",sid);
	
	cJSON_AddStringToObject(root,"did",device_id);
	
	
	for(i=0;i<arrayCount;i++)
	{
		paramsJson[i] = cJSON_CreateObject();
		//cJSON_AddStringToObject(paramsJson[i],"n","/600/0/600");
		cJSON_AddStringToObject(paramsJson[i],"sv","busy");
		cJSON_AddItemToArray(eArray,paramsJson[i]);
	}
	
	cJSON_AddStringToObject(msg,"o","res");
	cJSON_AddStringToObject(msg,"rc",returnCode);
	cJSON_AddItemToObject(msg,"e",eArray);
	
	cJSON_AddItemToObject(root,"msg",msg);
	
	sendMsg = cJSON_Print(root);
    if (sendMsg == NULL)
    {
        fprintf(stderr, "Failed to print monitor.\n");        
        goto end;
    }
	#ifdef DEBUG
		printf("sendMsg : \n%s\n",sendMsg);
	#endif

	esp_mqtt_client_publish(client, topic, sendMsg, 0, 1, 0);
	
	free(sendMsg);
	cJSON_Delete(root);
    
    return 1;
end:
    cJSON_Delete(root);  
    return -3;   
}


void getSid(char* message,char* returnValue){
	cJSON *root = cJSON_Parse(message);
	char sid[100];
	cJSON *sidJson = cJSON_GetObjectItem(root,"sid");
	if(sidJson == NULL){
		cJSON_Delete(root);
		return;	
	}
    strcpy(sid,cJSON_GetStringValue(sidJson));
	#ifdef DEBUG
    	printf("in function sid : %s",sid);
	#endif
	cJSON_Delete(root);
	strcpy(returnValue,sid);
	
}

int getSeq(char* message){
	cJSON *root = cJSON_Parse(message);
	char* seqChar;
	int seq;
	cJSON *seqJson = cJSON_GetObjectItem(root,"seq");
	if(seqJson == NULL){
		cJSON_Delete(root);
		return -1;	
	}
	seqChar = cJSON_GetStringValue(seqJson);
	seq = atoi(seqChar);
	cJSON_Delete(root);
	
	return seq;
	
}

void getOperation(char* message,char* returnValue){
    cJSON *root = cJSON_Parse(message);
    
    char operation[10];
    cJSON *msgJson = cJSON_GetObjectItem(root,"msg");
    cJSON *oJson = cJSON_GetObjectItem(msgJson,"o");
    if(oJson == NULL){
        cJSON_Delete(root);
        return;     
    }
    
    strcpy(operation,cJSON_GetStringValue(oJson));

    cJSON_Delete(root);
   strcpy(returnValue,operation);
    
}

int getReturnCode(char* message){
	char* rcChar;
	int rc;
	cJSON *root = cJSON_Parse(message);	
	cJSON *msgJson = cJSON_GetObjectItem(root,"msg");
	cJSON *rcJson = cJSON_GetObjectItem(msgJson,"rc");
	if(rcJson == NULL){
		cJSON_Delete(root);
		return -1;	
	}
	rcChar = cJSON_GetStringValue(rcJson);
	rc = atoi(rcChar);
	cJSON_Delete(root);
	return rc;
}

int getErrorCode(char* message){
	char* ecChar;
	int ec;
	cJSON *root = cJSON_Parse(message);	
	cJSON *msgJson = cJSON_GetObjectItem(root,"msg");
	cJSON *ecJson = cJSON_GetObjectItem(msgJson,"ec");
	if(ecJson == NULL){
		cJSON_Delete(root);
		return 0;	
	}
	ecChar = cJSON_GetStringValue(ecJson);
	ec = atoi(ecChar);
	cJSON_Delete(root);
	return ec;
}

void getResourceUri(char* message,int index,char* returnValue){
	
	cJSON *root = cJSON_Parse(message);
	cJSON *msgJson = cJSON_GetObjectItem(root,"msg");
	cJSON *eArray = cJSON_GetObjectItem(msgJson,"e");
	
	cJSON *eJson = cJSON_GetArrayItem(eArray,index);
	cJSON *n = cJSON_GetObjectItem(eJson,"n");
	if(n == NULL){
		cJSON_Delete(root);
		return ;	
	}
	
	strcpy(returnValue,cJSON_GetStringValue(n));
	cJSON_Delete(root);
	
	
}

int getResourceUriLength(char* message,int index){
	char* resourceUri;
	cJSON *root = cJSON_Parse(message);
	cJSON *msgJson = cJSON_GetObjectItem(root,"msg");
	cJSON *eArray = cJSON_GetObjectItem(msgJson,"e");
	
	cJSON *eJson = cJSON_GetArrayItem(eArray,index);
	cJSON *n = cJSON_GetObjectItem(eJson,"n");
	if(n == NULL){
		cJSON_Delete(root);
		return 0;	
	}
	resourceUri = cJSON_GetStringValue(n);
        
	cJSON_Delete(root);
	
	return strlen(resourceUri);
}


int getNArraySize(char* message){
	int jsonArraySize;
	
	cJSON *root = cJSON_Parse(message);
	cJSON *msgJson = cJSON_GetObjectItem(root,"msg");
	cJSON *eArray = cJSON_GetObjectItem(msgJson,"e");
	
	if(eArray == NULL){
		cJSON_Delete(root);
		return 0;	
	}
	
	jsonArraySize = cJSON_GetArraySize(eArray);
	
	cJSON_Delete(root);
	return jsonArraySize;
}

void getStringValue(const char* message, int index, char* returnValue, int returnValueSize) {
    // JSON 파싱을 시도
    cJSON *root = cJSON_Parse(message);
    if (root == NULL) {
        printf("Failed to parse JSON\n");
        return;
    }

    // "msg" 오브젝트 가져오기
    cJSON *msgJson = cJSON_GetObjectItem(root, "msg");
    if (msgJson == NULL) {
        printf("Failed to get 'msg' object\n");
        cJSON_Delete(root);
        return;
    }

    // "e" 배열 가져오기
    cJSON *eArray = cJSON_GetObjectItem(msgJson, "e");
    if (eArray == NULL) {
        printf("Failed to get 'e' array\n");
        cJSON_Delete(root);
        return;
    }

    // 배열의 인덱스에 해당하는 아이템 가져오기
    cJSON *eJson = cJSON_GetArrayItem(eArray, index);
    if (eJson == NULL) {
        printf("Failed to get array item at index %d\n", index);
        cJSON_Delete(root);
        return;
    }

    // "sv" 오브젝트 가져오기
    cJSON *sv = cJSON_GetObjectItem(eJson, "sv");
    if (sv == NULL) {
        printf("Failed to get 'sv' object\n");
        cJSON_Delete(root);
        return;
    }

    // 문자열 값 가져오기
    const char *svStr = cJSON_GetStringValue(sv);
    if (svStr != NULL) {
        strncpy(returnValue, svStr, returnValueSize - 1);
        returnValue[returnValueSize - 1] = '\0'; // 문자열 끝에 NULL 문자 추가
    } else {
        printf("Failed to get string value from 'sv' object\n");
    }

    // JSON 오브젝트 해제
    cJSON_Delete(root);
}


int getStringValueLength(char* message,int index){
	char* stringValue;
	cJSON *root = cJSON_Parse(message);
	cJSON *msgJson = cJSON_GetObjectItem(root,"msg");
	cJSON *eArray = cJSON_GetObjectItem(msgJson,"e");
	
	cJSON *eJson = cJSON_GetArrayItem(eArray,index);
	cJSON *sv = cJSON_GetObjectItem(eJson,"sv");
	
	if(sv == NULL){
		cJSON_Delete(root);
		return 0;
	}
	
	stringValue = cJSON_GetStringValue(sv);
	cJSON_Delete(root);
	
	return strlen(stringValue);
}


void getTime(char* message, int index, char* returnValue) {
    cJSON *root = cJSON_Parse(message);
    if (root == NULL) {
        printf("Failed to parse JSON\n");
        return;
    }

    cJSON *msgJson = cJSON_GetObjectItem(root, "msg");
    if (msgJson == NULL) {
        printf("Failed to get 'msg' object\n");
        cJSON_Delete(root);
        return;
    }

    cJSON *eArray = cJSON_GetObjectItem(msgJson, "e");
    if (eArray == NULL) {
        printf("Failed to get 'e' array\n");
        cJSON_Delete(root);
        return;
    }

    cJSON *eJson = cJSON_GetArrayItem(eArray, index);
    if (eJson == NULL) {
        printf("Failed to get array item at index %d\n", index);
        cJSON_Delete(root);
        return;
    }

    cJSON *ti = cJSON_GetObjectItem(eJson, "ti");
    if (ti == NULL) {
        printf("Failed to get 'ti' object\n");
        cJSON_Delete(root);
        return;
    }

    const char *tiStr = cJSON_GetStringValue(ti);
    if (tiStr != NULL) {
        strcpy(returnValue, tiStr);
    } else {
        printf("Failed to get string value from 'ti' object\n");
    }

    cJSON_Delete(root);  // 메모리 해제
}


int getTimeLength(const char* message, int index) {
    int timeLength = 0;
    cJSON *root = cJSON_Parse(message);
    if (root == NULL) {
        printf("Failed to parse JSON\n");
        return 0;
    }

    cJSON *msgJson = cJSON_GetObjectItem(root, "msg");
    if (msgJson == NULL) {
        printf("Failed to get 'msg' object\n");
        cJSON_Delete(root);
        return 0;
    }

    cJSON *eArray = cJSON_GetObjectItem(msgJson, "e");
    if (eArray == NULL) {
        printf("Failed to get 'e' array\n");
        cJSON_Delete(root);
        return 0;
    }

    cJSON *eJson = cJSON_GetArrayItem(eArray, index);
    if (eJson == NULL) {
        printf("Failed to get array item at index %d\n", index);
        cJSON_Delete(root);
        return 0;
    }

    cJSON *ti = cJSON_GetObjectItem(eJson, "ti");
    if (ti != NULL) {
        const char* timeStr = cJSON_GetStringValue(ti);
        if (timeStr != NULL) {
            timeLength = strlen(timeStr);
        }
    }

    cJSON_Delete(root);
    return timeLength;
}


void getServiceId(char* topic,char* returnValue){
	char tempTopicService[100];
	strcpy(tempTopicService,topic);
	char* ptrServiceId = strtok(tempTopicService,"/");
	strcpy(returnValue,ptrServiceId);
	
}

void getMsgType(char* topic,char* returnValue){
	int i=0;
	char tempTopicMsgType[100];
	strcpy(tempTopicMsgType,topic);
	char* ptrMsgType = strtok(tempTopicMsgType,"/");
	while(ptrMsgType!=NULL){
		
		ptrMsgType=strtok(NULL,"/");
		if(i==0){
			break;
		}
		i++;
	}
	strcpy(returnValue,ptrMsgType);
	
}

void getOriginId(char* topic,char* returnValue){
        int i=0;
        char tempTopicOriginId[100];
        strcpy(tempTopicOriginId,topic);
        char* ptrOriginId = strtok(tempTopicOriginId,"/");
        while(ptrOriginId!=NULL){

                ptrOriginId=strtok(NULL,"/");
                if(i==1){
                        break;
                }
                i++;
        }
		strcpy(returnValue,ptrOriginId);
        
}

void getTargetId(char* topic,char* returnValue){
        int i=0;
        char tempTopicTargetId[100];
        strcpy(tempTopicTargetId,topic);
        char* ptrTargetId = strtok(tempTopicTargetId,"/");
        while(ptrTargetId!=NULL){

                ptrTargetId=strtok(NULL,"/");
                if(i==2){
                        break;
                }
                i++;
        }
		strcpy(returnValue,ptrTargetId);
        
}

void getTopicOprId(char* topic,char* returnValue){
        int i=0;
        char tempTopicOperationId[100];
        strcpy(tempTopicOperationId,topic);
        char* ptrOperationId = strtok(tempTopicOperationId,"/");
        while(ptrOperationId!=NULL){

                ptrOperationId=strtok(NULL,"/");
                if(i==3){
                   break;
                }
                i++;
        }
        strcpy(returnValue,ptrOperationId);

}


int getMessageType(char*topic,char *message){
	char tempMsgType[20];
	getMsgType(topic,tempMsgType);
	
     char operation[10];
     getOperation(message,operation);
         printf("message확인!!!!! : \n%s\n",message);
	if(isFirmwareUpdateMessage(message)){
		return MESSAGE_TYPE_FIRMWARE_DOWNLOAD;
	}
	else if(strcmp(operation,"res")==0){
		return MESSAGE_TYPE_RESPONSE;
	}
	else if(strcmp(operation,"r")==0){
		if(strcmp(tempMsgType,"sync")==0){
			return MESSAGE_TYPE_READ;
		} else if(strcmp(tempMsgType,"async")==0){
			return MESSAGE_TYPE_READ_ASYNC;
		}
    }
    else if(strcmp(operation,"w")==0){
		if(strcmp(tempMsgType,"sync")==0){
			return MESSAGE_TYPE_WRITE;
		} else if(strcmp(tempMsgType,"async")==0){
			return MESSAGE_TYPE_WRITE_ASYNC;
		}
		
	}
	else if(strcmp(operation,"e")==0){
		if(strcmp(tempMsgType,"sync")==0){
			return MESSAGE_TYPE_EXECUTE;
		} 
		else if(strcmp(tempMsgType,"async")==0){
			return MESSAGE_TYPE_EXECUTE_ASYNC;
		}
    }
    else if(strcmp(operation,"d")==0 && strcmp(tempMsgType,"sync") == 0){
		return MESSAGE_TYPE_DELETE_SYNC;
    }
    else if(strcmp(operation,"d")==0 && strcmp(tempMsgType,"async") == 0){
		return MESSAGE_TYPE_DELETE_ASYNC;
	}

	return -1;

}

int isFirmwareUpdateMessage(char* message){
	
	cJSON *root = cJSON_Parse(message);
	cJSON *msgJson = cJSON_GetObjectItem(root,"msg");
	cJSON *eArray = cJSON_GetObjectItem(msgJson,"e");
	cJSON *eJsonObject;
	int jsonArraySize = cJSON_GetArraySize(eArray);
	int i=0;
	int updateNCount=0;
	char nValue[80];
	if(jsonArraySize==0){
		cJSON_Delete(root);
		return 0;
	}
	for(i=0;i<jsonArraySize;i++){
		eJsonObject = cJSON_GetArrayItem(eArray,i);
		strcpy(nValue,cJSON_GetStringValue(cJSON_GetObjectItem(eJsonObject,"n")));
		if(strcmp(nValue,"/3/0/3")==0 || strcmp(nValue,"/5/0/1")==0){
			updateNCount++;
		}
	}
	cJSON_Delete(root);
	if(updateNCount==2){
		return 1;
	} else {
		return 0;
	}

	

}

void  getStringValueByResourceUri(char* message,char* n, char* returnValue){

	cJSON *root = cJSON_Parse(message);
	cJSON *msgJson = cJSON_GetObjectItem(root,"msg");
	cJSON *eArray = cJSON_GetObjectItem(msgJson,"e");
	cJSON *eJsonObject;
	int jsonArraySize = cJSON_GetArraySize(eArray);
	int i=0;
	char nValue[15];
	if(jsonArraySize==0){
		cJSON_Delete(root);
        return ;
	}
	for(i=0;i<jsonArraySize;i++){
		eJsonObject = cJSON_GetArrayItem(eArray,i);
		strcpy(nValue,cJSON_GetStringValue(cJSON_GetObjectItem(eJsonObject,"n")));
		if(strcmp(nValue,n)==0){
			char* sv = (char*)cJSON_GetStringValue(cJSON_GetObjectItem(eJsonObject,"sv"));
			if(sv!=NULL){
				strcpy(returnValue,sv);
                cJSON_Delete(root);
                return ;
			} else {
				cJSON_Delete(root);
                return ;
			}
		}
	}
	cJSON_Delete(root);
}
