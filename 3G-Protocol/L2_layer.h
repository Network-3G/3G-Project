#define L2_STATE_RX 0
#define L2_STATE_TX 1

#define L2_ERR_NONE 0
#define L2_ERR_WRONGSTATE 1
#define L2_ERR_HWERROR 2
#define L2_ERR_SIZE 3

int L2_dataReq(uint8_t *dataPtr, uint8_t size, uint8_t destId);
void L2_init(uint8_t id, void (*dataCnfFunc)(int), void (*dataIndFunc)(uint8_t, uint8_t *, uint8_t, uint8_t));
int16_t L2_getDataRssi(void);
int8_t L2_getDataSnr(void);
int L2_configSrcId(uint8_t id);