#include "L3_FSMevent.h"
#include "L3_msg.h"
#include "L3_timer.h"
#include "L3_LLinterface.h"
#include "protocol_parameters.h"
#include "mbed.h"

//#include <stdio.h>
//#include <unistd.h>

// FSM state -------------------------------------------------

#define STATE_IDLE          0
#define STATE_CON_WAIT      1
#define STATE_CHAT          2
#define STATE_DIS_WAIT      3

static uint8_t conID    =   0;

// state variables
static uint8_t main_state = STATE_IDLE;
static uint8_t prev_state = main_state;

// SDU (input)
static uint8_t originalWord[1030];
static uint8_t wordLen = 0;
static uint8_t sdu[1030];

uint8_t pdu[5];      // PDU
uint8_t pduSize = 0;

// serial port interface
static Serial pc(USBTX, USBRX);
static uint8_t myDestId;
uint8_t verseID = 0; 

uint8_t cond_IDinput; // 0 - no ID, 1 - ID input is finished

//static uint8_t input_thisId; // my ID

// destination sdu
static uint8_t destinationWord[10];
static uint8_t destinationLen = 0;
//static uint8_t destinationsdu[10];
//char destinationString[12];

static uint8_t size = L3_LLI_getSize();


// application event handler : generating SDU from keyboard input
static void L3service_processInputWord(void)
{
    char c = pc.getc();

    if (main_state == STATE_IDLE){
        if(!L3_event_checkEventFlag(ReqCON_Send)){

            if (c == '\n' || c == '\r')
            {
                destinationWord[destinationLen++] = '\0';
            
                myDestId = atoi((const char*)destinationWord);
                destinationLen = 0;
                pc.printf("\n My destination ID : %i\n", myDestId);

                L3_event_setEventFlag(ReqCON_Send); 
            }
            else
            {
            destinationWord[destinationLen++] = c;
            }
        }
    }

    else if (main_state == STATE_CHAT){ 
        if(!L3_event_checkEventFlag(SDU_Rcvd)) {
            
            if (c == '\n' || c == '\r')
            {
                originalWord[wordLen++] = '\0';
                pc.printf("\n -------------------------------------------------\n You are sending : %s\n -------------------------------------------------\n", originalWord);
                L3_event_setEventFlag(SDU_Rcvd);
            }
            else
            {
                originalWord[wordLen++] = c;
            }
        }
    }
}

// initialize service layer
void L3_initFSM(uint8_t input_thisId)
{
    input_thisId = input_thisId;
    pc.printf(" \n What ID do you want to connect?");
    pc.attach(&L3service_processInputWord, Serial::RxIrq);
}


