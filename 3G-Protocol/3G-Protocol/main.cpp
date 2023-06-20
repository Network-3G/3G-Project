#include "mbed.h"
#include "string.h"
#include "L2_FSMmain.h"
#include "L3_FSMmain.h"

// serial port interface
Serial pc(USBTX, USBRX);

// GLOBAL variables (DO NOT TOUCH!) ------------------------------------------

// source/destination ID
static uint8_t input_thisId = 1;
//uint8_t input_destId = 0;

// FSM operation implementation ------------------------------------------------
int main(void)
{

    // initialization
    pc.printf("------------------ protocol stack starts! --------------------------\n");
    // source & destination ID setting
    pc.printf("\n ID for this node :\n");
    pc.scanf("%d", &input_thisId);
    pc.getc();

    pc.printf("\n My ID : %i\n", input_thisId);

    // initialize lower layer stacks, send my ID
    L2_initFSM(input_thisId); 
    L3_initFSM(input_thisId);

    while (1)
    {
        L2_FSMrun();
        L3_FSMrun();
    }
}