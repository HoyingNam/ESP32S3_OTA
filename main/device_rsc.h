
//###########################################################################
//
// FILE:   device_rsc.h
//
// TITLE:  DAMDA Device Resource Data Define File and Global !!!
//
//###########################################################################


#define DEVICE_MANUFACTURER 		"WINIAELECTRONICS"
#define DEVICE_MODEL 				"WN_ULTFREEZER"
#define DEVICE_TYPE 				"FREEZER"

/* ---ESP32 Device resources profile (For "DDSensor4_TestSensor" & test only)---- */
#define POWER                 			      "/4001/2/4016"
char power_value_0[] = "OFF";           //char* temperature_value_0 = " 40";
#define TEMPERATURE          		    	    "/4800/2/4802" 
char temperature_value_0[] = "30.00";
#define TEMPERATURESETTINGS                    "/4800/2/4812"
char temperaturesetting_value_0[] = " 20";
#define TEMPERATURESETTINGSMODE                 "/4800/2/4813"
char temperaturesettingmode_value_0[] = "1";

#define SALESTARGET                 			"/5404/2/6006"
char salestarget_value_0[] = "1";
#define LOCKSTATE          			"/3201/2/3206"
char lockstate_value_0[] = "1";
#define TEMPERUNIT          			"/4800/2/5000"
char temperunit_value_0[] = "1";
#define OPENSTATE                      		    "/1603/2/3803"
char openstate_value_0[] = "1";
#define ERRORCODE                       			"/3802/2/3421"
char errorcode_value_0[] = "8";
#define ALARMSTATUS                       			"/1603/2/3810"
char alarmstaus_value_0[] = "OFF";
#define VERSION                       			"/8000/2/1630"
char version_value_0[] = "1";

#define RSC_OID 		"oid"
#define RSC_RID 		"rid"
#define RSC_COUNT 		"count"
#define RSC_TYPE 		"type"
#define RSC_RANGE 		"range"

//char *deviceRscData = "[{\"oid\":\"4001\",\"rid\":\"4016\",\"count\":\"1\",\"type\":\"B\",\"range\":\"\"},{\"oid\":\"4800\",\"rid\":\"4802\",\"count\":\"1\",\"type\":\"I\",\"range\":\"-100~100\"},{\"oid\":\"4800\",\"rid\":\"4813\",\"count\":\"1\",\"type\":\"I\",\"range\":\"1~4\"},{\"oid\":\"5404\",\"rid\":\"6006\",\"count\":\"1\",\"type\":\"I\",\"range\":\"1~3\"},{\"oid\":\"3201\",\"rid\":\"3206\",\"count\":\"1\",\"type\":\"B\",\"range\":\"\"},{\"oid\":\"4800\",\"rid\":\"5000\",\"count\":\"1\",\"type\":\"B\",\"range\":\"\"},{\"oid\":\"1603\",\"rid\":\"3803\",\"count\":\"1\",\"type\":\"B\",\"range\":\"\"},{\"oid\":\"3802\",\"rid\":\"3421\",\"count\":\"1\",\"type\":\"I\",\"range\":\"0~8\"},{\"oid\":\"1603\",\"rid\":\"3810\",\"count\":\"1\",\"type\":\"B\",\"range\":\"\"},{\"oid\":\"8000\",\"rid\":\"1630\",\"count\":\"1\",\"type\":\"S\",\"range\":\"\"}]";
char *deviceRscData = "[{\"oid\":\"4001\",\"rid\":\"4016\",\"count\":\"1\",\"type\":\"B\",\"range\":\"\"},{\"oid\":\"4800\",\"rid\":\"4802\",\"count\":\"1\",\"type\":\"I\",\"range\":\"-100~100\"},{\"oid\":\"4800\",\"rid\":\"4812\",\"count\":\"1\",\"type\":\"I\",\"range\":\"-100~100\"},{\"oid\":\"4800\",\"rid\":\"4813\",\"count\":\"1\",\"type\":\"I\",\"range\":\"1~4\"},{\"oid\":\"5404\",\"rid\":\"6006\",\"count\":\"1\",\"type\":\"I\",\"range\":\"1~3\"},{\"oid\":\"3201\",\"rid\":\"3206\",\"count\":\"1\",\"type\":\"B\",\"range\":\"\"},{\"oid\":\"4800\",\"rid\":\"5000\",\"count\":\"1\",\"type\":\"B\",\"range\":\"\"},{\"oid\":\"1603\",\"rid\":\"3803\",\"count\":\"1\",\"type\":\"B\",\"range\":\"\"},{\"oid\":\"3802\",\"rid\":\"3421\",\"count\":\"1\",\"type\":\"I\",\"range\":\"0~8\"},{\"oid\":\"1603\",\"rid\":\"3810\",\"count\":\"1\",\"type\":\"B\",\"range\":\"\"},{\"oid\":\"8000\",\"rid\":\"1630\",\"count\":\"1\",\"type\":\"S\",\"range\":\"\"}]";
#define NOTI_PARAMS_TOTAL 		11
