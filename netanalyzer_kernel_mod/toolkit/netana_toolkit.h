/**************************************************************************************

   Copyright (c) Hilscher GmbH. All Rights Reserved.

 **************************************************************************************

   Filename:
    $Id: netana_toolkit.h 7669 2017-02-28 16:30:46Z Robert $
   Last Modification:
    $Author: Robert $
    $Date: 2017-02-28 17:30:46 +0100 (Tue, 28 Feb 2017) $
    $Revision: 7669 $

   Targets:
     Win32/ANSI   : yes

   Description:


   Changes:

     Version   Date        Author   Description
     ----------------------------------------------------------------------------------
     4        28.02.2017   RM       Added new CIFX_DEV_CLASS definition
     3        02.06.2015   SD       added mailbox resources
     2        05.02.2013   SD       add file header
     1        -/-          MT       initial version

**************************************************************************************/

#ifndef __NETANA_TOOLKIT__H
#define __NETANA_TOOLKIT__H

#include "netana_user.h"
#include "netana_errors.h"
#include "netana_dpmlayout.h"

#undef LIST_HEAD
#undef STAILQ_ENTRY
#include "sys/queue.h"

#include <OS_Dependent.h>

#include "netx_registers.h"

#define NETANA_TOOLKIT_VERSION_MAJOR                        1
#define NETANA_TOOLKIT_VERSION_MINOR                        7
#define NETANA_TOOLKIT_VERSION_BUILD                        0
#define NETANA_TOOLKIT_VERSION_REVISION                     0

#define NETANA_MAX_GPIOS                4
#define NETANA_MAX_PORTS                4
#define NETANA_MAX_FILES              256
#define NETANA_MAX_FILEPATH           256

#define NETANA_UPDATE_INTERVAL        500

#define NETANA_DEFAULT_DMABUFFERS       8
#define NETANA_DEFAULT_DMABUFFER_SIZE   (512 * 1024)
#define NETANA_DEFAULT_DMABUFFER_TIMEOUT 5 /* 50 ms */

#define NANL_DEV_CLASS 0x10
#define NSCP_DEV_CLASS 0x31
#define CIFX_DEV_CLASS 0x03

/* Give the user the possibility to use own macros for
   endianess conversion */
#ifndef BIGENDIAN_MACROS_PROVIDED
  #ifndef NETANA_TOOLKIT_BIGENDIAN
    /* Little endian, so we don't need a conversion */
    #define LE16_TO_HOST(a)   (a)
    #define LE32_TO_HOST(a)   (a)
    #define LE64_TO_HOST(a)   (a)
    #define HOST_TO_LE16(a)   (a)
    #define HOST_TO_LE32(a)   (a)
    #define HOST_TO_LE64(a)   (a)
  #else
    /* BIG endian, so we DO need a conversion */
    #define LE16_TO_HOST(a)   ( ((a & 0x00FF) << 8) | \
                                ((a & 0xFF00) >> 8) )

    #define LE32_TO_HOST(a)   ( ((a & 0x000000FFUL) << 24) | \
                                ((a & 0x0000FF00UL) << 8)  | \
                                ((a & 0x00FF0000UL) >> 8)  | \
                                ((a & 0xFF000000UL) >> 24) )

    #define LE64_TO_HOST(a)   ( ((a & 0x00000000000000FFULL) << 56) | \
                                ((a & 0x000000000000FF00ULL) << 40) | \
                                ((a & 0x0000000000FF0000ULL) << 24) | \
                                ((a & 0x00000000FF000000ULL) << 8)  | \
                                ((a & 0x000000FF00000000ULL) >> 8)  | \
                                ((a & 0x0000FF0000000000ULL) >> 24) | \
                                ((a & 0x00FF000000000000ULL) >> 40) | \
                                ((a & 0xFF00000000000000ULL) >> 56) )

    #define HOST_TO_LE16(a)   LE16_TO_HOST(a)
    #define HOST_TO_LE32(a)   LE32_TO_HOST(a)
    #define HOST_TO_LE64(a)   LE64_TO_HOST(a)
  #endif
