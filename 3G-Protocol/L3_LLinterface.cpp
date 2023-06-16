#include "mbed.h"
#include "L3_FSMevent.h"
#include "L3_msg.h"
#include "protocol_parameters.h"
#include "time.h"
#include "L2_layer.h"

static uint8_t rcvdMsg[L3_MAXDATASIZE];
static uint8_t rcvdSize;
static int16_t rcvdRssi;
static int8_t rcvdSnr;
static uint8_t rcvdSrcId;

//Downward primitives
//TX function
void (*L3_LLI_dataReqFunc)(uint8_t* msg, uint8_t size, uint8_t destId);
void (*L3_LLI_reconfigSrcIdReqFunc)(uint8_t myId);

//interface event : DATA_IND, RX data has arrived
void L3_LLI_dataInd(uint8_t* dataPtr, uint8_t srcId, uint8_t size, int8_t snr, int16_t rssi)
{
    debug_if(DBGMSG_L3, "\n[L3] --> DATA IND : size:%i, ptr[0]: %s,ptr[1]: %s,ptr[2]: %s,ptr[3]: %s, typeCheck : %i(%i, %i)\n", 
        size, dataPtr[0],dataPtr[1],dataPtr[2],dataPtr[3], Msg_checkIfChat_Rcvd(dataPtr), dataPtr[MSG_OFFSET_TYPE], dataPtr[MSG_OFFSET_RSC]);

    memcpy(rcvdMsg, dataPtr, size*sizeof(uint8_t));
    rcvdSize = size;
    rcvdSnr = snr;
    rcvdRssi = rssi;
    rcvdSrcId = srcId;

    
    if (Msg_checkIfReqCON(dataPtr))     // event b 
    {
        L3_event_setEventFlag(ReqCON_Rcvd);
    }
    else if (Msg_checkIfSetCON_Accept_Rcvd(dataPtr)) // event c 
    {
        L3_event_setEventFlag(SetCON_Accept_Rcvd);
    }

    else if (Msg_checkIfSetCON_Reject_Rcvd(dataPtr)) //event d
    {
        L3_event_setEventFlag(SetCON_Reject_Rcvd);
    }

    else if (Msg_checkIfCplCON_Rcvd(dataPtr)) //event e
    {
        L3_event_setEventFlag(CplCON_Rcvd); 
    }
    
/*    else if (Msg_checkIfotherPDU(dataPtr))  // event f 
    { 
        L3_event_setEventFlag(Other_ID_ReqCON); 
    }*/


    else if (Msg_checkIfReqDIS_Rcvd(dataPtr)) //event j
    {
        L3_event_setEventFlag(ReqDIS_Rcvd);
    }
    
    else if (Msg_checkIfSetDIS_Rcvd(dataPtr)) //event k
    {
        L3_event_setEventFlag(SetDIS_Rcvd);
    }

    else if (Msg_checkIfCplDIS_Rcvd(dataPtr))  //event l
    {
        L3_event_setEventFlag(CplDIS_Rcvd);
    }

    else if (Msg_checkIfChat_Rcvd(dataPtr))
    {
        L3_event_setEventFlag(Chat_Rcvd);
    }
/*    else if (Msg_checkIfotherPDU(dataPtr)==0 ) // event m
    {      
        L3_event_setEventFlag(ReqCON_Other_Rcvd);
    }*/

  }


void L3_LLI_dataCnf(uint8_t res)
{
    debug_if(DBGMSG_L3, "\n --> DATA CNF : res : %i\n", res);
    L3_event_setEventFlag(L3_event_dataSendCnf);
}
void L3_LLI_reconfigSrcIdCnf(uint8_t res)
{
    debug_if(DBGMSG_L3, "\n --> RECONFIG SRCID CNF : res : %i\n", res);
    L3_event_setEventFlag(L3_event_recfgSrcIdCnf);
}


uint8_t* L3_LLI_getMsgPtr()
{
    return rcvdMsg;
}
uint8_t L3_LLI_getSize()
{
    return rcvdSize;
}

uint8_t L3_LLI_getSrcId()
{
    return rcvdSrcId;
}

void L3_LLI_setDataReqFunc(void (*funcPtr)(uint8_t*, uint8_t, uint8_t))
{
    L3_LLI_dataReqFunc = funcPtr;   
}


void L3_LLI_setReconfigSrcIdReqFunc(void (*funcPtr)(uint8_t))
{
    L3_LLI_reconfigSrcIdReqFunc = funcPtr;
}