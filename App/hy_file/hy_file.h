#ifndef __HY_FILE_H__
#define __HY_FILE_H__
 
#include "main.h"
#include "FatFs.h"
#include "stdio.h"
#include "ff.h"
#include "hy_fram.h"
#include "hy_flash.h"
#include <stdlib.h>

/*定义自己的存储设备*/
/*用户存储设备扇区字节数*/
#define User_Sector 512
/*用户存储设备FatFS对象*/
#define User_FatFs 	USBHFatFS
/*用户存储设备卷路径*/
#define User_SDPath USBHPath
/*用户存储设备初始化类型*/
#define User_FatType FM_FAT

extern uint8_t export_start;
extern char file_name[24];		/* 18 字节文件名 + 终止符余量 */
extern uint8_t close_file;
extern char save_data[32];		/* 28 字节表格行 + 终止符余量 */
extern uint8_t receive_data_1;
extern uint8_t flash_access;

/*函数声明*/
void Mount_FatFs(void);
void FatFs_GetDiskInfo(void);
void FatFs_ScanDir(const TCHAR* PathName);
void FatFs_ReadTXTFile(TCHAR *filename);
void FatFs_WriteTXTFile(TCHAR *filename,uint16_t year, uint8_t month, uint8_t day);
void FatFs_GetFileInfo(TCHAR *filename);
void FatFs_DeleteFile(TCHAR *filename);
void FatFs_PrintfFileDate(WORD date, WORD time);
FRESULT Clear_File(char *dir_path);
FRESULT Create_File(FIL	*file);
FRESULT Save_Data(void);
 
#endif