#endif

typedef struct NETANA_DMABUFFER_Ttag
{
  void*     hBuffer;
  uint32_t  ulPhysicalAddress;
  void*     pvBufferAddress;
  uint32_t  ulBufferSize;
} NETANA_DMABUFFER_T;

typedef enum CHIPTYPE_Etag
{
  eCHIP_NETX100_500,
  eCHIP_NETX50,
  eCHIP_UNKNOWN
} CHIPTYPE_E;

typedef struct NETANA_IRQ2DSR_BUFFER_Ttag
{
  NETANA_HANDSHAKE_CELL atHskCell[8];
  int                   fValid;

} NETANA_IRQ2DSR_BUFFER_T;

typedef struct NETANA_FILE_DATA_Ttag
{
  void*    hFile;
  uint64_t ullOffset;
  uint64_t ullDataWritten;

} NETANA_FILE_DATA_T;

typedef struct NETANA_DPM_FILTER_Ttag
{
  NETANA_FILTER_BASE_T* ptBase;

  struct
  {
    uint8_t*  pbMask;
    uint8_t*  pbValue;
  } atFilter[NETANA_MAX_FILTERS];

} NETANA_DPM_FILTER_T;

typedef struct NETANA_IDENT_FILE_INFO_Ttag
{
  int      fIdentUpdateRequest;
  char     szIdentInfoFileName[NETANA_MAX_FILEPATH];
  int32_t  lError;
}NETANA_IDENT_FILE_INFO_T;

typedef void(APIENTRY *PFN_NOTIFY_CALLBACK)  (uint32_t ulNotification, uint32_t ulDataLen, void* pvData, void* pvUser);

/*****************************************************************************/
/*! Structure defining the send mailbox                                      */
/*****************************************************************************/
typedef struct NETANA_TX_MAILBOX_Ttag
{
  NETANA_SEND_MAILBOX_BLOCK* ptSendMailboxStart;        /*!< virtual start address of send mailbox            */
  uint32_t                   ulSendMailboxLength;       /*!< Length of send mailbox in bytes                  */
  uint32_t                   ulSendCMDBitmask;          /*!< Bitmask for Handshakeflags to send packet        */
  uint8_t                    bSendCMDBitoffset;         /*!< Bitnumber for send packet flag (used for notification array indexing) */
  void*                      pvSendMBXMutex;            /*!< Synchronistation object for the send mailbox     */
  uint32_t                   ulSendPacketCnt;           /*!< Number of packets sent on this mailbox           */
  PFN_NOTIFY_CALLBACK        pfnCallback;               /*!< Notification callback                            */
  void*                      pvUser;                    /*!< User pointer for callback                        */

} NETANA_TX_MAILBOX_T;

/*****************************************************************************/
/*! Structure defining the receive mailbox                                   */
/*****************************************************************************/
typedef struct NETANA_RX_MAILBOX_Ttag
{
  NETANA_RECV_MAILBOX_BLOCK* ptRecvMailboxStart;        /*!< virtual start address of receive mailbox         */
  uint32_t                   ulRecvMailboxLength;       /*!< Length of receive mailbox in bytes               */
  uint32_t                   ulRecvACKBitmask;          /*!< Bitmask for Handshakeflags to ack recv. packet   */
  uint8_t                    bRecvACKBitoffset;         /*!< Bitnumber for recv packet ack flag (used for notification array indexing) */
  void*                      pvRecvMBXMutex;            /*!< Synchronistation object for the receive mailbox  */
  uint32_t                   ulRecvPacketCnt;           /*!< Number of packets recevied on this mailbox       */
  PFN_NOTIFY_CALLBACK        pfnCallback;               /*!< Notification callback                            */
  void*                      pvUser;                    /*!< User pointer for callback                        */

} NETANA_RX_MAILBOX_T;

