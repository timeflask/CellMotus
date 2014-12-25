#ifndef __engine_H_
#define __engine_H_
#include "config.h"

SEN_DECL void  sen_init(float w, float h);
SEN_DECL void  sen_destroy();
SEN_DECL void  sen_process(); // update+draw
SEN_DECL void  sen_process_update();
SEN_DECL void  sen_process_draw();
SEN_DECL float sen_fps();
SEN_DECL void  sen_reload();
SEN_DECL void  sen_exit();


#define SEN_STATUS_DEAD        (0)
#define SEN_STATUS_INITIALIZED (1<<0)
#define SEN_STATUS_STOPPED     (1<<1)

int          sen_status();

void sen_screenshot();

#endif
