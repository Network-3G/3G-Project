#include "L3_FSMevent.h"
#include "L3_msg.h"
#include "L3_timer.h"
#include "L3_LLinterface.h"
#include "protocol_parameters.h"
#include "mbed.h"

// FSM state -------------------------------------------------
// #define L3STATE_IDLE              0

#define STATE_IDLE 0
#define STATE_CON_WAIT 1
#define STATE_CHAT 2
#define STATE_DIS_WAIT 3

static uint8_t myL3ID = 1;
static uint8_t destL3ID = 0;
static uint8_t conID = 0;
// state variables
static uint8_t main_state = STATE_IDLE; // protocol state
static uint8_t prev_state = main_state;

// SDU (input)
static uint8_t originalWord[1030];
static uint8_t wordLen = 0;

static uint8_t sdu[1030];

// ARQ 변수 나중에 바꿀 것 !!!!!!!!!!!!
uint8_t retxCnt = 0; // ARQ retransmission counter
uint8_t pdu[5];      // ACK PDU
uint8_t pduSize = 0;
// serial port interface
static Serial pc(USBTX, USBRX);
static uint8_t myDestId;
uint8_t verseID = 0; //initial value 0

// 새로 추가함
uint8_t cond_IDinput; // 0 - no ID, 1 - ID input is finished

static uint8_t input_thisId; // 나의 id
// destination sdu
static uint8_t destinationWord[10];
static uint8_t destinationLen = 0;

static uint8_t destinationsdu[10];
//static uint8_t check = 0;

char destinationString[12];


//static uint8_t *dataPtr = L3_LLI_getMsgPtr();
static uint8_t size = L3_LLI_getSize();

static uint8_t flag_needPrint = 1;