struct NETANA_DEVINSTANCE_T
{
  STAILQ_ENTRY(NETANA_DEVINSTANCE_T) tList;
  char                         szDeviceName[NETANA_MAX_DEVICENAMESIZE];
  char                         szAliasName[NETANA_MAX_DEVICENAMESIZE];
  void*                        pvDPM;
  uint32_t                     ulDPMPhysicalAddress;
  uint32_t                     ulDPMSize;
  uint32_t                     ulInterruptNr;
  uint32_t                     ulFlashBased;
  uint16_t                     usDeviceClass;
  uint32_t                     ulDevNo;

  uint32_t                     ulDMABufferCount;
  NETANA_DMABUFFER_T           atDMABuffers[NETANA_MAX_DMA_BUFFERS];

  uint32_t                     ulOpenCnt;

  PNETX_GLOBAL_REG_BLOCK       ptGlobalRegisters;

  NETANA_BASE_DPM_T*           ptBaseDPM;

  uint32_t                     ulPortCnt;

  /* Filters */
  uint32_t                     ulFilterSize;
  NETANA_DPM_FILTER_T          atFilters[NETANA_MAX_PORTS];

  /* Counters */
  uint32_t                     ulCounterSize;
  NETANA_COUNTER_BLOCK_T*      aptCounters[NETANA_MAX_PORTS];

  /* GPIO's */
  uint32_t                     ulGpioCnt;
  NETANA_GPIO_BLOCK_T*         aptGPIOs[NETANA_MAX_GPIOS];


  NETANA_HANDSHAKE_BLOCK_T*    ptHandshakeBlock;

  CHIPTYPE_E                   eChip;

  uint32_t                     ulDeviceNr;
  uint32_t                     ulSerialNr;

  /* IRQ stuff */
  uint32_t                     ulIrqCounter;
  int                          iIrq2DsrBuffer;
  NETANA_IRQ2DSR_BUFFER_T      atIrq2DsrBuffer[2];
  void*                        ahHandshakeBitEvents[16];
  void*                        hStopComplete;

  void*                        pvLock;
  uint32_t                     ulNetxFlags;
  uint32_t                     ulHostFlags;

  uint32_t                     ulCurrentBuffer;

  /* File list */
  NETANA_FILE_HEADER_T         tFileHeader;
  int                          fRingBuffer;
  int                          fWriteToFile;
  uint32_t                     ulFileCount;
  uint64_t                     ullFileSize;
  char                         szPath[NETANA_MAX_FILEPATH];
  char                         szBaseFilename[NETANA_MAX_FILEPATH];
  uint32_t                     ulCurrentFile;
  NETANA_FILE_DATA_T           atFiles[NETANA_MAX_FILES];

  /* User callbacks */
  PFN_STATUS_CALLBACK           pfnStatus;
  PFN_DATA_CALLBACK             pfnData;
  void*                         pvUser;

  void*                         pvOSDependent;

  /* license information */
  NETANA_IDENT_INFO_BLOCK_T     tIdentInfoBlock;

  NETANA_IDENT_FILE_INFO_T      tIdentInfoFile;

  /* default configuration, set when device becomes opened */
  NETANA_DPM_FILTER_T           atDefaultFilter[NETANA_MAX_PORTS];
  NETANA_GPIO_BLOCK_T           atDefaultGPIOs[NETANA_MAX_GPIOS];

  NETANA_TX_MAILBOX_T           tSendMbx;
  NETANA_RX_MAILBOX_T           tRecvMbx;
};

int32_t netana_tkit_init(void);
void    netana_tkit_deinit(void);

int32_t netana_tkit_deviceadd(struct NETANA_DEVINSTANCE_T* ptDevInstance, uint32_t ulDevNo);
int32_t netana_tkit_deviceremove(struct NETANA_DEVINSTANCE_T* ptDevInstance, int fForceRemove);

int32_t netana_restart_device( NETANA_HANDLE hDev);

#define NETANA_TKIT_ISR_IRQ_OTHERDEVICE   0
#define NETANA_TKIT_ISR_IRQ_HANDLED       1
#define NETANA_TKIT_ISR_IRQ_DSR_REQUESTED 2
int32_t netana_tkit_isr_handler(struct NETANA_DEVINSTANCE_T* ptDevInstance, int fCheckOtherDevice);

