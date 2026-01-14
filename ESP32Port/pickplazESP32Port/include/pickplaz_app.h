#ifndef PICKPLAZ_APP_H_
#define PICKPLAZ_APP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hal.h"

hal_status_t pickplaz_app_init(void);
hal_status_t pickplaz_app_start(void);
void pickplaz_app_stop(void);

#ifdef __cplusplus
}
#endif

#endif
