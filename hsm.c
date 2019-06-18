#include "hsm.h"

void Hsm_init(Hsm *me, Event const *e)
{
	me->state = (StateHandler)&Hsm_top;
}

void Hsm_dispatch(Hsm *me, Event const* e)
{
	StateHandler temp;
	StateHandler s;
	State ret;
	
	temp = me->state;
	
	do
	{
		s = me->state;
        ret = (*s) (me, e);
    } while (ret == (State)3);	// return SUPER

    if (ret == (State)2)          // return TRANS
	{
        temp = me->state;
	}
    me->state = temp;           // update state
}

State Hsm_top(Hsm *me, Event const *e)
{
	return IGNORED();
}