void L3_FSMrun(void)
{
    if (prev_state != main_state)
    {
        debug_if(DBGMSG_L3, "\n [L3] State transition from %i to %i\n", prev_state, main_state);
        prev_state = main_state;
    }

    // FSM should be implemented here! ---->>>>
    switch (main_state)
    {

    // IDLE STATE
    case STATE_IDLE:

        // a
        if (L3_event_checkEventFlag(ReqCON_Send))
        {
            Msg_encodeCONPDU(pdu, MSG_RSC_Req, MSG_ACP_ACCEPT);
            pc.printf("\n Trying connect to %i\n", myDestId);
            
            L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, myDestId); 
            conID = myDestId;

            // state chage
            main_state = STATE_CON_WAIT;
            L3_event_clearEventFlag(ReqCON_Send);
        }
        // b
        else if (L3_event_checkEventFlag(ReqCON_Rcvd)){

            conID = L3_LLI_getSrcId();
            pc.printf("\n ReqCON received\n");
            pc.printf("\n CONNECTING START : CHAT으로 가기 위한 여정의 시작...");
            myDestId = conID;

            Msg_encodeCONPDU(pdu, MSG_RSC_Set, MSG_ACP_ACCEPT);
            L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, myDestId);

            main_state = STATE_CON_WAIT;

            L3_event_clearEventFlag(ReqCON_Rcvd);
        }

        // c
        else if (L3_event_checkEventFlag(SetCON_Accept_Rcvd))
        {
            L3_event_clearEventFlag(SetCON_Accept_Rcvd);
        }

        // d
        else if (L3_event_checkEventFlag(SetCON_Reject_Rcvd))
        {
            L3_event_clearEventFlag(SetCON_Reject_Rcvd);
        }

        // e
        else if (L3_event_checkEventFlag(CplCON_Rcvd))
        {
            L3_event_clearEventFlag(CplCON_Rcvd);
        }

        // g
        else if (L3_event_checkEventFlag(SDU_Rcvd))
        {
            L3_event_clearEventFlag(SDU_Rcvd);
        }

        // h
        else if (L3_event_checkEventFlag(Chat_Rcvd))
        {
            L3_event_clearEventFlag(Chat_Rcvd);
        }

        // i
        else if (L3_event_checkEventFlag(SDU_Rcvd))
        {
            L3_event_clearEventFlag(SDU_Rcvd);
        }

        // j
        else if (L3_event_checkEventFlag(Chat_Timer_Expire))
        {
            L3_event_clearEventFlag(Chat_Timer_Expire);
        }

        // k
        else if (L3_event_checkEventFlag(SetDIS_Rcvd))
        {
            L3_event_clearEventFlag(SetDIS_Rcvd);
        }

        // l
        else if (L3_event_checkEventFlag(CplDIS_Rcvd))
        {
            L3_event_clearEventFlag(CplDIS_Rcvd);
        }
        break;


    // CON STATE
    case STATE_CON_WAIT:

        // c 
        if (L3_event_checkEventFlag(SetCON_Accept_Rcvd))
        {
            verseID = L3_LLI_getSrcId();
            if (verseID != conID)
            {
                pc.printf("\n PDU ID is not matching (verseID : %i, conID : %i))\n You can't connect ", verseID, conID);
            }
            else
            {
                pc.printf("\n SetCON accept received\n");
                Msg_encodeCONPDU(pdu, MSG_RSC_Cpl, MSG_ACP_ACCEPT);
                L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, myDestId);

                pc.printf(" \n -------------------------------------------------\n STATE : CHAT\n -------------------------------------------------\n");

                main_state = STATE_CHAT;
    
                pc.printf("\n -------------------------------------------------\n Give a Word to Send: \n");
            }
            L3_event_clearEventFlag(SetCON_Accept_Rcvd);
        }

        // d
        else if (L3_event_checkEventFlag(SetCON_Reject_Rcvd))
        {
            verseID = L3_LLI_getSrcId(); 
            if (verseID != conID)
            {
                 pc.printf("\n You got irrelevant ID PDU\n");
            }
            else
            {
                pc.printf("\n SetCON reject received, please wait\n");

                main_state = STATE_IDLE;
                pc.printf("\n -------------------------------------------------\n STATE BACK to IDLE \n -------------------------------------------------\n");
                pc.printf("\n -------------------------------------------------\n What ID do you want to connect ?");
            }
                L3_event_clearEventFlag(SetCON_Reject_Rcvd);

        }

        // e (timer)
        else if (L3_event_checkEventFlag(CplCON_Rcvd))
        {
            verseID = L3_LLI_getSrcId();
            if (verseID != conID)
            {
                pc.printf("\n PDU ID is not matching (verseID : %i, conID : %i))\n You can't connect \n", verseID, conID);
            }
            else
            {
                pc.printf("\n CplCON received\n");
                pc.printf(" \n -------------------------------------------------\n STATE : CHAT \n -------------------------------------------------\n");

                pc.printf("\n Timer start! \n");
                L3_timer_Chat_Timer();
                
                main_state = STATE_CHAT;

                pc.printf("\n -----------------------------------------\n Give a word to send : \n");


            }

            L3_event_clearEventFlag(CplCON_Rcvd);
        }


        // a
        else if (L3_event_checkEventFlag(ReqCON_Send))
        {
            L3_event_clearEventFlag(ReqCON_Send);
        }
         // f 
        if (L3_event_checkEventFlag(ReqCON_Rcvd)) 
        {
           verseID = L3_LLI_getSrcId();
            if (verseID != conID)
            {
                pc.printf("\n other ID is trying to connect with you");
                pc.printf("\n You are in CON_WAIT state, sending reject PDU ");
                Msg_encodeCONPDU(pdu, MSG_RSC_Set, MSG_ACP_REJECT);
                L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, verseID);
            }
            // b
            else
            {
                pc.printf("\n error action ");
            }

              L3_event_clearEventFlag(ReqCON_Rcvd);
        }

        // g
        else if (L3_event_checkEventFlag(SDU_Rcvd))
        {
            L3_event_clearEventFlag(SDU_Rcvd);
        }

        // h
        else if (L3_event_checkEventFlag(Chat_Rcvd))
        {
            L3_event_clearEventFlag(Chat_Rcvd);
        }

        // i
        else if (L3_event_checkEventFlag(SDU_Rcvd))
        {
            L3_event_clearEventFlag(SDU_Rcvd);
        }

        // j
        else if (L3_event_checkEventFlag(Chat_Timer_Expire))
        {
            L3_event_clearEventFlag(Chat_Timer_Expire);
        }

        // k
        else if (L3_event_checkEventFlag(SetDIS_Rcvd))
        {
            L3_event_clearEventFlag(SetDIS_Rcvd);
        }

        // l
        else if (L3_event_checkEventFlag(CplDIS_Rcvd))
        {
            L3_event_clearEventFlag(CplDIS_Rcvd);
        }
        break;



    case STATE_CHAT:
        
        // f 
        if (L3_event_checkEventFlag(ReqCON_Rcvd))
        {
           verseID = L3_LLI_getSrcId();
            if (verseID != conID)
            {
                pc.printf("\n other ID is trying to connect with you");
                pc.printf("\n You are in CHAT state, sending reject PDU \n");
                Msg_encodeCONPDU(pdu, MSG_RSC_Set, MSG_ACP_REJECT);
                L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, verseID);
                pc.printf("\n -------------------------------------------------\n Give a Word to Send: \n");
            }

            else // b
            {
                pc.printf("\n error action \n");
            }
              L3_event_clearEventFlag(ReqCON_Rcvd);
        }

        // i
        else if (L3_event_checkEventFlag(Chat_Timer_Expire))
        {
            {
                Msg_encodeDISPDU(pdu, MSG_RSC_Req);
                L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, myDestId);
                pc.printf("\n -------------------------------------------------\n Timer Expired, start DISconnection\n -------------------------------------------------\n");
               
                main_state = STATE_DIS_WAIT;
                wordLen = 0;
                L3_event_clearEventFlag(Chat_Timer_Expire);
            }
        }

        // j
        else if (L3_event_checkEventFlag(ReqDIS_Rcvd))
        {
            verseID = L3_LLI_getSrcId();
            if (verseID != conID)
            {
                pc.printf("\n PDU ID is not matching (verseID : %i, conID : %i)) You can't connect ", verseID, conID);
            }

            else
            {
                pc.printf("\n ReqDIS Received, start DISconnection ");
                Msg_encodeDISPDU(pdu, MSG_RSC_Set);
                L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, myDestId);

                pc.printf(" \n -------------------------------------------------\n STATE : DIS_WAIT\n -------------------------------------------------\n");

                main_state = STATE_DIS_WAIT;
                wordLen = 0;  
            }
                L3_event_clearEventFlag(ReqDIS_Rcvd);    
        }
        
        // g
        else if (L3_event_checkEventFlag(SDU_Rcvd)) // if data needs to be sent (keyboard input)
        {
            verseID = conID;
            if (verseID != conID)
            {
                pc.printf(" \n g: PDU ID is not matching (verseID : %i, conID : %i))\n You can't connect ", verseID, conID);   
            }
            else
            {
                // msg header setting
                pduSize = Msg_encodeCHAT(sdu, originalWord, wordLen);
                // Msg_encodeCHAT
                L3_LLI_dataReqFunc(sdu, pduSize, myDestId);

                pc.printf("\n -------------------------------------------------\n you are sending to %i, pduSize:%i\n -------------------------------------------------\n", myDestId, pduSize);
                pc.printf("\n -------------------------------------------------\n Give a word to send : \n");
                
                wordLen = 0;
            }

           L3_event_clearEventFlag(SDU_Rcvd);
        }   

            // h
            else if (L3_event_checkEventFlag(Chat_Rcvd))
            {
                uint8_t* msg;
                uint8_t* dataPtr=L3_LLI_getMsgPtr();
                uint8_t size = L3_LLI_getSize();
                verseID = L3_LLI_getSrcId();

                msg = L3_msg_getChat(dataPtr);
                if (verseID != conID)
                {
                    pc.printf("\n h:PDU ID is not matching (verseID : %i, conID : %i))\n You can't connect ", verseID, conID);
                }
                else
                {
                    pc.printf("\n -------------------------------------------------\n RCVD MSG : %s (length:%i)\n -------------------------------------------------\n", msg, size);
                }
                L3_event_clearEventFlag(Chat_Rcvd);
            }
            
            // a
            else if (L3_event_checkEventFlag(ReqCON_Send))
            {
                L3_event_clearEventFlag(ReqCON_Send);
            }

            // c
            else if (L3_event_checkEventFlag(SetCON_Accept_Rcvd))
            {
                L3_event_clearEventFlag(SetCON_Accept_Rcvd);
            }

            // d
            else if (L3_event_checkEventFlag(SetCON_Reject_Rcvd))
            {
                L3_event_clearEventFlag(SetCON_Reject_Rcvd);
            }

            // e
            else if (L3_event_checkEventFlag(CplCON_Rcvd))
            {
                L3_event_clearEventFlag(CplCON_Rcvd);
            }

            // k
            else if (L3_event_checkEventFlag(SetDIS_Rcvd))
            {
                L3_event_clearEventFlag(SetDIS_Rcvd);
            }

            // l
            else if (L3_event_checkEventFlag(CplDIS_Rcvd))
            {
                L3_event_clearEventFlag(CplDIS_Rcvd);
            }
            break;

    // DIS_WAIT STATE
    case STATE_DIS_WAIT:
        // l
        if (L3_event_checkEventFlag(CplDIS_Rcvd))
        {
            {
                verseID = L3_LLI_getSrcId();
                if (verseID != conID)
                {
                    pc.printf("\n PDU ID is not matching (verseID : %i, conID : %i))\n You can't connect ", verseID, conID);
                }
                else
                {
                    pc.printf("\n CplDIS received\n");
                    pc.printf("\n -------------------------------------------------\n DISconnect complete \n -------------------------------------------------\n");


                    pc.printf(" \n -------------------------------------------------\n STATE : IDLE \n -------------------------------------------------\n");

                    main_state = STATE_IDLE;
                    pc.printf(" \n -------------------------------------------------\n What ID do you want to connect ? ");
                }
            }
            L3_event_clearEventFlag(CplDIS_Rcvd);
        }

        // k
        else if (L3_event_checkEventFlag(SetDIS_Rcvd))
        {
            verseID = L3_LLI_getSrcId(); 
            if (verseID != conID)
            {
                pc.printf("\n PDU ID is not matching (verseID : %i, conID : %i))\n You can't connect ", verseID, conID);
            }

            else
            {
                pc.printf("\n SetDIS received\n");
               
                Msg_encodeDISPDU(pdu, MSG_RSC_Cpl);
                L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, myDestId);

                pc.printf("\n -------------------------------------------------\n STATE : IDLE\n -------------------------------------------------\n");

                main_state = STATE_IDLE;
                pc.printf(" \n -------------------------------------------------\n What ID do you want to connect ? ");
                
            }
            L3_event_clearEventFlag(SetDIS_Rcvd);
        }

        // a
        else if (L3_event_checkEventFlag(ReqCON_Send))
        {
            L3_event_clearEventFlag(ReqCON_Send);
        }
         // f 
        if (L3_event_checkEventFlag(ReqCON_Rcvd))
        {
           verseID = L3_LLI_getSrcId();
            if (verseID != conID)
            {
                pc.printf("\n other ID is trying to connect with you");
                pc.printf("\n You are in DIS_WAIT state, sending reject PDU \n");
                Msg_encodeCONPDU(pdu, MSG_RSC_Set, MSG_ACP_REJECT);
                L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, verseID);
  
            }

            else // b
            {
                pc.printf("\n error action ");
            }
              L3_event_clearEventFlag(ReqCON_Rcvd);
        }

        // c
        else if (L3_event_checkEventFlag(SetCON_Accept_Rcvd))
        {
            L3_event_clearEventFlag(SetCON_Accept_Rcvd);
        }

        // d
        else if (L3_event_checkEventFlag(SetCON_Reject_Rcvd))
        {
            L3_event_clearEventFlag(SetCON_Reject_Rcvd);
        }

        // e
        else if (L3_event_checkEventFlag(CplCON_Rcvd))
        {
            L3_event_clearEventFlag(CplCON_Rcvd);
        }

        // g
        else if (L3_event_checkEventFlag(SDU_Rcvd))
        {
            L3_event_clearEventFlag(SDU_Rcvd);
        }

        // h
        else if (L3_event_checkEventFlag(Chat_Rcvd))
        {
            L3_event_clearEventFlag(Chat_Rcvd);
        }

        // i
        else if (L3_event_checkEventFlag(SDU_Rcvd))
        {
            L3_event_clearEventFlag(SDU_Rcvd);
        }

        // j
        else if (L3_event_checkEventFlag(Chat_Timer_Expire))
        {
            L3_event_clearEventFlag(Chat_Timer_Expire);
        }

        break;

        default:
            break;
        }
    }
