/*****************************************************************
 * Linux verion of Simple Calculator using HSMs implementation
 * technique, based on Hierarchical Event Processor Implementation
 * guide (PSiCC2 book chapter 4, state-machine.com).
 *
 * My Simple Calculator
 * @QuanTM
 ****************************************************************/

#include "hsm.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/ioctl.h>
#include <termios.h>

/********* use to enter a char instantly from keyboard **********/
void enable_raw_mode()
{
    struct termios term;
    tcgetattr(0, &term);
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &term);
}

void disable_raw_mode()
{
    struct termios term;
    tcgetattr(0, &term);
    term.c_lflag |= ICANON | ECHO;
    tcsetattr(0, TCSANOW, &term);
}
/****************************************************************/

enum CalcSignals {
    C_SIG,
    DIGIT_0_SIG,
    DIGIT_1_9_SIG,
    POINT_SIG,
    OPER_SIG,
    EQUAL_SIG,
    OFF_SIG,
    NONE
};

typedef struct CalcEvent {
    Event super;
    char keyCode;
} CalcEvent;

typedef struct Calc {
    Hsm super;
    double operand1;
    char operator;
} Calc;

void Calc_ctor();
static State Calc_initial  (Calc *me, Event const *e);/*init. pseudostate */
static State Calc_on       (Calc *me, Event const *e);   /* state handler */
static State Calc_error    (Calc *me, Event const *e);   /* state handler */
static State Calc_ready    (Calc *me, Event const *e);   /* state handler */
static State Calc_result   (Calc *me, Event const *e);   /* state handler */
static State Calc_negated1 (Calc *me, Event const *e);   /* state handler */
static State Calc_operand1 (Calc *me, Event const *e);   /* state handler */
static State Calc_zero1    (Calc *me, Event const *e);   /* state handler */
static State Calc_int1     (Calc *me, Event const *e);   /* state handler */
static State Calc_frac1    (Calc *me, Event const *e);   /* state handler */
static State Calc_opEntered(Calc *me, Event const *e);   /* state handler */
static State Calc_negated2 (Calc *me, Event const *e);   /* state handler */
static State Calc_operand2 (Calc *me, Event const *e);   /* state handler */
static State Calc_zero2    (Calc *me, Event const *e);   /* state handler */
static State Calc_int2     (Calc *me, Event const *e);   /* state handler */
static State Calc_frac2    (Calc *me, Event const *e);   /* state handler */
static State Calc_off      (Calc *me, Event const *e);   /* state handler */

static char buffer[100];          // a buffer use to store value of operands
static unsigned int count = 0;

void insert(char ch)                              // append a char to buffer
{
    buffer[count++] = ch;
}

double getValue()               // get the value of buffer then empty buffer
{
    double result = 0.0;
    unsigned int dot = count;
    for (int i = 0; i < count; i++)
        if (buffer[i] == '.') dot = i;
    for (int i = dot + 1; i < count; i++)
        result += (((int)buffer[i]) - 48) / pow(10.0, i - dot);
    if (buffer[0] == '-')
    {
        for (int i = 1; i < dot; i++)
            result += (((int)buffer[i]) - 48) * pow(10.0, dot - 1 - i);
        result *= -1;
    }
    else
    {
        for (int i = 0; i < dot; i++)
            result += (((int)buffer[i]) - 48) * pow(10.0, dot - 1 - i);
    }
    count = 0;
    return result;
}

static Calc l_calc;                              // the calculator instance

void Calc_ctor()
{
    printf("\nWelcome to my simple calculator..."
           "\nPress any button...");
    Calc *me = &l_calc;
    Hsm_ctor(&me->super, (StateHandler)&Calc_initial);
}

State Calc_initial(Calc *me, Event const *e)
{
    printf("\nInitialized My Calculator..."
           "\nPress 'e' to start");
    return TRANS(&Calc_on);
}

State Calc_on(Calc *me, Event const *e)
{
    switch (e->signal)
    {
    case OFF_SIG:
    {
        printf("\nTurn off calculator...");
        sleep(1);
        printf("\nBye Bye!\n");
        sleep(1);
        exit(0);
        return TRANS(&Calc_off);
    }
    case EQUAL_SIG:
    {
        printf("\nMy Calculator is ready");
        return TRANS(&Calc_ready);
    }
    case C_SIG:
    {
        printf("\n");
        getValue();
        return TRANS(&Calc_ready);
    }
    }
    return SUPER(&Hsm_top);
}

