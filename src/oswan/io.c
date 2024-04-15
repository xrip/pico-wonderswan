////////////////////////////////////////////////////////////////////////////////
// I/O ports
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <io.h>
#include <fcntl.h>
#if 0
#include <conio.h>
#endif
#include <time.h>
//#include "log.h"
#include "rom.h"
#include "./nec/nec.h"
#include "initialio.h"
#include "ieeprom.h"
#include "gpu.h"
#include "psram_spi.h"
//#include "audio.h"

#if 0
extern	uint8	*externalEeprom;
#else
//extern	uint8	externalEeprom[131072];
#endif
extern	uint32	externalEepromAddressMask;
extern	uint32	romAddressMask;

#if 0
uint8	*ws_ioRam=NULL;
#else
uint8	ws_ioRam[0x100];
#endif

uint8	ws_key_start;
uint8	ws_key_left;
uint8	ws_key_right;
uint8	ws_key_up;
uint8	ws_key_down;
uint8	ws_key_button_1;
uint8	ws_key_button_2;
uint8	ws_key_flipped;

#if 1
uint8	ws_key_menu;
#endif

int		rtcDataRegisterReadCount=0;

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
void ws_io_reset(void)
{
	ws_key_start=0;
	ws_key_left=0;
	ws_key_right=0;
	ws_key_up=0;
	ws_key_down=0;
	ws_key_button_1=0;
	ws_key_button_2=0;

#ifdef __cplusplus
	for (int i=0;i<0x100;i++)
#else
	int i;
	for (i=0;i<0x100;i++)
#endif
		ws_ioRam[i]=initialIoValue[i];
	for (i=0;i<0xc9;i++)
		cpu_writeport(i,initialIoValue[i]);

	rtcDataRegisterReadCount=0;
}
////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
void ws_io_init(void)
{
#if 0
	if (ws_ioRam==NULL)
		ws_ioRam=(uint8*)malloc(0x100);
#endif

	ws_io_reset();
	ws_key_flipped=0;
}
////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
void ws_io_flipControls(void)
{
	ws_key_flipped= (!ws_key_flipped);
}
////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
void ws_io_done(void)
{
#if 0
	if (ws_ioRam==NULL)
		free(ws_ioRam);
#else
	memset(ws_ioRam, 0x00, sizeof(ws_ioRam));
#endif
}
////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
uint8_t cpu_readport(uint8_t port)
{
	int w1,w2;

	switch (port)
	{
	case 0x4e:
	case 0x4f:
	case 0x50:
	case 0x51:  
	case 0x80:
	case 0x81:
	case 0x82:
	case 0x83:
	case 0x84:
	case 0x85:
	case 0x86:
	case 0x87:
	case 0x88:
	case 0x89:
	case 0x8a:
	case 0x8b:
	case 0x8c:
	case 0x8d:
	case 0x8e:
	case 0x8f:
	case 0x90:
	case 0x91:
	case 0x92:
	case 0x93:
	case 0x94:
//				return(ws_audio_port_read(port));
                return 0xff;
    case 0xaa:	return 0xff;
	case 0xb3:	// ???
				if (ws_ioRam[0xb3]<0x80)
					return 0;
				
				if (ws_ioRam[0xb3]<0xc0)
					return 0x84;
				
				return 0xc4;
	case 0xb5:	w1=ws_ioRam[0xb5];
				if(w1&0x40)
				{
					w2=0x00;
					if (ws_key_flipped)
						w2=(ws_key_start<<1);
					else
						w2=(ws_key_start<<1)|(ws_key_button_1<<2)|(ws_key_button_2<<3);
					return (uint8)((w1&0xf0)|w2);
				}
				if(w1&0x20)
				{
					w2=0x00;
					if (ws_key_flipped)
						w2=(ws_key_button_1)|(ws_key_button_2<<2);
					else
						w2=(ws_key_up<<0)|(ws_key_right<<1)|(ws_key_down<<2)|(ws_key_left<<3);
					return (uint8)((w1&0xf0)|w2);
				}
				if(w1&0x10)
				{
					w2=0x00;
					if (ws_key_flipped)
						w2=(ws_key_up<<1)|(ws_key_right<<2)|(ws_key_down<<3)|(ws_key_left);
					return (uint8)((w1&0xf0)|w2);
				}
			 break;
	case 0xbe:	// internal eeprom status/command register
				// ack eeprom write
				 if(ws_ioRam[0xbe]&0x20)
					 return ws_ioRam[0xbe]|2;

				 // ack eeprom read
				 if(ws_ioRam[0xbe]&0x10)
					 return ws_ioRam[0xbe]|1;
				 
				 // else ack both
				 return ws_ioRam[0xbe]|3;
	case 0xba:	// eeprom even uint8_t read
				w1=((((uint16)ws_ioRam[0xbd])<<8)|((uint16)ws_ioRam[0xbc]));
				w1=(w1<<1)&0x3ff;
				return internalEeprom[w1];
	case 0xbb:	// eeprom odd uint8_t read
				w1=((((uint16)ws_ioRam[0xbd])<<8)|((uint16)ws_ioRam[0xbc]));
				w1=((w1<<1)+1)&0x3ff;
				return internalEeprom[w1];
	case 0xc0 : // ???
				return ((ws_ioRam[0xc0]&0xf)|0x20);

	case 0xc4:	// external eeprom even uint8_t read
				w1=(((((uint16_t)ws_ioRam[0xc7])<<8)|((uint16_t)ws_ioRam[0xc6]))<<1)&(externalEepromAddressMask);
                return read8psram(w1);
				//return externalEeprom[w1];

	case 0xc5:	// external eeprom odd uint8_t read
				w1=(((((uint16_t)ws_ioRam[0xc7])<<8)|((uint16_t)ws_ioRam[0xc6]))<<1)&(externalEepromAddressMask);
                return read8psram(w1+1);
                //				return externalEeprom[w1+1];

	case 0xc8:	// external eeprom status/command register
				// ack eeprom write
				 if(ws_ioRam[0xc8]&0x20)
					 return ws_ioRam[0xc8]|2;

				 // ack eeprom read
				 if(ws_ioRam[0xc8]&0x10)
					 return ws_ioRam[0xc8]|1;
				 
				 // else ack both
				 return ws_ioRam[0xc8]|3;

	case 0xca : // RTC Command and status register
				// set ack to always 1
				return (ws_ioRam[0xca]|0x80);
	case 0xcb : // RTC data register
#if 0
				if(ws_ioRam[0xca]==0x15)	// get time command 
				{
					struct tm *newtime;
					time_t long_time;
					time( &long_time );                
					newtime = localtime( &long_time ); 

					#define  BCD(value) ((value/10)<<4)|(value%10)
					switch(rtcDataRegisterReadCount)
					{
					  case 0:	rtcDataRegisterReadCount++;
								return BCD(newtime->tm_year-100);
					  case 1:	rtcDataRegisterReadCount++;
								return BCD(newtime->tm_mon);
					  case 2:	rtcDataRegisterReadCount++;
								return BCD(newtime->tm_mday);
					  case 3:	rtcDataRegisterReadCount++;
								return BCD(newtime->tm_wday);
					  case 4:	rtcDataRegisterReadCount++;
								return BCD(newtime->tm_hour);
					  case 5:	rtcDataRegisterReadCount++;
								return BCD(newtime->tm_min);
					  case 6:	rtcDataRegisterReadCount=0;
								return BCD(newtime->tm_sec);
					}
					return 0;
				}
				else
				{
					// set ack
					return (ws_ioRam[0xcb]|0x80);
				}
#else
				break;
#endif
	}
	return(ws_gpu_port_read(port));
}
////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
void cpu_writeport(uint32_t port,uint8_t value)
{
	int w1,w2;
	int unknown_io_port=0;

	if (ws_ioRam[port]==value)
		return;
	
	ws_ioRam[port]=value;
	
	switch (port)
	{
	case 0x4e:
	case 0x4f:
	case 0x50:
	case 0x51:  
	case 0x80:
	case 0x81:
	case 0x82:
	case 0x83:
	case 0x84:
	case 0x85:
	case 0x86:
	case 0x87:
	case 0x88:
	case 0x89:
	case 0x8a:
	case 0x8b:
	case 0x8c:
	case 0x8d:
	case 0x8e:
	case 0x8f:
	case 0x90:
	case 0x91:
	case 0x92:
	case 0x93:
	case 0x94:
//				ws_audio_port_write(port,value);
				break;
	case 0x48:	// DMA

				// bit 7 set to start dma transfer
				if(value&0x80)
				{
					int dma_start = (((uint32_t)ws_ioRam[0x41])<<8)|(((uint32_t)ws_ioRam[0x40]))|(((uint32_t)ws_ioRam[0x42])<<16);
					int dma_end   = (((uint32_t)ws_ioRam[0x45])<<8)|(((uint32_t)ws_ioRam[0x44]))|(((uint32_t)ws_ioRam[0x43])<<16);
					int dma_size  = (((uint32_t)ws_ioRam[0x47])<<8)|(((uint32_t)ws_ioRam[0x46]));
#ifdef __cplusplus
					for(int ix=0;ix<dma_size;ix++)
#else
					int ix;
					for(ix=0;ix<dma_size;ix++)
#endif
						cpu_writemem20(dma_end++,cpu_readmem20(dma_start++));
					
					ws_ioRam[0x47]=0;
					ws_ioRam[0x46]=0;
					ws_ioRam[0x41]=(uint8_t)(dma_start>>8);
					ws_ioRam[0x40]=(uint8_t)(dma_start&0xff);
					ws_ioRam[0x45]=(uint8_t)(dma_end>>8);
					ws_ioRam[0x44]=(uint8_t)(dma_end&0xff);
					ws_ioRam[0x48]=0; 
				}
				break;
   case 0xba: w1=(((((uint16_t)ws_ioRam[0xbd])<<8)|((uint16_t)ws_ioRam[0xbc])));
			  w1=(w1<<1)&0x3ff;
			  internalEeprom[w1]=value;
			  return;
	  
	case 0xbb: w1=(((((uint16_t)ws_ioRam[0xbd])<<8)|((uint16_t)ws_ioRam[0xbc])));
			   w1=((w1<<1)+1)&0x3ff;
			   internalEeprom[w1]=value;
			   return;

	case 0xc4: w1=(((((uint16_t)ws_ioRam[0xc7])<<8)|((uint16_t)ws_ioRam[0xc6]))<<1)&externalEepromAddressMask;
            write8psram(w1, value);
    //			   externalEeprom[w1]=value;
			   return;
	  
	case 0xc5: w1=(((((uint16_t)ws_ioRam[0xc7])<<8)|((uint16_t)ws_ioRam[0xc6]))<<1)&externalEepromAddressMask;
            write8psram(w1+1, value);
    //			   externalEeprom[w1+1]=value;
			   return;

	case 0xca:	if(value==0x15)
					rtcDataRegisterReadCount=0; 
				break;
			break;
	default:
			unknown_io_port=1;
	}
	ws_gpu_port_write(port,value);
//	if ((ws_gpu_unknownPort)&&(unknown_io_port))
//	{
//		fprintf(log_get(),"io: writing 0x%.2x to unknown port 0x%.2x\n",value,port); 
//	}
}