#define NETANA_TKIT_DSR_HANDLED             0
#define NETANA_TKIT_DSR_PROCESSING_REQUEST  1
int32_t netana_tkit_dsr_handler(struct NETANA_DEVINSTANCE_T* ptDevInstance);
void    netana_tkit_process(struct NETANA_DEVINSTANCE_T* ptDevInstance);

void    netana_tkit_ToggleBit(struct NETANA_DEVINSTANCE_T* ptDevInst, uint32_t ulBit);
int32_t netana_tkit_writefileheader(struct NETANA_DEVINSTANCE_T* ptDevInst, uint64_t ullSize);

#define NETANA_DEFAULT_TIMEOUT   1000
#define NETANA_BITSTATE_SET      0
#define NETANA_BITSTATE_CLEAR    1
#define NETANA_BITSTATE_EQUAL    2
#define NETANA_BITSTATE_UNEQUAL  3

int netana_tkit_WaitForBitState(struct NETANA_DEVINSTANCE_T* ptDevInst, uint32_t ulBit,
                                uint8_t bState, uint32_t ulTimeout);

void netana_tkit_disable_hwinterrupts(struct NETANA_DEVINSTANCE_T* ptDevInstance);
void netana_tkit_enable_hwinterrupts(struct NETANA_DEVINSTANCE_T* ptDevInstance);

int32_t netana_signal_driver_error(struct NETANA_DEVINSTANCE_T* ptDevInst, uint32_t ulError);

STAILQ_HEAD(NETANA_CARD_LIST, NETANA_DEVINSTANCE_T);

extern struct NETANA_CARD_LIST g_tCardList;
extern uint32_t                g_ulCardCnt;
extern void*                   g_pvCardLock;

extern uint32_t                g_ulTraceLevel;
extern uint32_t                g_ulDMABufferCount;
extern uint32_t                g_ulDMABufferSize;
extern uint32_t                g_ulDMABufferTimeout;
extern int32_t                 g_lTimezoneCorrection;

#define NETANA_MAX_FILE_NAME_LENGTH   256

/*****************************************************************************/
/*! Structure passed to USER implemented function, for getting device        *
*   specific configuration files                                             */
/*****************************************************************************/
typedef struct NETANA_TKIT_FILE_INFO_Ttag
{
  char  szShortFileName[16];                          /*!< Short filename (8.3) of the file       */
  char  szFullFileName[NETANA_MAX_FILE_NAME_LENGTH];  /*!< Full filename (including path) to file */
} NETANA_TKIT_FILE_INFO_T, *PNETANA_TKIT_FILE_INFO_T;

void USER_GetBootloaderFile(struct NETANA_DEVINSTANCE_T* ptDevInst,
                            PNETANA_TKIT_FILE_INFO_T ptFileInfo);

void USER_GetFirmwareFile(struct NETANA_DEVINSTANCE_T* ptDevInst,
                            PNETANA_TKIT_FILE_INFO_T ptFileInfo);

void USER_GetIdentInfoFile(struct NETANA_DEVINSTANCE_T* ptDevInst,
                            PNETANA_TKIT_FILE_INFO_T ptFileInfo);

int32_t  USER_SetDevClassFilter  ( uint32_t ulDeviceFilter);
uint32_t USER_GetDevClassFilter  ( void);
void     USER_ClearDevClassFilter( void);
int32_t  USER_GetAliasName( uint32_t ulDeviceClass, char* szAliasName, uint32_t ulNameLen);

/* User specific functions */
#define NETANA_TRACELEVEL_ERROR   1
#define NETANA_TRACELEVEL_WARNING 2
#define NETANA_TRACELEVEL_INFO    3
#define NETANA_TRACELEVEL_DEBUG   4

void USER_Trace(struct NETANA_DEVINSTANCE_T* ptDevInst, uint32_t ulTraceLevel,
                char* szFormat, ...);

#endif /*  __NETANA_TOOLKIT__H */
