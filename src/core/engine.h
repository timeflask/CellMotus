#ifndef __engine_H_
#define __engine_H_
#include "config.h"

void         sen_init(float w, float h);
void         sen_destroy();
void         sen_process();
float        sen_fps();
void         sen_reload();
void         sen_exit();


#define SEN_STATUS_DEAD        (0)
#define SEN_STATUS_INITIALIZED (1<<0)
#define SEN_STATUS_STOPPED     (1<<1)

int          sen_status();

#endif
