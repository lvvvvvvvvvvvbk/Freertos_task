#ifndef APP_TASKS_H
#define APP_TASKS_H

#include "app_config.h"

void app_tasks_init(void);

#if !APP_USE_FREERTOS
void app_tasks_run_once(void);
#endif

#endif