State Calc_ready(Calc *me, const Event *e)
{
    switch (e->signal)
    {
    case DIGIT_0_SIG:
    {
        printf("\n0");
        insert('0');
        return TRANS(&Calc_zero1);
    }
    case DIGIT_1_9_SIG:
    {
        printf("\n%c", ((CalcEvent*)e)->keyCode);
        insert(((CalcEvent*)e)->keyCode);
        return TRANS(&Calc_int1);
    }
    case POINT_SIG:
    {
        insert('0');
        insert('.');
        printf("\n0.");
        return TRANS(&Calc_frac1);
    }
    case OPER_SIG:
    {
        if (((CalcEvent*)e)->keyCode == '-')
        {
            printf("\n-");
            insert(((CalcEvent*)e)->keyCode);
            return TRANS(&Calc_negated1);
        }
    }
    }
    return SUPER(&Calc_on);
}

State Calc_zero1(Calc *me, const Event *e)
{
    switch (e->signal)
    {
    case DIGIT_0_SIG: return HANDLED();
    case DIGIT_1_9_SIG:
    {
        printf("%c", ((CalcEvent*)e)->keyCode);
        insert(((CalcEvent*)e)->keyCode);
        return TRANS(&Calc_int1);
    }
    case POINT_SIG:
    {
        printf(".");
        insert('.');
        return TRANS(&Calc_frac1);
    }
    }
    return SUPER(&Calc_operand1);
}

State Calc_int1(Calc *me, const Event *e)
{
    switch (e->signal)
    {
    case DIGIT_0_SIG: case DIGIT_1_9_SIG:
    {
        printf("%c", ((CalcEvent*)e)->keyCode);
        insert(((CalcEvent*)e)->keyCode);
        return HANDLED();
    }
    case POINT_SIG:
    {
        printf(".");
        insert('.');
        return TRANS(&Calc_frac1);
    }
    }
    return SUPER(&Calc_operand1);
}

State Calc_frac1(Calc *me, const Event *e)
{
    switch (e->signal)
    {
    case DIGIT_0_SIG: case DIGIT_1_9_SIG:
    {
        printf("%c", ((CalcEvent*)e)->keyCode);
        insert(((CalcEvent*)e)->keyCode);
        return HANDLED();
    }
    case POINT_SIG: return HANDLED();
    }
    return SUPER(&Calc_operand1);
}

State Calc_operand1(Calc *me, const Event *e)
{
    switch (e->signal)
    {
    case OPER_SIG:
    {
        me->operand1 = getValue();
        printf(" %c ", ((CalcEvent*)e)->keyCode);
        me->operator = ((CalcEvent*)e)->keyCode;
        return TRANS(&Calc_opEntered);
    }
    case EQUAL_SIG:
    {
        me->operand1 = getValue();
        printf(" = %f", me->operand1);
        return TRANS(&Calc_result);
    }
    }
    return SUPER(&Calc_on);
}

State Calc_negated1(Calc *me, Event const *e)
{
    switch (e->signal)
    {
    case OPER_SIG: return HANDLED();
    }
    return SUPER(&Calc_ready);
}

State Calc_opEntered(Calc *me, const Event *e)
{
    switch (e->signal)
    {
    case OPER_SIG:
    {
        if (((CalcEvent*)e)->keyCode == '-')
        {
            printf("-");
            insert(((CalcEvent*)e)->keyCode);
            return TRANS(&Calc_negated2);
        }
        return HANDLED();
    }
    case DIGIT_0_SIG:
    {
        printf("0");
        insert(((CalcEvent*)e)->keyCode);
        return TRANS(&Calc_zero2);
    }
    case DIGIT_1_9_SIG:
    {
        printf("%c", ((CalcEvent*)e)->keyCode);
        insert(((CalcEvent*)e)->keyCode);
        return TRANS(&Calc_int2);
    }
    case POINT_SIG:
    {
        printf("0.");
        insert((int)'0');
        insert((int)'.');
        return TRANS(&Calc_frac2);
    }
    }
    return SUPER(&Calc_on);
}

State Calc_negated2(Calc *me, Event const *e)
{
    switch (e->signal)
    {
    case OPER_SIG: return HANDLED();
    }
    return SUPER(&Calc_opEntered);
}

State Calc_zero2(Calc *me, const Event *e)
{
    switch (e->signal)
    {
    case DIGIT_0_SIG: return HANDLED();
    case DIGIT_1_9_SIG:
    {
        printf("%c", ((CalcEvent*)e)->keyCode);
        insert(((CalcEvent*)e)->keyCode);
        return TRANS(&Calc_int2);
    }
    case POINT_SIG:
    {
        printf(".");
        insert('.');
        return TRANS(&Calc_frac2);
    }
    }
    return SUPER(&Calc_operand2);
}

State Calc_int2(Calc *me, const Event *e)
{
    switch (e->signal)
    {
    case DIGIT_0_SIG: case DIGIT_1_9_SIG:
    {
        printf("%c", ((CalcEvent*)e)->keyCode);
        insert(((CalcEvent*)e)->keyCode);
        return HANDLED();
    }
    case POINT_SIG:
    {
        printf(".");
        insert('.');
        return TRANS(&Calc_frac2);
    }
    }
    return SUPER(&Calc_operand2);
}

