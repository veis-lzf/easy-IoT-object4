#ifndef __EDP_DEMO_H__
#define __EDP_DEMO_H__

#include "lwip/opt.h"

#if LWIP_SOCKET
#if ONENET_ENABLE
void init_onenet_edpclient(void);
#endif
#endif /* LWIP_SOCKET */


#endif /*__EDP_DEMO_H__*/
