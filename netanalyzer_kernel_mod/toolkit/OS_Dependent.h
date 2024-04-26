/**************************************************************************************

   Copyright (c) Hilscher GmbH. All Rights Reserved.

 **************************************************************************************

   Filename:
    $Id: OS_Dependent.h 6884 2015-06-02 07:46:45Z sebastiand $
   Last Modification:
    $Author: sebastiand $
    $Date: 2015-06-02 09:46:45 +0200 (Tue, 02 Jun 2015) $
    $Revision: 6884 $

   Targets:
     Win32/ANSI   : yes

   Description:
    Header file for the OS dependant definitions

   Changes:

     Version   Date        Author   Description
     ----------------------------------------------------------------------------------
     3        26.05.2015   SD       Added mutex-functions
     2        05.02.2013   SD       add file header
     1        -/-          MT       initial version

**************************************************************************************/

#ifndef __OS_DEPENDENT__H
#define __OS_DEPENDENT__H

#include <OS_Includes.h>

#define NETANA_EVENT_TIMEOUT    0
#define NETANA_EVENT_SIGNALLED  1

int  OS_Init(void);
void OS_Deinit(void);

uint32_t OS_GetMilliSecCounter(void);

void* OS_Memalloc(uint32_t ulSize);
void  OS_Memfree(void* pvMem);
void  OS_Memcpy(void* pvDest, const void* pvSrc, unsigned long ulSize);
int   OS_Memcmp(const void* pvBuf1, const void* pvBuf2, unsigned long ulSize);
void  OS_Memset(void* pvMem, uint8_t bFill, uint32_t ulLen);

void* OS_CreateLock(void);
void  OS_DeleteLock(void* hLock);
void  OS_EnterLock(void* hLock);
void  OS_LeaveLock(void* hLock);

void* OS_ReadPCIConfig(void* pvOSDependent);
void  OS_WritePCIConfig(void* pvOSDependent, void* pvPCIConfig);
void  OS_EnableDeviceInterrupts(void* pvOSDependent);
void  OS_DisableDeviceInterrupts(void* pvOSDependent);

void  OS_Sleep(unsigned long ulSleepTimeMs);

int   OS_Strnicmp(const char* pszBuf1, const char* pszBuf2, uint32_t ulLen);
char* OS_Strncpy(char* szDest, const char* szSource, uint32_t ulLength);
int   OS_Strlen(const char* szText);

#define OS_FILE_FLAG_OPENEXISTING   0x00000001
#define OS_FILE_FLAG_CREATEIFNEEDED 0x00000002

void*         OS_FileOpen(char* szFilename, uint64_t* pullFileSize, uint32_t ulFlags);
unsigned long OS_FileSeek(void* pvFile,  unsigned long ulOffset);
unsigned long OS_FileRead(void* pvFile,  unsigned long ulOffset, unsigned long ulSize, void* pvBuffer);
unsigned long OS_FileWrite(void* pvFile, unsigned long ulSize, void* pvBuffer);
void          OS_FileClose(void* pvFile);

void*         OS_CreateEvent(void);
void          OS_SetEvent(void* pvEvent);
void          OS_ResetEvent(void* pvEvent);
void          OS_DeleteEvent(void* pvEvent);
uint32_t      OS_WaitEvent(void* pvEvent, uint32_t ulTimeout);

int           OS_Snprintf(char* szBuffer, uint32_t ulSize, const char* szFormat, ...);

#ifdef TOOLKIT_ENABLE_DSR_LOCK
void  OS_IrqLock  ( void* pvOSDependent);
void  OS_IrqUnlock( void* pvOSDependent);
#endif

void* OS_CreateMutex(void);
int   OS_WaitMutex(void* pvMutex, uint32_t ulTimeout);
void  OS_ReleaseMutex(void* pvMutex);
void  OS_DeleteMutex(void* pvMutex);

#endif /*  __OS_DEPENDENT__H */