// application event handler : generating SDU from keyboard input
static void L3service_processInputWord(void)
{
    char c = pc.getc();

    if (main_state == STATE_IDLE && !L3_event_checkEventFlag(ReqCON_Send))
    {
        // 입력받고 변경하기 char 를 intf로, condition =1로
        //pc.printf(":: ID for the destination ji : ");
        if (c == '\n' || c == '\r')
        {
            //pc.printf(":: ready2 : ");
            destinationWord[destinationLen++] = '\0';
            //L3_event_setEventFlag(SDU_Rcvd); //하나 만들기
            //sprintf(destinationString, "%s", destinationWord);
            //pc.printf("%s", destinationWord);
            myDestId = atoi((const char*)destinationWord);
            destinationLen = 0;
            //cond_IDinput = 1;
            pc.printf("mydestnode : %i\n", myDestId);
            L3_event_setEventFlag(ReqCON_Send); //하나 만들기
        
            // myDestId=atoi(destinationWord);//a to i Tmrl
            
            //pc.printf("mydestnode : %i\n", myDestId);
            //debug_if(DBGMSG_L3, "destination is ready! ::: %s\n", myDestId);
        }
        else
        {
            //pc.printf(":: readye : ");
            
            destinationWord[destinationLen++] = c;

            //destinationWord[destinationLen++] = '\0';
            //L3_event_setEventFlag(SDU_Rcvd);
            //sprintf(destinationString, "%s", destinationWord);
            //pc.printf("hihi %s", destinationWord);
            //myDestId = atoi(destinationString);

            
            
            //pc.printf("mynode,dest : %i\n", myDestId);
           // destinationWord[wordLen++] = c;
        }
        //pc.printf(":: read4ye : ");
        //cond_IDinput = 1;
    }
    else if (main_state == STATE_CHAT &&
             !L3_event_checkEventFlag(SDU_Rcvd))
    {
        //pc.printf(":: !!!!!!!!!!!1chat ready!!!!!!!!!!! : ");
        if (c == '\n' || c == '\r')
        {
            //pc.printf(":: chat ready!!!!!!!!!!! : ");
            originalWord[wordLen++] = '\0';

            //myDestId = atoi((const char*)destinationWord);
            //wordLen = 0;
            //cond_IDinput = 1;
            //pc.printf("mydestnode : %i\n", myDestId);
            pc.printf("word is %s\n", originalWord);
            L3_event_setEventFlag(SDU_Rcvd); //하나 만들기

            /*
            originalWord[wordLen++] = '\0';
            L3_event_setEventFlag(SDU_Rcvd);
            debug_if(DBGMSG_L3, "word is ready! ::: %s\n", originalWord);*/
        }
        else
        {
            originalWord[wordLen++] = c;
            //pc.printf(":: chat ready???????????: ");
            /*
            originalWord[wordLen++] = c;
            if (wordLen >= L3_MAXDATASIZE - 1)
            {
                originalWord[wordLen++] = '\0';
                L3_event_setEventFlag(SDU_Rcvd);
                pc.printf("\n max reached! word forced to be ready :::: %s\n", originalWord);
            }*/
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

// void로 하기
void L3_initFSM(uint8_t input_thisId)
{

    input_thisId = input_thisId;
    // initialize service layer
    pc.attach(&L3service_processInputWord, Serial::RxIrq);

   pc.printf("Give a word to send initfsm: ");
}

void L3_FSMrun(void)
{
   // uint8_t flag_needPrint = 1;
    //uint8_t prev_state = 0;
    

    //pc.printf("ji: ");    
    if (prev_state != main_state)
    {
        //pc.printf("ji: ");   
        debug_if(DBGMSG_L3, "[L3] State transition from %i to %i\n", prev_state, main_state);
        prev_state = main_state;
    }

    // FSM should be implemented here! ---->>>>
    switch (main_state)
    {
    // IDLE STATE
    case STATE_IDLE:
          
        // b
        if (L3_event_checkEventFlag(ReqCON_Rcvd)){
            //{pc.printf("main"); 

            conID = L3_LLI_getSrcId();
            
            // debug("\n -------------------------------------------------\nRCVD MSG : %s (length:%i)\n -------------------------------------------------\n", dataPtr, size);

            pc.printf("\nCHAT으로 가기 위한 여정의 시작..");
            // pc.printf("\n -------------------------------------------------\nRCVD from %i : %s (length:%i, seq:%i)\n -------------------------------------------------\n", srcId, L3_msg_getWord(dataPtr), size, L3_msg_getSeq(dataPtr));
            //  pc.printf("Give a word to send : ");

            // pdu
            // L3_msg_encodeAck(arqAck, L3_msg_getSeq(dataPtr));

            //conID = myDestId; // 받은 ID가 내 목적지가 됨
            myDestId = conID;
            // 처음 받자마자 accept setCONPDU를 보내야 함

            Msg_encodeCONPDU(pdu, MSG_RSC_Set, MSG_ACP_ACCEPT); // srcID로 한 이유는 처음 받은 애한테는 무조건 받아야 해서..
            L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, myDestId);

            // main_state = MAINSTATE_TX;
            main_state = STATE_CON_WAIT;
            flag_needPrint = 1; // flag = 1 일 때 채팅 상태로 넘어감!!

            // L3_event_clearEventFlag(L3_event_dataRcvd);

            L3_event_clearEventFlag(ReqCON_Rcvd);
        }

        // a: 목적지 설정하는 거
        else if (L3_event_checkEventFlag(ReqCON_Send))
        {

            //pc.printf(":: ID for the destination : ");

            // msg header setting
            // PDU라 나중에 수정할 것 !!
            // strcpy((char *)sdu, (char *)originalWord);
            // debug("[L3] msg length : %i\n", wordLen);
            Msg_encodeCONPDU(pdu, MSG_RSC_Req, MSG_ACP_ACCEPT);
            pc.printf("[MAIN] sending to %i, PDU context : %i %i %i\n", myDestId, pdu[0], pdu[1], pdu[2]);
            
            L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, myDestId); // reqcon을 만들고 보내야 하는데 이건 뭐지?
            conID = myDestId;

            // main_state = MAINSTATE_TX;
            main_state = STATE_CON_WAIT;
            //flag_needPrint = 1;

            //wordLen = 0;
            //L3_event_clearEventFlag(ReqCON_Send); // 작업 완료

            // debug_if(DBGMSG_L3, "[L3] sending msg....\n");
            //wordLen = 0;

            // pc.printf("Give a word to send : ");

            L3_event_clearEventFlag(ReqCON_Send);
        }

        // IDLE 돌아오는거
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

        /*m
        else if (L3_event_checkEventFlag(Other_PDU_Rcvd))
        {
            pc.printf("m");
            L3_event_clearEventFlag(Other_PDU_Rcvd);
        }
        */

        /*
        // k
        else if (flag_needPrint == 1)
        {

            flag_needPrint = 0;
            pc.printf("CplDIS 받음 ");
            pc.printf("\n -------------------------------------------------\nRCVD from %i : %s (length:%i, seq:%i)\n -------------------------------------------------\n", srcId, L3_msg_getWord(dataPtr), size, L3_msg_getSeq(dataPtr));
            pc.printf("Give Word to Send: ");
        }
        */
        break;

    // CON STATE
    case STATE_CON_WAIT:

        // c, SDU 생성 event 안넣었음(위에 있는 함수)
        if (L3_event_checkEventFlag(SetCON_Accept_Rcvd))
        {
            verseID = L3_LLI_getSrcId(); // accept의 id
            if (verseID != conID)
            {
                pc.printf(" NO! (ID is not matching (verseID : %i, conID : %i))\n", verseID, conID);
            }
            else
            {
                pc.printf("STATE CHANGED CON 2 CHAT \n");

                // pdu
                // L3_msg_encodeAck(arqAck, L3_msg_getSeq(dataPtr));

                // cplCON을 보내야 함
                Msg_encodeCONPDU(pdu, MSG_RSC_Cpl, MSG_ACP_ACCEPT);
                L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, myDestId);

                // main_state = MAINSTATE_TX;
                main_state = STATE_CHAT;
                // flag_needPrint = 1;
                pc.printf("Give Word to Send: ");

                // L3_event_clearEventFlag(L3_event_dataRcvd);
            }
            L3_event_clearEventFlag(SetCON_Accept_Rcvd);
        }

        // d
        else if (L3_event_checkEventFlag(SetCON_Reject_Rcvd))
        {
            verseID = L3_LLI_getSrcId(); // accept의 id
            if (verseID != conID)
            {
                // check = 1;
                 pc.printf(" NO ");
            }
            else
            {

                pc.printf("STATE CHANGED CON 2 IDLE ");

                //cond_IDinput = 0;//??

                main_state = STATE_IDLE;
                L3_event_clearEventFlag(SetCON_Reject_Rcvd);
            }
        }

        // e (timer)
        else if (L3_event_checkEventFlag(CplCON_Rcvd)) // data TX finished
        {
            verseID = L3_LLI_getSrcId(); // accept의 id
            if (verseID != conID)
            {
                pc.printf(" NO ");
            }
            else
            {
                pc.printf("Timer Starts!! ");
                L3_timer_Chat_Timer();

                main_state = STATE_CHAT;
                pc.printf("Give Word to Send this state is con_Wait: ");
                L3_event_clearEventFlag(CplCON_Rcvd);
            }
        }

        // CON 대기로 돌아오는 거
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

        /*       else if (L3_event_checkEventFlag(ReqCON_Other_Rcvd))
        {
            pc.printf("f");
            L3_event_clearEventFlag(ReqCON_Other_Rcvd);
        }*/ 
 

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

        /*        else if (L3_event_checkEventFlag(Other_PDU_Rcvd))
        {
            pc.printf("m");
            L3_event_clearEventFlag(Other_PDU_Rcvd);
        }*/

        break;

    // DIS STATE
    case STATE_DIS_WAIT:

        // l
        if (L3_event_checkEventFlag(CplDIS_Rcvd))
        {
            {
                verseID = L3_LLI_getSrcId(); // accept의 id
                if (verseID != conID)
                {
                    pc.printf(" NO ");
                }
                else
                {
                    pc.printf("STATE CHANGED DIS 2 IDLE & ComDIS");
                    //cond_IDinput = 0;
                    main_state = STATE_IDLE;
                    L3_event_clearEventFlag(CplDIS_Rcvd);
                }
            }
        }

        // k
        else if (L3_event_checkEventFlag(SetDIS_Rcvd))
        {
            verseID = L3_LLI_getSrcId(); // accept의 id
            if (verseID != conID)
            {
                pc.printf(" NO ");
            }

            else
            {
                pc.printf("STATE CHANGED DIS 2 IDLE & SetupDIS");

                // CplDISPDU 보내기
                Msg_encodeDISPDU(pdu, MSG_RSC_Cpl);
                L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, myDestId);

               // cond_IDinput = 0;
                main_state = STATE_IDLE;
                L3_event_clearEventFlag(SetDIS_Rcvd);
            }
        }

        // DIS 대기로 돌아오는 거
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

        /*        else if (L3_event_checkEventFlag(ReqCON_Other_Rcvd))
        {
            pc.printf("f");
            L3_event_clearEventFlag(ReqCON_Other_Rcvd);
        }*/ 


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

        /*m
        else if (L3_event_checkEventFlag(Other_PDU_Rcvd))
        {
            pc.printf("m");
            L3_event_clearEventFlag(Other_PDU_Rcvd);
        }*/
        break;

    // CHAT STATE
    case STATE_CHAT:

        // i
        if (L3_event_checkEventFlag(Chat_Timer_Expire))
        {
            verseID = L3_LLI_getSrcId(); // accept의 id
            if (verseID != conID)
            {
                pc.printf(" NO ");
            }
            else
            {
                Msg_encodeDISPDU(pdu, MSG_RSC_Req);
                L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, myDestId);

                main_state = STATE_DIS_WAIT;
                L3_event_clearEventFlag(Chat_Timer_Expire);
               
            }
            flag_needPrint == 1;
        }

        // j
        else if (L3_event_checkEventFlag(ReqDIS_Rcvd))
        {
            verseID = L3_LLI_getSrcId(); // accept의 id
            if (verseID != conID)
            {
                pc.printf(" NO ");
            }

            else
            {
                pc.printf("STATE CHANGE 2 DIC CON ");

                // set
                Msg_encodeDISPDU(pdu, MSG_RSC_Set);
                L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, myDestId);

                // main_state = MAINSTATE_TX;
                main_state = STATE_DIS_WAIT;
                // flag_needPrint = 1;

                // L3_event_clearEventFlag(L3_event_dataRcvd);

                L3_event_clearEventFlag(ReqDIS_Rcvd);
            }
            flag_needPrint == 1;
           //pc.printf("\n -------------------------------------------------\nRCVD from dest-j  : %s (length:%i, seq:%i)\n -------------------------------------------------\n",  L3_msg_getWord(dataPtr), size, L3_msg_getSeq(dataPtr));
        }
        

        // f

        else if (L3_event_checkEventFlag(ReqCON_Rcvd)) // 다른 ID로부터 req를 받으면
        {
           verseID = L3_LLI_getSrcId(); // accept의 id
            if (verseID != conID)
            {
                pc.printf("SetCON reject ");

                uint8_t srcId = L3_LLI_getSrcId();

                // setCON reject pdu 생성
                Msg_encodeCONPDU(pdu, MSG_RSC_Set, MSG_ACP_REJECT);
                L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, verseID);

                flag_needPrint = 1;

                pc.printf("\n other id \n");
                // L3_event_clearEventFlag(L3_event_dataRcvd);

              
            }

            else
            {
                pc.printf(" wow e ge dweh ne");
            }
           // pc.printf("\n -------------------------------------------------\nRCVD from dest-f  : %s (length:%i, seq:%i)\n -------------------------------------------------\n",  L3_msg_getWord(dataPtr), size, L3_msg_getSeq(dataPtr));
              L3_event_clearEventFlag(ReqCON_Rcvd);
        }

        // g
        else if (L3_event_checkEventFlag(SDU_Rcvd)) // if data needs to be sent (keyboard input)
        {
            verseID = L3_LLI_getSrcId();
            uint8_t* dataPtr=L3_LLI_getMsgPtr();
            if (verseID != conID)
            {
                pc.printf(" NO ");
            }
            else
            {
                 // msg header setting
                pduSize = Msg_encodeCHAT(sdu, originalWord, wordLen);
                // Msg_encodeCHAT
                L3_LLI_dataReqFunc(sdu, pduSize, myDestId);

                pc.printf("[MAIN] sending to %i, pduSize:%i\n", myDestId, pduSize);
                //dataPtr=L3_LLI_getMsgPtr();
                 pc.printf("\n -------------------------------------------------\nRCVD from dest-inner g  : %s (length:%i, seq:%i)\n -------------------------------------------------\n",  L3_msg_getWord(dataPtr), size, L3_msg_getSeq(dataPtr));
            //  pc.printf("Give a word to send : ");
                
                flag_needPrint = 1;
                wordLen = 0;
                
            }
            flag_needPrint == 1;
            pc.printf("\n -------------------------------------------------\nRCVD from dest-g  : %s (length:%i, seq:%i)\n -------------------------------------------------\n",  L3_msg_getWord(dataPtr), size, L3_msg_getSeq(dataPtr));
           //L3_event_clearEventFlag(SDU_Rcvd); //없애면 여기서 못나가고 무한로딩됨...
           
           L3_event_clearEventFlag(SDU_Rcvd);
            //=L3_LLI_getMsgPtr();
           //dataPtr=L3_LLI_getMsgPtr();
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
            else if (L3_event_checkEventFlag(Chat_Rcvd)) // if data needs to be sent (keyboard input)
            {
               // uint8_t srcId = arqLLI_getSrcId();
                uint8_t* msg;
                uint8_t* dataPtr=L3_LLI_getMsgPtr();
                uint8_t size = L3_LLI_getSize();
                pc.printf("hihihihihihihih!!!!!");
                verseID = L3_LLI_getSrcId();
                //dataPtr=L3_LLI_getMsgPtr(); 
                msg = L3_msg_getChat(dataPtr);
                if (verseID != conID)
                {
                    pc.printf(" NO ");

                }
                else
                {
                    pc.printf("!!!!!!!!!!!!hihihihihihihih!!!!!");

                    debug("\n -------------------------------------------------\nRCVD MSG : %s (length:%i)\n -------------------------------------------------\n",
                          msg, size);
                    //pc.printf("\n -------------------------------------------------\nRCVD from dest-1  : %s (length:%i, seq:%i)\n -------------------------------------------------\n",  
                    //L3_msg_getWord(dataPtr), size, L3_msg_getSeq(dataPtr));

                    pc.printf("Give a word to send : \n");

                    
                }
                //pc.printf("\n -------------------------------------------------\nRCVD from dest-2  : %s (length:%i, seq:%i)\n -------------------------------------------------\n",  L3_msg_getWord(dataPtr), size, L3_msg_getSeq(dataPtr));
                flag_needPrint == 1;
                L3_event_clearEventFlag(Chat_Rcvd);
            }
            // CHAT으로 돌아오는 거
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

            /*m
            else if (L3_event_checkEventFlag(Other_PDU_Rcvd))
            {
                pc.printf("m");
                L3_event_clearEventFlag(Other_PDU_Rcvd);
            }*/ 
            else if (flag_needPrint == 1)
                {
                    pc.printf("Give a word to send : ");
                    flag_needPrint = 0;
                }  
            break;

        default:
            break;
        }
    }
