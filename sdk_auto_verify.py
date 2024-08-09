import csv
import os
import sys

help_cmd = ['-h', '-H', '-Help', '-help']
use_info_file_cmd = ['-m', '-model', '-M', '-Model']
folder_name = ''
dvc_manufacture = ''
dvc_type = ''
dvc_model = ''
cmd_mode = 0
argument = sys.argv

if len(sys.argv) == 2:
    if argument[1] in help_cmd:
        print("\n")
        print(" ===========================================================================================================")
        print(" |                                                                                                         |")
        print(" |  " + '\033[99m' + "ESP32_flash_start_for_dummy.py" +  '\033[0m' + "                                                                         |")
        print(" |                                                                                                         |")
        print(" |  [" + '\033[59m' + " Instruction " + '\033[0m' +"]                                                                                        |")
        print(" |                                                                                                         |")
        print(" |  The Firmware for verification transfer data by MQTT is flashed an ESP32 Device.                        |")
        print(" |                                                                                                         |")
        print(" |   - To excuting this verification tool, you can use below :                                             |")
        print(" |       $ python ESP32_flash_start_for_dummy.py [MANUFACTURER_NAME] [DEVICE_TYPE] [MODEL_NAME]            |")
        print(" |                                                                                                         |")
        print(" |   - you can also use another way like below :                                                           |")
        print(" |       $ python ESP32_flash_start_for_dummy.py                                                           |")        
        print(" |         input a manufacture :  [MANUFACTURER_NAME]                                                      |")
        print(" |         input a model type  :  [DEVICE_TYPE]                                                            |")
        print(" |         input a model name  :  [MODEL_NAME]                                                             |")
        print(" |                                                                                                         |")
        print(" ===========================================================================================================\n")
    else :
        print("'" + argument[1] + "' is wrong option.")
        print("please, use '-h' option for the information.")
    exit(1)
elif len(sys.argv) == 3 :
    if argument[1] in use_info_file_cmd :
        cmd_mode = 3
        folder_name = argument[2]
        
        if not folder_name :
            print("'" + folder_name + "' is wrong folder.")
            exit(1)

        devInfoCsv = open('./resource_profile/' + folder_name + '/device_info.csv')
        devInfoCsv.seek(0)
        devReader = csv.DictReader(devInfoCsv)        
        for info in devReader:       
            dvc_manufacture = info['제조사']
            dvc_type = info['제품유형']
            dvc_model = info['모델명'] 

        if (not dvc_manufacture) or (not dvc_type) or (not dvc_model):
            print("get fail " + folder_name + " device info.....")
            print("please, use '-h' option for the information.")
            exit(1)
        
    else :
        print("'" + argument[1] + "' is wrong option.")
        print("please, use '-h' option for the information.")
        exit(1)
elif len(sys.argv) == 1 :
    cmd_mode = 1
    dvc_manufacture = input("input a manufacture : ")
    dvc_type = input("input a model type  : ")
    dvc_model = input("input a model name  : ")    
elif len(sys.argv) >= 4:
    cmd_mode = 2
    dvc_manufacture = argument[1]
    dvc_type = argument[2]
    dvc_model = argument[3]    
else :
    exit(1)

rscH = open('./main/device_rsc.h', 'w')

print("DEVICE_MANUFACTURER : " + dvc_manufacture)
print("DEVICE_TYPE         : " + dvc_type)
print("DEVICE_MODEL        : " + dvc_model)


rscH.write("\n")
rscH.write("//###########################################################################\n")
rscH.write("//\n")
rscH.write("// FILE:   device_rsc.h\n")
rscH.write("//\n")
rscH.write("// TITLE:  DAMDA Device Resource Data Define File and Global !!!\n")
rscH.write("//\n")
rscH.write("//###########################################################################\n")
rscH.write("\n\n")

rscH.write("#define DEVICE_MANUFACTURER 		\"" + dvc_manufacture + "\"\n")
rscH.write("#define DEVICE_MODEL 				\"" + dvc_model + "\"\n")
rscH.write("#define DEVICE_TYPE 				\"" + dvc_type + "\"\n")

rscH.write("\n")

rscH.write("#define RSC_OID 		\"oid\"\n")
rscH.write("#define RSC_RID 		\"rid\"\n")
rscH.write("#define RSC_COUNT 		\"count\"\n")
rscH.write("#define RSC_TYPE 		\"type\"\n")
rscH.write("#define RSC_RANGE 		\"range\"\n")

rscH.write("\n")

rscH.write("char *deviceRscData = \"[")

rscCsv = open('./resource_profile/' + dvc_model + '/resource_upload_sample.csv')
reader = csv.DictReader(rscCsv)

line_size = 0
for line in reader:
    line_size += 1

rscCsv.seek(0)
reader = csv.DictReader(rscCsv)
line_cnt = 0
instance_cnt = 0
for line in reader:
    print(line['객체 ID'], line['리소스 ID'], line['인스턴스 개수'], line['자료형'], line['범위'])
    rscH.write("{")
    rscH.write("\\\"oid\\\":")
    rscH.write("\\\"" + line['객체 ID'] + "\\\",")
    rscH.write("\\\"rid\\\":")
    rscH.write("\\\"" + line['리소스 ID'] + "\\\",")
    rscH.write("\\\"count\\\":")
    rscH.write("\\\"" + line['인스턴스 개수'] + "\\\",")
    rscH.write("\\\"type\\\":")
    rscH.write("\\\"" + line['자료형'] + "\\\",")
    rscH.write("\\\"range\\\":")
    rscH.write("\\\"" + line['범위'] + "\\\"")
    rscH.write("}")
    instance_cnt += int(line['인스턴스 개수'])

    if line_cnt < (line_size-1):
        rscH.write(",")
    print("cnt " + str(line_cnt) + " / " + str(line_size))
    line_cnt += 1

rscH.write("]\";\n")

rscH.write("\n")

print("instance_cnt : " + str(instance_cnt))
rscH.write("#define NOTI_PARAMS_TOTAL 		" + str(instance_cnt) + "\n")

rscCsv.close()
rscH.close()

os.system('export IDF_PATH=/d/workspace/ESPTest/esp/esp-idf')
os.system('export PATH=/d/workspace/ESPTest/xtensa-esp32-elf/xtensa-esp32-elf/bin')

print("starting flash ......")
os.system('idf.py flash monitor')