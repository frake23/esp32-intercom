#ifndef CAM_H
#define CAM_H

#include "driver/ledc.h"
#include "driver/i2c_master.h"
#include "esp_camera.h"
#include "esp_log.h"

esp_err_t camera_init();

#endif // CAM_H