State Calc_frac2(Calc *me, const Event *e)
{
    switch (e->signal)
    {
    case DIGIT_0_SIG: case DIGIT_1_9_SIG:
    {
        printf("%c", ((CalcEvent*)e)->keyCode);
        insert(((CalcEvent*)e)->keyCode);
        return HANDLED();
    }
    case POINT_SIG: return HANDLED();
    }
    return SUPER(&Calc_operand2);
}

State Calc_operand2(Calc *me, const Event *e)
{
    switch (e->signal)
    {
    case OPER_SIG:
    {
        if (me->operator == '+') me->operand1 += getValue();
        if (me->operator == '-') me->operand1 -= getValue();
        if (me->operator == '*') me->operand1 *= getValue();
        if (me->operator == '/')
        {
            if (getValue() == 0.0)
            {
                printf("\nMath ERROR");
                return TRANS(&Calc_error);
            }
            me->operand1 /= getValue();
        }
        printf("\n%f %c ", me->operand1, ((CalcEvent*)e)->keyCode);
        me->operator = ((CalcEvent*)e)->keyCode;
        return TRANS(&Calc_opEntered);
    }
    case EQUAL_SIG:
    {
        if (me->operator == '+') me->operand1 += getValue();
        if (me->operator == '-') me->operand1 -= getValue();
        if (me->operator == '*') me->operand1 *= getValue();
        if (me->operator == '/')
        {
            if (getValue() == 0.0)
            {
                printf("\nMath ERROR");
                return TRANS(&Calc_error);
            }
            me->operand1 /= getValue();
        }
        printf(" = %f", me->operand1);
        return TRANS(&Calc_result);
    }
    }
    return SUPER(&Calc_on);
}

State Calc_result(Calc *me, const Event *e)
{
    switch (e->signal)
    {
    case OPER_SIG:
    {
        printf("\n%f %c ", me->operand1, ((CalcEvent*)e)->keyCode);
        me->operator = ((CalcEvent*)e)->keyCode;
        return TRANS(&Calc_opEntered);
    }
    }
    return SUPER(&Calc_ready);
}

State Calc_error(Calc *me, const Event *e)
{
    return SUPER(&Calc_on);
}

State Calc_off(Calc *me, Event const *e)
{
    return HANDLED();
}

/***************************************************************/

int main()
{
    //Hsm_init((Hsm*)&l_calc, (Event*)0);

    printf("Press 0...9 to enter digit\n"
           "Press '.'   to enter decimal point\n"
           "Press 'a'   to add\n"
           "Press 's'   to subtract\n"
           "Press 'm'   to multiply\n"
           "Press 'd'   to divide\n"
           "Press 'e'   to get result\n"
           "Press 'c'   to clear\n"
           "Press ESC   to turn off Calculator\n");

    Calc_ctor();

    enable_raw_mode();

    while (1)
    {
        char c = getchar();
        CalcEvent ce;
        ((Event *)&ce)->signal = NONE;
        switch (c)
        {
        case '0':
        {
            ce.keyCode = '0';
            ((Event *)&ce)->signal = DIGIT_0_SIG;
            break;
        }
        case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        {
            ce.keyCode = c;
            ((Event *)&ce)->signal = DIGIT_1_9_SIG;
            break;
        }
        case '.':
        {
            ce.keyCode = c;
            ((Event *)&ce)->signal = POINT_SIG;
            break;
        }
        case 'c':
        {
            ce.keyCode = c;
            ((Event *)&ce)->signal = C_SIG;
            break;
        }
        case 'a':
        {
            ce.keyCode = '+';
            ((Event *)&ce)->signal = OPER_SIG;
            break;
        }
        case 's':
        {
            ce.keyCode = '-';
            ((Event *)&ce)->signal = OPER_SIG;
            break;
        }
        case 'm':
        {
            ce.keyCode = '*';
            ((Event *)&ce)->signal = OPER_SIG;
            break;
        }
        case 'd':
        {
            ce.keyCode = '/';
            ((Event *)&ce)->signal = OPER_SIG;
            break;
        }
        case 'e':
        {
            ce.keyCode = '=';
            ((Event *)&ce)->signal = EQUAL_SIG;
            break;
        }
        case '\33':
        {
            ce.keyCode = c;
            ((Event *)&ce)->signal = OFF_SIG;
            break;
        }
        }
        if (((Event *)&ce)->signal != NONE)
            Hsm_dispatch((Hsm*)&l_calc, (Event*)&ce);
    }

    disable_raw_mode();

    return 0;
}
