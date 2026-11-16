#include <xc.h> // include processor files - each processor file is guarded.  

typedef struct RECIEVED_MESSAGE {
    uint8_t CHANNEL_MSB;
    uint8_t CHANNEL_LSB;
    uint8_t CMD_CLASS;
    uint8_t CMD_FUNC;
    uint8_t LEN_MSB;
    uint8_t LEN_LSB;
    
} RECIEVED_MESSAGE;

typedef struct METATEDS_TEMPLATE {
    uint8_t TEDSID[6];
    uint8_t UUID[12];
    uint8_t MAXCHAN[4];
} METATEDS_TEMPLATE;


typedef struct TRANSDUCERCHANNEL_TEDS_TEMPLATE {
    uint8_t TEDSID[6];
    uint8_t CHANNEL_TYPE[3];
    uint8_t UNITS[12];
    uint8_t LOW_RANGE_LIMIT[6];
    uint8_t HIGH_RANGE_LIMIT[6];
    uint8_t DATA_MODEL[3];
    uint8_t DATA_MODEL_LENGTH[3];
    uint8_t MODEL_SIG_BITS[4];
} TRANSDUCERCHANNEL_TEDS_TEMPLATE;





#ifndef HEADER_C
#define	HEADER_C

#ifdef	__cplusplus
extern "C" {
#endif





#ifdef	__cplusplus
}
#endif

#endif	/* HEADER_C */

