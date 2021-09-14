#ifndef _CONFIG_H_
#define _CONFIG_H_

#define ANALOG_FILTER 1 
#define ANALOG_THRESHOLD 1

#define MOTA_END K10
#define MOTA_ENDLEVEL 1
#define MOTA_A K12
//#define MOTA_B K11 // K11 is used by DMX512 TX
#define MOTA_B K9

#define LAMP1 K6

//DMX config:
#define DMX_UART_NUM 	AUXSERIAL_NUM // 2 for Versa
#define DMX_UART_PIN	AUXSERIAL_TX  // K11 for Versa
#define DMX_NBCHAN 	4

#endif // _CONFIG_H_

