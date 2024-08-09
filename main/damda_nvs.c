
#include "damda_ESP32_Project.h"

flash_info_t factory_flash_info =
{
    .booting_mode                       = INIT_MODE,
    .esp_mac 							= "",
    .ver_string    						= "1.0", //TODO 1.0 -> 1.1 CHAGED JAEEUN
    .ota_mode_fail                      = false,
    .igain								= 0,
    .vgain								= 0,
    .sgain								= 0,

};

void damda_nvs_init(void)
{
    esp_err_t ret = nvs_flash_init();
    
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    
	ESP_ERROR_CHECK(ret);

    read_flash(&flash_info);
   //strcpy(flash_info.ver_string, VER_STRING);
}

void write_flash(flash_info_t *pdata)
{
    nvs_handle handle;
    esp_err_t err;

    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    
	if (err != ESP_OK)
    {
         printf("Opening NVS handle Error! (%x)\n", err);
        return;
    }

    err = nvs_set_blob(handle, "app_data_info", pdata, sizeof(flash_info_t));
    
	if (err != ESP_OK)
    {
        printf("Updating flash data Error! (%x)\n", err);
        nvs_close(handle);
        return;
    }

    err = nvs_commit(handle);
    if (err != ESP_OK)
	{
         printf("Committing updates in NVS Error! (%x)\n", err);
	}
    nvs_close(handle);
}

void reset_flash(flash_info_t* pdata)
{
    factory_flash_info.igain = pdata->igain;
    factory_flash_info.vgain = pdata->vgain;
    factory_flash_info.sgain = pdata->sgain;
	memcpy(pdata, &factory_flash_info, sizeof(flash_info_t));
}

void read_flash(flash_info_t *pdata)
{
    nvs_handle handle;
    esp_err_t err;

    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK)
    {
        return;
    }

    size_t required_size = sizeof(flash_info_t);
    err = nvs_get_blob(handle, "app_data_info", pdata, &required_size);

    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
    {
        nvs_close(handle);
        return;
    }

    switch (err)
    {
	    case ESP_OK:
	        break;

	    case ESP_ERR_NVS_NOT_FOUND:
	        reset_flash(pdata);
	        pdata->booting_mode = FACTORY_MODE;
	        write_flash(pdata);
	        break;
    }
    nvs_close(handle);
}

