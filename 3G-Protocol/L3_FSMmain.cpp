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

//static uint8_t myL3ID   =   1;
//static uint8_t destL3ID =   0;
static uint8_t conID    =   0;

// state variables
static uint8_t main_state = STATE_IDLE; // protocol state
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

// 새로 추가함
uint8_t cond_IDinput; // 0 - no ID, 1 - ID input is finished

static uint8_t input_thisId; // my ID

// destination sdu
static uint8_t destinationWord[10];
static uint8_t destinationLen = 0;
static uint8_t destinationsdu[10];
char destinationString[12];

static uint8_t size = L3_LLI_getSize();

//static uint8_t flag_needPrint = 1;


//unsigned sleep(unsigned int a);


// application event handler : generating SDU from keyboard input
static void L3service_processInputWord(void)
{
    char c = pc.getc();

    if (main_state == STATE_IDLE){
        if(!L3_event_checkEventFlag(ReqCON_Send)){
        // 입력받고 변경하기 char 를 intf로, condition =1로
            //pc.printf(" \n What number do you want to connect ?????????? ");

            if (c == '\n' || c == '\r')
            {

                destinationWord[destinationLen++] = '\0';
            
                myDestId = atoi((const char*)destinationWord);
                destinationLen = 0;

                pc.printf("My dest ID : %i\n", myDestId);

                L3_event_setEventFlag(ReqCON_Send); 
            }
            else
            {
            destinationWord[destinationLen++] = c; // 이게 뭐지
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
    #if 0
    else if (!L3_event_checkEventFlag(ReqCON_Send))
    {
        if (c == '\n' || c == '\r')
        {
            originalWord[wordLen++] = '\0';
            L3_event_setEventFlag(ReqCON_Send);
            debug_if(DBGMSG_L3, "destination word is ready! ::: %s\n", originalWord);
        }
        else
        {
            originalWord[wordLen++] = c;
            if (wordLen >= IDLEMSG_MAXDATASIZE - 1)
            {
                originalWord[wordLen++] = '\0';
                L3_event_setEventFlag(ReqCON_Send);
                pc.printf("\n max reached! destination word forced to be ready :::: %s\n", originalWord);
            }
        }
    }
    #endif
}

// initialize service layer
void L3_initFSM(uint8_t input_thisId)
{

    input_thisId = input_thisId;

    pc.printf(" \n What number do you want to connect ? man~  ");

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
            pc.printf("\nTry connect to %i, PDU context : %i %i %i\n", myDestId, pdu[0], pdu[1], pdu[2]);
            
            L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, myDestId); 

            conID = myDestId;

            pc.printf("\n Send ReqCON PUD to MT \n");

            // state chage MT(전화하는 애) : To CON_WAIT
            main_state = STATE_CON_WAIT;
            L3_event_clearEventFlag(ReqCON_Send);

        }
        // b
        else if (L3_event_checkEventFlag(ReqCON_Rcvd)){

            conID = L3_LLI_getSrcId();
            
            pc.printf("\n CONNECTING START : CHAT으로 가기 위한 여정의 시작..");
       
            //conID = myDestId; // 받은 ID가 내 목적지가 됨
            myDestId = conID;

            // make SetCONPDU with accept
            Msg_encodeCONPDU(pdu, MSG_RSC_Set, MSG_ACP_ACCEPT); // srcID로 한 이유는 처음 받은 애한테는 무조건 받아야 해서..
            L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, myDestId);

            pc.printf("\n Send SetCON accept PUD to MO ");

            // state change : MO(전화받는 애) To CON_WAIT
            main_state = STATE_CON_WAIT;
            //flag_needPrint = 1; // flag = 1 일 때 채팅 상태로 넘어감!!

            L3_event_clearEventFlag(ReqCON_Rcvd);
        }

        // return to IDLE 
        // c
        else if (L3_event_checkEventFlag(SetCON_Accept_Rcvd))
        {
            pc.printf("c");
            L3_event_clearEventFlag(SetCON_Accept_Rcvd);
        }

        // d
        else if (L3_event_checkEventFlag(SetCON_Reject_Rcvd))
        {
            pc.printf("d");
            L3_event_clearEventFlag(SetCON_Reject_Rcvd);
        }

        // e
        else if (L3_event_checkEventFlag(CplCON_Rcvd))
        {
            pc.printf("e");
            L3_event_clearEventFlag(CplCON_Rcvd);
        }

        // g
        else if (L3_event_checkEventFlag(SDU_Rcvd))
        {
            pc.printf("g");
            L3_event_clearEventFlag(SDU_Rcvd);
        }

        // h
        else if (L3_event_checkEventFlag(Chat_Rcvd))
        {
            pc.printf("h");
            L3_event_clearEventFlag(Chat_Rcvd);
        }

        // i
        else if (L3_event_checkEventFlag(SDU_Rcvd))
        {
            pc.printf("i");
            L3_event_clearEventFlag(SDU_Rcvd);
        }

        // j
        else if (L3_event_checkEventFlag(Chat_Timer_Expire))
        {
            pc.printf("j");
            L3_event_clearEventFlag(Chat_Timer_Expire);
        }

        // k
        else if (L3_event_checkEventFlag(SetDIS_Rcvd))
        {
            pc.printf("k");
            L3_event_clearEventFlag(SetDIS_Rcvd);
        }

        // l
        else if (L3_event_checkEventFlag(CplDIS_Rcvd))
        {
            pc.printf("l");
            L3_event_clearEventFlag(CplDIS_Rcvd);
        }

        break;

    // CON STATE
    case STATE_CON_WAIT:

       

        // c 
        if (L3_event_checkEventFlag(SetCON_Accept_Rcvd))
        {
            verseID = L3_LLI_getSrcId(); // MT ID check
            if (verseID != conID)
            {
                pc.printf(" PDU ID is not matching (verseID : %i, conID : %i))\n You can't connect ", verseID, conID);
            }
            else
            {
                pc.printf("\nSTART CON_WAIT \n");

                // cplCON을 보내야 함
                Msg_encodeCONPDU(pdu, MSG_RSC_Cpl, MSG_ACP_ACCEPT);
                L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, myDestId);

                pc.printf("\n Send CplCON PUD to MT\n ");
                pc.printf(" \n -------------------------------------------------\nSTATE : CHAT MODE\n -------------------------------------------------\n");

                // state chage MO(전화하는 애) : To CHAT 
                main_state = STATE_CHAT;
    
                pc.printf("\n -------------------------------------------------\nGive Word to Send: \n -------------------------------------------------\n");
            }
            L3_event_clearEventFlag(SetCON_Accept_Rcvd);
        }

        // d
        else if (L3_event_checkEventFlag(SetCON_Reject_Rcvd))
        {
            verseID = L3_LLI_getSrcId(); 
            if (verseID != conID)
            {
                // check = 1;
                 pc.printf("\n -------------------------------------------------\n YOU get irrelevant ID PDU\n -------------------------------------------------\n ");
            }
            else
            {
                pc.printf("\n -------------------------------------------------\nYOU are reiected please wait\n -------------------------------------------------\n");

                main_state = STATE_IDLE;
                pc.printf("\n -------------------------------------------------\nSTATE BACK to IDLE \n -------------------------------------------------\n");
                pc.printf(" \n -------------------------------------------------\n What number do you want to connect ? retry  ");
            }
                L3_event_clearEventFlag(SetCON_Reject_Rcvd);

        }

        // e (timer)
        else if (L3_event_checkEventFlag(CplCON_Rcvd)) // data TX finished
        {
            verseID = L3_LLI_getSrcId(); // accept의 id
            if (verseID != conID)
            {
                pc.printf(" PDU ID is not matching (verseID : %i, conID : %i))\n You can't connect ", verseID, conID);

            }
            else
            {
                pc.printf("\n -------------------------------------------------\nYOU get CplCON from MO\n -------------------------------------------------\n");
                pc.printf(" \n -------------------------------------------------\nSTATE : CHAT MODE\n -------------------------------------------------\n");

                pc.printf("\n -------------------------------------------------\nTimter start!\n -------------------------------------------------\n");
                L3_timer_Chat_Timer();
                
                main_state = STATE_CHAT;

                pc.printf("\n -----------------------------------------\nGive a word to send : \n -----------------------------------------\n");


            }

            L3_event_clearEventFlag(CplCON_Rcvd);
        }

        // return to CON_wait
        // a
        else if (L3_event_checkEventFlag(ReqCON_Send))
        {
            pc.printf("a");
            L3_event_clearEventFlag(ReqCON_Send);
        }
         // f 
        if (L3_event_checkEventFlag(ReqCON_Rcvd)) // 다른 ID로부터 req를 받으면
        {
           verseID = L3_LLI_getSrcId(); // accept의 id
            if (verseID != conID)
            {
                pc.printf("other ID is try to connect with you");

                pc.printf("\n -----------------------------------------\nYou are in CON_Wait mode\n -----------------------------------------\n, I will send to reject! ");

                // setCON reject pdu
                Msg_encodeCONPDU(pdu, MSG_RSC_Set, MSG_ACP_REJECT);
                L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, verseID);
            }

            else
            {
                pc.printf(" error action ");
            }

              L3_event_clearEventFlag(ReqCON_Rcvd);
        }
        // b
        else if (L3_event_checkEventFlag(ReqCON_Rcvd))
        {
            pc.printf("b");
            L3_event_clearEventFlag(ReqCON_Rcvd);
        }

        // g
        else if (L3_event_checkEventFlag(SDU_Rcvd))
        {
            pc.printf("g");
            L3_event_clearEventFlag(SDU_Rcvd);
        }

        // h
        else if (L3_event_checkEventFlag(Chat_Rcvd))
        {
            pc.printf("h");
            L3_event_clearEventFlag(Chat_Rcvd);
        }

        // i
        else if (L3_event_checkEventFlag(SDU_Rcvd))
        {
            pc.printf("i");
            L3_event_clearEventFlag(SDU_Rcvd);
        }

        // j
        else if (L3_event_checkEventFlag(Chat_Timer_Expire))
        {
            pc.printf("j");
            L3_event_clearEventFlag(Chat_Timer_Expire);
        }

        // k
        else if (L3_event_checkEventFlag(SetDIS_Rcvd))
        {
            pc.printf("k");
            L3_event_clearEventFlag(SetDIS_Rcvd);
        }

        // l
        else if (L3_event_checkEventFlag(CplDIS_Rcvd))
        {
            pc.printf("l");
            L3_event_clearEventFlag(CplDIS_Rcvd);
        }
        break;



    case STATE_CHAT:
        
        // f 
        if (L3_event_checkEventFlag(ReqCON_Rcvd)) // 다른 ID로부터 req를 받으면
        {
           verseID = L3_LLI_getSrcId(); // accept의 id
            if (verseID != conID)
            {
                pc.printf("other ID is try to connect with you");

                pc.printf("You are in chat mode, I will send to reject! ");
                
                // setCON reject pdu
                Msg_encodeCONPDU(pdu, MSG_RSC_Set, MSG_ACP_REJECT);
                L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, verseID);
            }

            else
            {
                pc.printf(" error action ");
            }

              L3_event_clearEventFlag(ReqCON_Rcvd);
        }

        // i
        else if (L3_event_checkEventFlag(Chat_Timer_Expire))
        {
            //verseID = L3_LLI_getSrcId(); // accept의 id
            //if (verseID != conID)
            //{
                 //pc.printf(" PDU ID is not matching (verseID : %i, conID : %i))\n You can't connect ", verseID, conID);
             
           // }
            //else
            {
                Msg_encodeDISPDU(pdu, MSG_RSC_Req);
                L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, myDestId);
                pc.printf("\n -------------------------------------------------\nTimer_Expire, start DISconnection!\n -------------------------------------------------\n");
               
                main_state = STATE_DIS_WAIT;
                wordLen = 0;    // clear
                L3_event_clearEventFlag(Chat_Timer_Expire);
               
            }
            //flag_needPrint == 1;
        }

        // j
        else if (L3_event_checkEventFlag(ReqDIS_Rcvd))
        {
            verseID = L3_LLI_getSrcId(); // accept의 id
            if (verseID != conID)
            {
                pc.printf(" PDU ID is not matching (verseID : %i, conID : %i))\n You can't connect ", verseID, conID);

            }

            else
            {
                
                pc.printf("YOU get ReqDIS from MT, DISConnect is start! ");

                pc.printf(" Your STATE is  DIS_WAIT ");

                // SET DIS PUD 
                Msg_encodeDISPDU(pdu, MSG_RSC_Set);
                L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, myDestId);

                pc.printf("\n Send SetDIS PUD to MT ");

                pc.printf(" \nYour STATE is  DIS_WAIT ");
           
                main_state = STATE_DIS_WAIT;
                wordLen = 0;    // clear

            }
                L3_event_clearEventFlag(ReqDIS_Rcvd);
        
        }
        
        // g
        else if (L3_event_checkEventFlag(SDU_Rcvd)) // if data needs to be sent (keyboard input)
        {
            verseID = L3_LLI_getSrcId();
            uint8_t* dataPtr=L3_LLI_getMsgPtr();
            if (verseID != conID)
            {
                pc.printf(" PDU ID is not matching (verseID : %i, conID : %i))\n You can't connect ", verseID, conID);
        
            }
            else
            {
                 // msg header setting
                pduSize = Msg_encodeCHAT(sdu, originalWord, wordLen);
                // Msg_encodeCHAT
                L3_LLI_dataReqFunc(sdu, pduSize, myDestId);

                pc.printf("\n -------------------------------------------------\n your are sending to %i, pduSize:%i\n -------------------------------------------------\n", myDestId, pduSize);

                //dataPtr=L3_LLI_getMsgPtr();
               // pc.printf("\n -------------------------------------------------\nRCVD from dest-inner g  : %s (length:%i, seq:%i)\n -------------------------------------------------\n",  L3_msg_getWord(dataPtr), size, L3_msg_getSeq(dataPtr));
                pc.printf("\n Give a word to send : ");
                
                //flag_needPrint = 1;
                wordLen = 0;
                
            }
            //flag_needPrint == 1;

           L3_event_clearEventFlag(SDU_Rcvd);
        }   
            #if 0
            else if (flag_needPrint == 1)
            {
                 flag_needPrint = 0;
                //pc.printf("G !!!  ");
                //pc.printf("Give Word to Send: ");
               //flag_needPrint = 0;
            }
            #endif

            // h
            else if (L3_event_checkEventFlag(Chat_Rcvd)) // message recived from MO or MT
            {
               
                uint8_t* msg;
                uint8_t* dataPtr=L3_LLI_getMsgPtr();
                uint8_t size = L3_LLI_getSize();
                verseID = L3_LLI_getSrcId();
                //dataPtr=L3_LLI_getMsgPtr(); 

                msg = L3_msg_getChat(dataPtr);
                if (verseID != conID)
                {
                    pc.printf(" PDU ID is not matching (verseID : %i, conID : %i))\n You can't connect ", verseID, conID);

                }
                else
                {
                    pc.printf("\n -------------------------------------------------\nRCVD MSG : %s (length:%i)\n -------------------------------------------------\n", msg, size);

                    //debug("\n -------------------------------------------------\nRCVD MSG : %s (length:%i)\n -------------------------------------------------\n", msg, size);
                   
                }
                //flag_needPrint == 1;
                L3_event_clearEventFlag(Chat_Rcvd);
            }
            
            // Return to state chat 
            // a
            else if (L3_event_checkEventFlag(ReqCON_Send))
            {
                pc.printf("a");
                L3_event_clearEventFlag(ReqCON_Send);
            }

            // b
            else if (L3_event_checkEventFlag(ReqCON_Rcvd))
            {
                pc.printf("b");
                L3_event_clearEventFlag(ReqCON_Rcvd);
            }

            // c
            else if (L3_event_checkEventFlag(SetCON_Accept_Rcvd))
            {
                pc.printf("c");
                L3_event_clearEventFlag(SetCON_Accept_Rcvd);
            }

            // d
            else if (L3_event_checkEventFlag(SetCON_Reject_Rcvd))
            {
                pc.printf("d");
                L3_event_clearEventFlag(SetCON_Reject_Rcvd);
            }

            // e
            else if (L3_event_checkEventFlag(CplCON_Rcvd))
            {
                pc.printf("e");
                L3_event_clearEventFlag(CplCON_Rcvd);
            }

            // k
            else if (L3_event_checkEventFlag(SetDIS_Rcvd))
            {
                pc.printf("k");
                L3_event_clearEventFlag(SetDIS_Rcvd);
            }

            // l
            else if (L3_event_checkEventFlag(CplDIS_Rcvd))
            {
                pc.printf("l");
                L3_event_clearEventFlag(CplDIS_Rcvd);
            }
            break;

    // DIS STATE
    case STATE_DIS_WAIT:
        //pc.printf("this is state dis wait\n");
       
        // l
        if (L3_event_checkEventFlag(CplDIS_Rcvd))
        {
            {
                verseID = L3_LLI_getSrcId(); // accept의 id
                if (verseID != conID)
                {
                    pc.printf(" PDU ID is not matching (verseID : %i, conID : %i))\n You can't connect ", verseID, conID);

                }
                else
                {
                    pc.printf("\n -------------------------------------------------\nyou get CplDIS from MT \n -------------------------------------------------\n");
                    pc.printf("\n -------------------------------------------------\nDISconnect complete \n -------------------------------------------------\n");


                    pc.printf(" \n -------------------------------------------------\nSTATE CHANGED to IDLE ( MO )\n -------------------------------------------------\n");

                    main_state = STATE_IDLE;
                    pc.printf(" \n -------------------------------------------------\n What number do you want to connect ? retry  ");
                }
            }
            L3_event_clearEventFlag(CplDIS_Rcvd);
        }

        // k
        else if (L3_event_checkEventFlag(SetDIS_Rcvd))
        {
            verseID = L3_LLI_getSrcId(); // accept의 id
            if (verseID != conID)
            {
                pc.printf(" PDU ID is not matching (verseID : %i, conID : %i))\n You can't connect ", verseID, conID);

            }

            else
            {
                pc.printf("\n -------------------------------------------------\nyou get SETDIS from MO \n -------------------------------------------------\n");
                pc.printf("\n -------------------------------------------------\nSend CplDIS to MO \n -------------------------------------------------\n");

                // CplDISPDU 보내기
                Msg_encodeDISPDU(pdu, MSG_RSC_Cpl);
                L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, myDestId);

                pc.printf("\n -------------------------------------------------\nSTATE CHANGED to IDLE ( MT )\n -------------------------------------------------\n");


                main_state = STATE_IDLE;
                pc.printf(" \n -------------------------------------------------\nWhat number do you want to connect ? retry  ");
                
            }
            L3_event_clearEventFlag(SetDIS_Rcvd);
        }

        // return to DIS_WAIT
        // a
        else if (L3_event_checkEventFlag(ReqCON_Send))
        {
            pc.printf("a");
            L3_event_clearEventFlag(ReqCON_Send);
        }
         // f 
        if (L3_event_checkEventFlag(ReqCON_Rcvd)) // 다른 ID로부터 req를 받으면
        {
           verseID = L3_LLI_getSrcId(); // accept의 id
            if (verseID != conID)
            {
                pc.printf("other ID is try to connect with you");

                pc.printf("\n -----------------------------------------\nYou are in DIS_Wait mode\n -----------------------------------------\n, I will send to reject! ");

                // setCON reject pdu
                Msg_encodeCONPDU(pdu, MSG_RSC_Set, MSG_ACP_REJECT);
                L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, verseID);
            }

            else
            {
                pc.printf(" error action ");
            }

              L3_event_clearEventFlag(ReqCON_Rcvd);
        }
        // b
        else if (L3_event_checkEventFlag(ReqCON_Rcvd))
        {
            pc.printf("b");
            L3_event_clearEventFlag(ReqCON_Rcvd);
        }

        // c
        else if (L3_event_checkEventFlag(SetCON_Accept_Rcvd))
        {
            pc.printf("c");
            L3_event_clearEventFlag(SetCON_Accept_Rcvd);
        }

        // d
        else if (L3_event_checkEventFlag(SetCON_Reject_Rcvd))
        {
            pc.printf("d");
            L3_event_clearEventFlag(SetCON_Reject_Rcvd);
        }

        // e
        else if (L3_event_checkEventFlag(CplCON_Rcvd))
        {
            pc.printf("e");
            L3_event_clearEventFlag(CplCON_Rcvd);
        }

        // g
        else if (L3_event_checkEventFlag(SDU_Rcvd))
        {
            pc.printf("g");
            L3_event_clearEventFlag(SDU_Rcvd);
        }

        // h
        else if (L3_event_checkEventFlag(Chat_Rcvd))
        {
            pc.printf("h");
            L3_event_clearEventFlag(Chat_Rcvd);
        }

        // i
        else if (L3_event_checkEventFlag(SDU_Rcvd))
        {
            pc.printf("i");
            L3_event_clearEventFlag(SDU_Rcvd);
        }

        // j
        else if (L3_event_checkEventFlag(Chat_Timer_Expire))
        {
            pc.printf("j");
            L3_event_clearEventFlag(Chat_Timer_Expire);
        }

        break;

        default:
            break;
        }
    }
