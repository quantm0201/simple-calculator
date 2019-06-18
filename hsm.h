
#ifndef hsm_h
#define hsm_h
typedef unsigned int State;

typedef struct Event {
unsigned int signal;
} Event;

typedef State (*StateHandler) (void *me, Event const *e);

typedef struct Hsm {
    StateHandler state;
} Hsm;

#define Hsm_ctor(me, initial) ((me)->state = (initial))
void Hsm_init(Hsm *me, Event const *e);
void Hsm_dispatch(Hsm *me, Event const* e);
State Hsm_top(Hsm *me, Event const *e);

#define HANDLED() ((State)0)
#define IGNORED() ((State)1)
#define TRANS(target) ((((Hsm *)me)->state = (StateHandler)target), (State)2)
#define SUPER(super) ((((Hsm *)me)->state = (StateHandler)super), (State)3)
#define UNHANDLED() ((State)4)

#endif
