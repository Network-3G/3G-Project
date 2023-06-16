typedef enum L3_event
{
    /*L3_event_dataTxDone = 0,
    L3_event_ackTxDone = 1,
    L3_event_ackRcvd = 2,
    L3_event_msgRcvd = 3,
    L3_event_dataToSend = 4,
    L3_event_arqTimeout = 5,
    L3_event_reconfigSrcId = 6,
    L3_event_dataToSendBuffer = 7*/

    // IDLE
    ReqCON_Rcvd = 2,
    ReqCON_Send = 4,

    L3_event_dataSendCnf = 5,
    L3_event_recfgSrcIdCnf = 6,

    L3_event_dataConDone = 7,
    L3_event_dataDisDone = 8,

    // state CON
    SetCON_Accept_Rcvd = 9,
    SetCON_Reject_Rcvd = 10,
    CplCON_Rcvd = 11,

    // state DIS
    SetDIS_Rcvd = 12,
    CplDIS_Rcvd = 13,

    // STATE CHAT
    Chat_Timer_Expire = 14,
    ReqDIS_Rcvd = 15,
    ReqCON_Other_Rcvd = 16,
    Chat_Rcvd = 17,
    SDU_Rcvd = 18, 
    Other_PDU_Rcvd =19

} L3_event_e;

void L3_event_setEventFlag(L3_event_e event);
void L3_event_clearEventFlag(L3_event_e event);
void L3_event_clearAllEventFlag(void);
int L3_event_checkEventFlag(L3_event_e event);