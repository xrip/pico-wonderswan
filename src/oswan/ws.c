////////////////////////////////////////////////////////////////////////////////
// Wonderswan emulator
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
// 07.04.2002: speed problems partially fixed
// 13.04.2002: Set cycles by line to 256 (according to toshi)
//			   this seems to work well in most situations with
//			   the new nec v30 cpu core
//
//
//
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <io.h>
#if 0
#include <fcntl.h>
#include <conio.h>
#endif
#include "./nec/nec.h"
#include "./nec/necintrf.h"
#include "memory.h"
#include "gpu.h"
#include "ws.h"


//#define DEBUG
#undef DEBUG
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

uint32	ws_cycles;
uint32	ws_skip;
uint32	ws_cyclesByLine=677;

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
void ws_patchRom(void)
{

	uint8	*rom=memory_getRom();
	uint32	romSize=memory_getRomSize();

#if 0
	fprintf(log_get(),"developper Id: 0x%.2x\nGame Id: 0x%.2x\n",rom[romSize-10],rom[romSize-8]);
#endif

	if((rom[romSize-10]==0x01)&&(rom[romSize-8]==0x27)) // Detective Conan 
	{
		// WS cpu is using cache/pipeline or
		//   there's protected ROM bank where 
		//   pointing CS 
		
		rom[0xfffe8]=0xea;
		rom[0xfffe9]=0x00;
		rom[0xfffea]=0x00;
		rom[0xfffeb]=0x00;
		rom[0xfffec]=0x20;
		
	}

#if 1
	ws_cyclesByLine=256;
#else
	ws_cyclesByLine=677;

	if((( rom[romSize-10]==0x01)&&(rom[romSize-8]==0x23))|| //Final Lap special
		((rom[romSize-10]==0x00)&&(rom[romSize-8]==0x17))|| //turn tablis
		((rom[romSize-10]==0x01)&&(rom[romSize-8]==0x08))|| //klonoa
		((rom[romSize-10]==0x26)&&(rom[romSize-8]==0x01))|| //ring infinity
		((rom[romSize-10]==0x01)&&(rom[romSize-8]==0x04))|| //puyo puyo 2
		((rom[romSize-10]==0x1b)&&(rom[romSize-8]==0x03))|| //rainbow islands
		((rom[romSize-10]==0x28)&&(rom[romSize-8]==0x01))|| //FF1
		((rom[romSize-10]==0x28)&&(rom[romSize-8]==0x02)))  //FF2
			ws_cyclesByLine=837;

	if(((rom[romSize-10]==0x01)&&(rom[romSize-8]==0x3)))    //digimon tamers
		ws_cyclesByLine=574;
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
int ws_init(uint8  *rom, size_t romSize)
{
	ws_memory_init(rom, romSize);
	ws_patchRom();
	ws_io_init();
//	ws_audio_init();
	ws_gpu_init();

	if (ws_rotated())
		ws_io_flipControls();

	return(1);
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
void ws_reset(void)
{
	ws_memory_reset();
	ws_io_reset();
//	ws_audio_reset();
	ws_gpu_reset();
	nec_reset(NULL);
	nec_set_reg(NEC_SP,0x2000);
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
//#define DEBUG
#undef DEBUG
int ws_executeLine(uint8 *framebuffer, int renderLine)
{
	int drawWholeScreen=0;
 
//	ws_audio_process();

	// update scanline register
	ws_ioRam[2]=ws_gpu_scanline;

	ws_cycles=nec_execute((ws_cyclesByLine>>1)+(rand()&7));

	ws_cycles+=nec_execute((ws_cyclesByLine>>1)+(rand()&7));
#ifdef DEBUG
sprintf(buf, "%d", ws_cycles);
pgDebug(buf, 2);
#endif
	if(ws_cycles>=ws_cyclesByLine+ws_cyclesByLine)
		ws_skip=ws_cycles/ws_cyclesByLine;
	else
		ws_skip=1;
	ws_cycles%=ws_cyclesByLine;

#ifdef DEBUG
sprintf(buf, "%d", ws_cycles);
pgDebug(buf, 3);
#endif

	for(uint32 uI=0;uI<ws_skip;uI++)
	{
	   if (renderLine)
		   ws_gpu_renderScanline(framebuffer);

		ws_gpu_scanline++;
		if(ws_gpu_scanline==144) {
			drawWholeScreen=1;
//			renderer_update_sound();
//			render_screen();
		}

	}

#ifdef DEBUG
sprintf(buf, "ws_gpu_scanline = %d", ws_gpu_scanline);
pgDebug(buf, 4);
#endif
	if(ws_gpu_scanline>158)
	{
		ws_gpu_scanline=0;
		{
			if((ws_ioRam[0xb2]&32))/*VBLANK END INT*/ 
			{
				if(ws_ioRam[0xa7]!=0x35)/*Beatmania Fix*/
				{
					ws_ioRam[0xb6]&=~32;
					nec_int((ws_ioRam[0xb0]+5)*4);
				}
			}
		}
	}
	ws_ioRam[2]=ws_gpu_scanline;
	if(drawWholeScreen)
	{
	
		if(ws_ioRam[0xb2]&64) /*VBLANK INT*/
		{
#ifdef DEBUG
pgDebug("VBLANK INT", 5);
#endif
			ws_ioRam[0xb6]&=~64;
			nec_int((ws_ioRam[0xb0]+6)*4);
		}
	}
	if(ws_ioRam[0xa4]&&(ws_ioRam[0xb2]&128)) /*HBLANK INT*/
	{
#ifdef DEBUG
pgDebug("VBLANK INT", 6);
#endif
		if(!ws_ioRam[0xa5])
			ws_ioRam[0xa5]=ws_ioRam[0xa4];
		if(ws_ioRam[0xa5]) 
			ws_ioRam[0xa5]--;
		if((!ws_ioRam[0xa5])&&(ws_ioRam[0xb2]&128))
		{
			ws_ioRam[0xb6]&=~128;
			nec_int((ws_ioRam[0xb0]+7)*4);
		}
	}

	if((ws_ioRam[0x2]==ws_ioRam[0x3])&&(ws_ioRam[0xb2]&16)) /*SCANLINE INT*/
	{
#ifdef DEBUG
pgDebug("SCANLINE INT", 76);
#endif
		ws_ioRam[0xb6]&=~16;
		nec_int((ws_ioRam[0xb0]+4)*4);
	}

  return(drawWholeScreen);
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
void ws_done(void)
{
	ws_memory_done();
	ws_io_done();
//	ws_audio_done();
	ws_gpu_done();
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
void	ws_set_colour_scheme(int scheme)
{
	ws_gpu_set_colour_scheme(scheme);
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
void	ws_set_system(int system)
{
	if (system==WS_SYSTEM_COLOR)
		ws_gpu_forceColorSystem();
	else
	if (system==WS_SYSTEM_MONO)
		ws_gpu_forceMonoSystem();
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
#if 0
#define MacroLoadNecRegisterFromFile(F,R)        \
		read(fp,&value,sizeof(value));		\
	    nec_set_reg(R,value); 
#else
#define MacroLoadNecRegisterFromFile(F,R)        \
		if (ms_fread(&value,sizeof(value),1,F) < 1) goto ERROR;\
	    nec_set_reg(R,value); 
#endif

#if 0
int ws_loadSRAM(char *sram_path)
{
	uint16	version;
	uint16	crc=memory_getRomCrc();
	uint16	newCrc;
	unsigned	value;
	uint8	ws_newVideoMode;

	int fd = ms_fopen(sram_path, "r");
	if (fd < 0) {
		pgMessage("SRAM FILE not found.", MSG_WARN, 100);
		goto ERROR_PROC1;
	}

	ms_fread(&version, 2, 1, fd);
	if (version != PSWAN_SRAM_VERSION) {
#if 1
		pgMessage("SRAM FILE version error.", MSG_ERROR, 100);
		goto ERROR_PROC1;
#endif // �o�[�W�������Ƃɂ��܂������������

	}
	ms_fread(&newCrc, 2, 1, fd);
	if (newCrc!=crc)
	{
		pgMessage("SRAM FILE  CRC error.", MSG_ERROR, 100);
		goto ERROR_PROC1;
	}

	pgMessage("now loading....", MSG_INFO, 0);

	if ((ms_fread(ws_staticRam,65536,1,fd)) < 1) goto ERROR_PROC;
	ms_fclose(fd);
	
	// force a video mode change to make all tiles dirty
	ws_gpu_clearCache();

#ifdef DEBUG
	pgMessage("load successfully.", MSG_INFO, 100);
#endif
	return 1;

ERROR_PROC :
	pgMessage("SRAM FILE load failed.", MSG_ERROR, 100);
ERROR_PROC1 :
	ws_reset();
	return 0;
}
#endif

#if 0
int	ws_loadState(char *statepath)
{
	fprintf(log_get(),"loading %s\n",statepath);
	uint16	crc=memory_getRomCrc();
	uint16	newCrc;
	unsigned	value;
	uint8	ws_newVideoMode;

	int fp=open(statepath,O_BINARY|O_RDONLY);
	if (fp==NULL)
		return(0);
	read(fp,&newCrc,2);
	if (newCrc!=crc)
	{
		return(-1);
	}
	MacroLoadNecRegisterFromFile(fp,NEC_IP);
	MacroLoadNecRegisterFromFile(fp,NEC_AW);
	MacroLoadNecRegisterFromFile(fp,NEC_BW);
	MacroLoadNecRegisterFromFile(fp,NEC_CW);
	MacroLoadNecRegisterFromFile(fp,NEC_DW);
	MacroLoadNecRegisterFromFile(fp,NEC_CS);
	MacroLoadNecRegisterFromFile(fp,NEC_DS);
	MacroLoadNecRegisterFromFile(fp,NEC_ES);
	MacroLoadNecRegisterFromFile(fp,NEC_SS);
	MacroLoadNecRegisterFromFile(fp,NEC_IX);
	MacroLoadNecRegisterFromFile(fp,NEC_IY);
	MacroLoadNecRegisterFromFile(fp,NEC_BP);
	MacroLoadNecRegisterFromFile(fp,NEC_SP);
	MacroLoadNecRegisterFromFile(fp,NEC_FLAGS);
	MacroLoadNecRegisterFromFile(fp,NEC_VECTOR);
	MacroLoadNecRegisterFromFile(fp,NEC_PENDING);
	MacroLoadNecRegisterFromFile(fp,NEC_NMI_STATE);
	MacroLoadNecRegisterFromFile(fp,NEC_IRQ_STATE);
	
	read(fp,internalRam,65536);
	read(fp,ws_staticRam,65536);
	read(fp,ws_ioRam,256);
	read(fp,ws_paletteColors,8);
	read(fp,ws_palette,16*4*2);
	read(fp,wsc_palette,16*16*2);
	read(fp,&ws_newVideoMode,1);
	read(fp,&ws_gpu_scanline,1);
	read(fp,externalEeprom,131072);	

	ws_audio_readState(fp);
	close(fp);
	
	// force a video mode change to make all tiles dirty
	ws_gpu_clearCache();

	return(1);
}
#endif
#if 0
int	ws_loadState()
{
	unsigned	value;
	uint8	ws_newVideoMode;

	// STATE FILE NAME
	int ret = 0;
	char save_path[256];
	char *p;
	strcpy(save_path, ws_rom_path);
	p = strrchr(save_path, '.');
	*(++p) = 0;
	strcat(save_path, "sv0");

	// INITIALIZE AUDIO ENVIRONMENT
	ws_audio_init();

	// READ STATE FILE
	int fd;
	if ((fd = ms_fopen(save_path, "r")) < 0) return 0;

	// VERSION
	uint16	version;
	ms_fread(&version, 2, 1, fd);
	if (version != PSWAN_SAVE_VERSION) {
#if 1
		pgMessage("STATE FILE version error.", MSG_ERROR, 100);
		goto ERROR_PROC1;
#endif // �o�[�W�������Ƃɏ����������
	}

	// CRC
	uint16	crc=memory_getRomCrc();
	uint16	newCrc;
	if ((ms_fread(&newCrc, 1, 2, fd)) < 1) {
		pgMessage("STATE FILE CRC not matched.", MSG_ERROR, 100);
		goto ERROR_PROC1;
	}
	if (newCrc!=crc) return 0;

	// registers
	MacroLoadNecRegisterFromFile(fd,NEC_IP);
	MacroLoadNecRegisterFromFile(fd,NEC_AW);
	MacroLoadNecRegisterFromFile(fd,NEC_BW);
	MacroLoadNecRegisterFromFile(fd,NEC_CW);
	MacroLoadNecRegisterFromFile(fd,NEC_DW);
	MacroLoadNecRegisterFromFile(fd,NEC_CS);
	MacroLoadNecRegisterFromFile(fd,NEC_DS);
	MacroLoadNecRegisterFromFile(fd,NEC_ES);
	MacroLoadNecRegisterFromFile(fd,NEC_SS);
	MacroLoadNecRegisterFromFile(fd,NEC_IX);
	MacroLoadNecRegisterFromFile(fd,NEC_IY);
	MacroLoadNecRegisterFromFile(fd,NEC_BP);
	MacroLoadNecRegisterFromFile(fd,NEC_SP);
	MacroLoadNecRegisterFromFile(fd,NEC_FLAGS);
	MacroLoadNecRegisterFromFile(fd,NEC_VECTOR);
	MacroLoadNecRegisterFromFile(fd,NEC_PENDING);
	MacroLoadNecRegisterFromFile(fd,NEC_NMI_STATE);
	MacroLoadNecRegisterFromFile(fd,NEC_IRQ_STATE);
	
	// memories
	if (ms_fread(internalRam,      1,   65536, fd) < 1) goto ERROR;
	if (ms_fread(ws_staticRam,     1,   65536, fd) < 1) goto ERROR;
	if (ms_fread(ws_ioRam,         1,     256, fd) < 1) goto ERROR;
	if (ms_fread(ws_paletteColors, 1,       8, fd) < 1) goto ERROR;
	if (ms_fread(ws_palette,       1,  16*4*2, fd) < 1) goto ERROR;
	if (ms_fread(wsc_palette,      1, 16*16*2, fd) < 1) goto ERROR;
	if (ms_fread(&ws_newVideoMode, 1,       1, fd) < 1) goto ERROR;
	if (ms_fread(&ws_gpu_scanline, 1,       1, fd) < 1) goto ERROR;
	if (ms_fread(externalEeprom,   1,  131072, fd) < 1) goto ERROR;	

	if ((ret = ws_audio_readState(fd)) ==0) goto ERROR;

	ret = 1;

ERROR :
#ifdef DEBUG
	if (ret) pgMessage("State load successfully.", MSG_INFO,   30);
	else     pgMessage("State load failed.",       MSG_ERROR, 100);
#else
	if (!ret) pgMessage("Satate load failed.", MSG_ERROR, 100);
#endif

ERROR_PROC1 :
	ms_fclose(fd);
	
	// force a video mode change to make all tiles dirty
	ws_gpu_clearCache();

	return ret;

}
#endif

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
#if 0
int ws_saveSRAM(char *sram_path)
{
	uint16	version;
	uint16	crc=memory_getRomCrc();
	unsigned	value;

	ms_remove(sram_path);

	int fd;
	fd = ms_fopen(sram_path, "w");
	if(fd<0) {
		pgPrint(0, 30, 0xff00, "save failed");
		return 0;
	}
	version = PSWAN_SRAM_VERSION;
	ms_fwrite(&version, 2, 1, fd);
	ms_fwrite(&crc, 2, 1, fd);
	ms_fwrite(ws_staticRam,65536,1,fd);
	ms_fclose(fd);
#ifdef DEBUG
	pgMessage("save successfully", MSG_INFO, 100);
#endif
	return 1;
}
#endif

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
#if 0
#define MacroStoreNecRegisterToFile(F,R)        \
	    value=nec_get_reg(R); \
		write(fp,&value,sizeof(value));

#else
#define MacroStoreNecRegisterToFile(F,R)        \
	    value=nec_get_reg(R); \
		if (ms_fwrite(&value,sizeof(value),1,F) < 1) goto ERROR;
#endif

#if 0
int	ws_saveState(char *statepath)
{
	uint16	crc=memory_getRomCrc();
	unsigned	value;
	char	*newPath;
	
	newPath=new char[1024];
	char newPath[MAX_PATH];

	if (strlen(statepath)<4)
		sprintf(newPath,"%s.wss",statepath);
	else
	{
		int len=strlen(statepath);
		if ((statepath[len-1]!='s')&&(statepath[len-1]!='S'))
			sprintf(newPath,"%s.wss",statepath);
		else
		if ((statepath[len-2]!='s')&&(statepath[len-2]!='S'))
			sprintf(newPath,"%s.wss",statepath);
		else

		if ((statepath[len-3]!='w')&&(statepath[len-3]!='w'))

			sprintf(newPath,"%s.wss",statepath);
		else
		if (statepath[len-4]!='.')
			sprintf(newPath,"%s.wss",statepath);
		else
			sprintf(newPath,"%s",statepath);
	}
	int	fp=open(newPath,O_BINARY|O_RDWR|O_CREAT);
	delete newPath;
	if (fp==-1)
		return(0);
	write(fp,&crc,2);
	MacroStoreNecRegisterToFile(fp,NEC_IP);
	MacroStoreNecRegisterToFile(fp,NEC_AW);
	MacroStoreNecRegisterToFile(fp,NEC_BW);
	MacroStoreNecRegisterToFile(fp,NEC_CW);
	MacroStoreNecRegisterToFile(fp,NEC_DW);
	MacroStoreNecRegisterToFile(fp,NEC_CS);
	MacroStoreNecRegisterToFile(fp,NEC_DS);
	MacroStoreNecRegisterToFile(fp,NEC_ES);
	MacroStoreNecRegisterToFile(fp,NEC_SS);
	MacroStoreNecRegisterToFile(fp,NEC_IX);
	MacroStoreNecRegisterToFile(fp,NEC_IY);
	MacroStoreNecRegisterToFile(fp,NEC_BP);
	MacroStoreNecRegisterToFile(fp,NEC_SP);
	MacroStoreNecRegisterToFile(fp,NEC_FLAGS);
	MacroStoreNecRegisterToFile(fp,NEC_VECTOR);
	MacroStoreNecRegisterToFile(fp,NEC_PENDING);
	MacroStoreNecRegisterToFile(fp,NEC_NMI_STATE);
	MacroStoreNecRegisterToFile(fp,NEC_IRQ_STATE);

	write(fp,internalRam,65536);
	write(fp,ws_staticRam,65536);
	write(fp,ws_ioRam,256);
	write(fp,ws_paletteColors,8);
	write(fp,ws_palette,16*4*2);
	write(fp,wsc_palette,16*16*2);
	write(fp,&ws_videoMode,1);
	write(fp,&ws_gpu_scanline,1);
	write(fp,externalEeprom,131072);	
	
	ws_audio_writeState(fp);
	close(fp);

	return(1);
}
#endif
#if 0
int	ws_saveState(void)
{
	int ret;
	char save_path[256];
	char *p;
	strcpy(save_path, ws_rom_path);
	p = strrchr(save_path, '.');
	*(++p) = 0;
	strcat(save_path, "sv0");

	unsigned	value;
	uint16	crc=memory_getRomCrc();

	int fd;
	if ((fd = ms_fopen(save_path, "w")) < 0) return 0;

	// VERSION
	uint16 version;
	version = PSWAN_SAVE_VERSION;
	ms_fwrite(&version, 2, 1, fd);

	// CRC
	ms_fwrite(&crc, 1, 2, fd);

	// registers
	MacroStoreNecRegisterToFile(fd,NEC_IP);
	MacroStoreNecRegisterToFile(fd,NEC_AW);
	MacroStoreNecRegisterToFile(fd,NEC_BW);
	MacroStoreNecRegisterToFile(fd,NEC_CW);
	MacroStoreNecRegisterToFile(fd,NEC_DW);
	MacroStoreNecRegisterToFile(fd,NEC_CS);
	MacroStoreNecRegisterToFile(fd,NEC_DS);
	MacroStoreNecRegisterToFile(fd,NEC_ES);
	MacroStoreNecRegisterToFile(fd,NEC_SS);
	MacroStoreNecRegisterToFile(fd,NEC_IX);
	MacroStoreNecRegisterToFile(fd,NEC_IY);
	MacroStoreNecRegisterToFile(fd,NEC_BP);
	MacroStoreNecRegisterToFile(fd,NEC_SP);
	MacroStoreNecRegisterToFile(fd,NEC_FLAGS);
	MacroStoreNecRegisterToFile(fd,NEC_VECTOR);
	MacroStoreNecRegisterToFile(fd,NEC_PENDING);
	MacroStoreNecRegisterToFile(fd,NEC_NMI_STATE);
	MacroStoreNecRegisterToFile(fd,NEC_IRQ_STATE);

	// memories
	if (ms_fwrite(internalRam,       1,   65536, fd) < 1) goto ERROR;
	if (ms_fwrite(ws_staticRam,      1,   65536, fd) < 1) goto ERROR;
	if (ms_fwrite(ws_ioRam,          1,     256, fd) < 1) goto ERROR;
	if (ms_fwrite(ws_paletteColors,  1,       8, fd) < 1) goto ERROR;
	if (ms_fwrite(ws_palette,        1,  16*4*2, fd) < 1) goto ERROR;
	if (ms_fwrite(wsc_palette,       1, 16*16*2, fd) < 1) goto ERROR;
	if (ms_fwrite(&ws_videoMode,     1,       1, fd) < 1) goto ERROR;
	if (ms_fwrite(&ws_gpu_scanline,  1,       1, fd) < 1) goto ERROR;
	if (ms_fwrite(externalEeprom,    1,  131072, fd) < 1) goto ERROR;

	if (ws_audio_writeState(fd) ==0) goto ERROR;

	ret = 1;

ERROR :
#ifdef DEBUG
	if (ret) pgMessage("State save successfully.", MSG_INFO,   30);
	else     pgMessage("State save failed.",       MSG_ERROR, 100);
#else
	if (!ret) pgMessage("State save failed.", MSG_ERROR, 100);
#endif

	ms_fclose(fd);
	return ret;
}
#endif

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
int ws_rotated(void)
{
	uint8	*rom=memory_getRom();
	uint32	romSize=memory_getRomSize();
	
	return(rom[romSize-4]&1);
}
