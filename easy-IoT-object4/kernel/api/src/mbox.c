#include "api.h"
#include "debug.h"
#include "mbox.h"

void mbox_new(mbox_t *mbox, void *pmsg)
{
#ifdef OS_UCOS
#ifdef UCOS_V3
    return;
#else
	OS_EVENT *event;

	event = OSMboxCreate(pmsg);
	if (event == 0)
		p_err("mbox_new err");

	*mbox = (mbox_t)event;
#endif

#else
    return;
#endif
}

void *mbox_get(mbox_t *mbox, unsigned int timeout)
{
#ifdef OS_UCOS
#ifdef UCOS_V3
		return 0;
#else
	INT8U err;
	void *msg;

	if ((timeout > 0) && (timeout < 1000uL / OS_TICKS_PER_SEC))
		timeout = 1000uL / OS_TICKS_PER_SEC;

	timeout = timeout * OS_TICKS_PER_SEC / 1000uL;

	msg = OSMboxPend((OS_EVENT*) * mbox, timeout, &err);
	if (err != OS_ERR_NONE)
	{
		p_err("mbox_get err %d", err);
		return 0;
	}

	return msg;
#endif
	
#else
	return 0;
#endif

}

int mbox_post(mbox_t *mbox, void *pmsg)
{
#ifdef OS_UCOS
#ifdef UCOS_V3
	return 0;
#else

	INT8U perr;
	perr = OSMboxPost((OS_EVENT*) * mbox, pmsg);
	if (perr == OS_ERR_NONE)
		return 0;

	p_err("mbox_post err %d", perr);
	return  - 1;
#endif
		
#else
	return 0;
#endif

}

int mbox_destory(mbox_t *mbox)
{
#ifdef OS_UCOS
#ifdef UCOS_V3
	return 0;
#else

	INT8U perr;

	OSMboxDel((OS_EVENT*) * mbox, OS_DEL_ALWAYS, &perr);
	if (perr == OS_ERR_NONE)
		return 0;

	p_err("mbox_destory err %d", perr);
	return  - 1;
#endif
			
#else
	return 0;
#endif

}
