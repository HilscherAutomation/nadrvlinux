/**************************************************************************************

   Copyright (c) Hilscher GmbH. All Rights Reserved.

 **************************************************************************************

   Filename:
    $Id: netana_toolkit.c 7669 2017-02-28 16:30:46Z Robert $
   Last Modification:
    $Author: Robert $
    $Date: 2017-02-28 17:30:46 +0100 (Tue, 28 Feb 2017) $
    $Revision: 7669 $

   Targets:
     Win32/ANSI   : yes

   Description:
    Toolkit internal functions

   Changes:

     Version   Date        Author   Description
     ----------------------------------------------------------------------------------
     7        28.02.2017   RM       added handling for new CIFX_DEV_CLASS
     6        02.06.2015   SD       add initialization of mailbox system
     5        10.04.2014   SD       bugfix: netana_tkit_deviceadd() does not return an error
                                    in case of device class request failure (RequestDeviceClass())
     4        05.02.2014   SD       netana_tkit_deviceadd(): store filter, gpio default configuration
     3        05.07.2013   SD       add device differentiation (RequestDeviceClass())
     2        05.02.2013   SD       add file header
     1        -/-          MT       initial version

**************************************************************************************/

#include "netana_toolkit.h"
#include "netx_registers.h"
#include "netx_romloader.h"
#include "netx50_romloader_dpm.h"

#include "rcX/rcX_Public.h"   /* Needed for Download */

#ifndef min
  #define min(a,b)  ((a < b) ? a : b)
#endif

uint32_t g_ulDMABufferCount    = NETANA_DEFAULT_DMABUFFERS;
uint32_t g_ulDMABufferSize     = NETANA_DEFAULT_DMABUFFER_SIZE;
uint32_t g_ulDMABufferTimeout  = NETANA_DEFAULT_DMABUFFER_TIMEOUT;
int32_t  g_lTimezoneCorrection = 0;

struct NETANA_CARD_LIST g_tCardList    = STAILQ_HEAD_INITIALIZER(g_tCardList);
uint32_t                g_ulCardCnt    = 0;
void*                   g_pvCardLock   = NULL;

uint32_t                g_ulTraceLevel = 0;

typedef int32_t (*PFN_TRANSFER_PACKET)(void* pvChannel, RCX_PACKET* ptSendPkt, RCX_PACKET* ptRecvPkt, unsigned long ulRecvBufferSize, unsigned long ulTimeout);

#define TO_SEND_PACKET 1000
#define DPM_NO_MEMORY_ASSIGNED           0x0BAD0BADUL
#define DPM_INVALID_CONTENT              0xFFFFFFFFUL

#define DPMSIGNATURE_BSL_STR             "BOOT"
#define DPMSIGNATURE_BSL_VAL             0x544F4F42UL
#define DPMSIGNATURE_FW_STR              "netX"
#define DPMSIGNATURE_FW_VAL              0x5874656EUL

/*****************************************************************************/
/*! Waits for a given handshake bit state on the channel (polling mode)
*   \param ptHsk        Handshake Cell (8Bit)
*   \param ulBitMask    BitMask to wait for
*   \param bState       State the handshake bit should be in after returning
*                       from this function
*   \param ulTimeout    Maximum time in ms to wait for the desired bit state
*   \return 0 on error/timeout, 1 on success                                 */
/*****************************************************************************/
static int WaitForBitStatePollBL(NETX_HANDSHAKE_CELL* ptHsk, unsigned long ulBitMask, unsigned char bState, unsigned long ulTimeout)
{
  unsigned char   bActualState;
  int             iRet        = 0;
  int32_t         lStartTime  = 0;
  uint8_t         bHostFlags  = ptHsk->t8Bit.bHostFlags;
  uint8_t         bNetXFlags  = ptHsk->t8Bit.bNetxFlags;
  unsigned long   ulDiffTime  = 0L;

  if( (bHostFlags ^ bNetXFlags) & ulBitMask)
    bActualState = RCX_FLAGS_NOT_EQUAL;
  else
    bActualState = RCX_FLAGS_EQUAL;

  /* The desired state is already there, so just return true */
  if(bActualState == bState)
    return 1;

  /* If no timeout is given, don't try to wait for the Bit change */
  if(0 == ulTimeout)
    return 0;

  lStartTime = (long)OS_GetMilliSecCounter();

  /* Poll for desired bit state */
  do
  {
    bHostFlags  = ptHsk->t8Bit.bHostFlags;
    bNetXFlags  = ptHsk->t8Bit.bNetxFlags;

    if((bHostFlags ^ bNetXFlags) & ulBitMask)
      bActualState = RCX_FLAGS_NOT_EQUAL;
    else
      bActualState = RCX_FLAGS_EQUAL;

    if(bActualState == bState)
      break;

    /* Check for timeout */
    ulDiffTime = OS_GetMilliSecCounter() - lStartTime;
    if ( ulDiffTime < ulTimeout)
    {
      OS_Sleep(0);
    }
  } while(ulDiffTime <= ulTimeout);

  if(bActualState == bState)
    iRet = 1;

  return iRet;
}

/*****************************************************************************/
/*! Sends a Packet to the device/channel
*   \param ptSysChannel System Channel
*   \param ptSendPkt    Packet to send
*   \param ulTimeout    Maximum time in ms to wait for an empty mailbox
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
static int32_t PutPacketBL(NETX_SYSTEM_CHANNEL* ptSysChannel, RCX_PACKET* ptSendPkt, unsigned long ulTimeout)
{
  int32_t                 lRet          = NETANA_DEV_MAILBOX_FULL;
  NETX_HANDSHAKE_CHANNEL* ptHskChannel  = (NETX_HANDSHAKE_CHANNEL*)(ptSysChannel + 1);

  if(0 == (ptHskChannel->tSysFlags.t8Bit.bNetxFlags & NSF_READY))
    return NETANA_DEV_NOT_READY;

  /* Check if packet fits into the mailbox */
  if( (LE32_TO_HOST(ptSendPkt->tHeader.ulLen) + RCX_PACKET_HEADER_SIZE) > NETX_SYSTEM_MAILBOX_MIN_SIZE)
    return NETANA_DEV_MAILBOX_TOO_SHORT;

  if(WaitForBitStatePollBL(&ptHskChannel->tSysFlags, HSF_SEND_MBX_CMD, RCX_FLAGS_EQUAL, ulTimeout))
  {
    /* Copy packet to mailbox */
    OS_Memcpy(ptSysChannel->tSystemSendMailbox.abSendMbx,
              ptSendPkt,
              LE32_TO_HOST(ptSendPkt->tHeader.ulLen) + RCX_PACKET_HEADER_SIZE);

    /* Signal new packet */
    ptHskChannel->tSysFlags.t8Bit.bHostFlags ^= HSF_SEND_MBX_CMD;

    lRet = NETANA_NO_ERROR;
  }

  return lRet;
}

/*****************************************************************************/
/*! Retrieves a Packet from the device/channel
*   \param ptSysChannel     System channel
*   \param ptRecvPkt        Pointer to place received Packet in
*   \param ulRecvBufferSize Length of the receive buffer
*   \param ulTimeout        Maximum time in ms to wait for an empty mailbox
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
static int32_t GetPacketBL(NETX_SYSTEM_CHANNEL* ptSysChannel, RCX_PACKET* ptRecvPkt, unsigned long ulRecvBufferSize, unsigned long ulTimeout)
{
  int32_t                 lRet          = NETANA_NO_ERROR;
  uint32_t                ulCopySize    = 0;
  RCX_PACKET*             ptPacket      = NULL;
  NETX_HANDSHAKE_CHANNEL* ptHskChannel  = (NETX_HANDSHAKE_CHANNEL*)(ptSysChannel + 1);
  uint32_t                ulPacketLen, ulPacketCmd;

  if(0 == (ptHskChannel->tSysFlags.t8Bit.bNetxFlags & NSF_READY))
    return NETANA_DEV_NOT_READY;

  if(!WaitForBitStatePollBL(&ptHskChannel->tSysFlags, HSF_RECV_MBX_ACK, RCX_FLAGS_NOT_EQUAL, ulTimeout))
    return NETANA_DEV_GET_NO_PACKET;

  ptPacket    = (RCX_PACKET*)ptSysChannel->tSystemRecvMailbox.abRecvMbx;

  /* Debug */
  ulPacketLen = LE32_TO_HOST(ptPacket->tHeader.ulLen);
  ulPacketCmd = LE32_TO_HOST(ptPacket->tHeader.ulCmd);

  ulCopySize  = LE32_TO_HOST(ptPacket->tHeader.ulLen) + RCX_PACKET_HEADER_SIZE;
  if(ulCopySize > ulRecvBufferSize)
  {
    /* We have to free the mailbox, read as much as possible */
    ulCopySize = ulRecvBufferSize;
    lRet = NETANA_BUFFER_TOO_SHORT;

  }

  OS_Memcpy(ptRecvPkt, ptPacket, ulCopySize);

  /* Signal read packet done */
  ptHskChannel->tSysFlags.t8Bit.bHostFlags ^= HSF_RECV_MBX_ACK;

  return lRet;
}

/*****************************************************************************/
/*! Transfer a Packet from the device/channel
*   \param pvChannel        System channel
*   \param ptSendPkt        Pointer to send Packet
*   \param ptRecvPkt        Pointer to received packet
*   \param ulRecvBufferSize Length of the receive buffer
*   \param ulTimeout        Maximum time in ms to wait for an empty mailbox
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
static int32_t TransferPacket(void* pvChannel, RCX_PACKET* ptSendPkt, RCX_PACKET* ptRecvPkt, unsigned long ulRecvBufferSize, unsigned long ulTimeout)
{
  struct NETANA_DEVINSTANCE_T* ptDevInstance = (struct NETANA_DEVINSTANCE_T*)pvChannel;
  NETX_SYSTEM_CHANNEL*         ptSysChannel  = (NETX_SYSTEM_CHANNEL*)ptDevInstance->pvDPM;
  int32_t                      lRet          = NETANA_NO_ERROR;
  int32_t                      lStartTime    = OS_GetMilliSecCounter();

  if( NETANA_NO_ERROR == (lRet = PutPacketBL(ptSysChannel, ptSendPkt, ulTimeout)) )
  {
    do
    {
      if(NETANA_NO_ERROR == (lRet = GetPacketBL(ptSysChannel, ptRecvPkt, ulRecvBufferSize, ulTimeout)) )
      {
        /* Check if we got the answer */
        if((LE32_TO_HOST(ptRecvPkt->tHeader.ulCmd) & ~RCX_MSK_PACKET_ANSWER) == LE32_TO_HOST(ptSendPkt->tHeader.ulCmd) )
        {
          /* Check rest of packet data */
          if ( (ptRecvPkt->tHeader.ulSrc   == ptSendPkt->tHeader.ulSrc)    &&
               (ptRecvPkt->tHeader.ulId    == ptSendPkt->tHeader.ulId)     &&
               (ptRecvPkt->tHeader.ulSrcId == ptSendPkt->tHeader.ulSrcId)  )
          {
            /* We got the answer message */
            /* lRet = ptRecvPkt->tHeader.ulState; */ /* Do not deliever back this information */
            break;
          }
        }
        /* Reset error, in case we might drop out of the loop, with no proper answer,
           returning a "good" state */
        lRet = NETANA_TRANSFER_TIMEOUT;
      }else
      {
        /* Error during packet receive */
        break;
      }
    } while ( (OS_GetMilliSecCounter() - lStartTime) < ulTimeout);
  }

  return lRet;
}

/*****************************************************************************/
/*! netANALYZER Toolkit initialization
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t netana_tkit_init(void)
{
  int32_t lRet = NETANA_TKIT_INITIALIZATION_FAILED;

  if(OS_Init())
  {
    STAILQ_INIT(&g_tCardList);
    g_ulCardCnt = 0;

    if(NULL != (g_pvCardLock = OS_CreateLock()))
    {
      lRet = NETANA_NO_ERROR;
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! netANALYZER Toolkit uninitialization                                     */
/*****************************************************************************/
void netana_tkit_deinit(void)
{
  struct NETANA_DEVINSTANCE_T* ptDevInst;

  if(NULL != g_pvCardLock)
  {
    OS_DeleteLock(g_pvCardLock);
    g_pvCardLock = NULL;
  }

  /* Remove all devices */
  while(NULL != (ptDevInst = STAILQ_FIRST(&g_tCardList)))
  {
    netana_tkit_deviceremove(ptDevInst, 1);
  }

  OS_Deinit();
}

/*****************************************************************************/
/*! CRC 32 lookup table                                                      */
/*****************************************************************************/
static uint32_t Crc32Table[256]=
{
  0x00000000UL, 0x77073096UL, 0xee0e612cUL, 0x990951baUL, 0x076dc419UL,
  0x706af48fUL, 0xe963a535UL, 0x9e6495a3UL, 0x0edb8832UL, 0x79dcb8a4UL,
  0xe0d5e91eUL, 0x97d2d988UL, 0x09b64c2bUL, 0x7eb17cbdUL, 0xe7b82d07UL,
  0x90bf1d91UL, 0x1db71064UL, 0x6ab020f2UL, 0xf3b97148UL, 0x84be41deUL,
  0x1adad47dUL, 0x6ddde4ebUL, 0xf4d4b551UL, 0x83d385c7UL, 0x136c9856UL,
  0x646ba8c0UL, 0xfd62f97aUL, 0x8a65c9ecUL, 0x14015c4fUL, 0x63066cd9UL,
  0xfa0f3d63UL, 0x8d080df5UL, 0x3b6e20c8UL, 0x4c69105eUL, 0xd56041e4UL,
  0xa2677172UL, 0x3c03e4d1UL, 0x4b04d447UL, 0xd20d85fdUL, 0xa50ab56bUL,
  0x35b5a8faUL, 0x42b2986cUL, 0xdbbbc9d6UL, 0xacbcf940UL, 0x32d86ce3UL,
  0x45df5c75UL, 0xdcd60dcfUL, 0xabd13d59UL, 0x26d930acUL, 0x51de003aUL,
  0xc8d75180UL, 0xbfd06116UL, 0x21b4f4b5UL, 0x56b3c423UL, 0xcfba9599UL,
  0xb8bda50fUL, 0x2802b89eUL, 0x5f058808UL, 0xc60cd9b2UL, 0xb10be924UL,
  0x2f6f7c87UL, 0x58684c11UL, 0xc1611dabUL, 0xb6662d3dUL, 0x76dc4190UL,
  0x01db7106UL, 0x98d220bcUL, 0xefd5102aUL, 0x71b18589UL, 0x06b6b51fUL,
  0x9fbfe4a5UL, 0xe8b8d433UL, 0x7807c9a2UL, 0x0f00f934UL, 0x9609a88eUL,
  0xe10e9818UL, 0x7f6a0dbbUL, 0x086d3d2dUL, 0x91646c97UL, 0xe6635c01UL,
  0x6b6b51f4UL, 0x1c6c6162UL, 0x856530d8UL, 0xf262004eUL, 0x6c0695edUL,
  0x1b01a57bUL, 0x8208f4c1UL, 0xf50fc457UL, 0x65b0d9c6UL, 0x12b7e950UL,
  0x8bbeb8eaUL, 0xfcb9887cUL, 0x62dd1ddfUL, 0x15da2d49UL, 0x8cd37cf3UL,
  0xfbd44c65UL, 0x4db26158UL, 0x3ab551ceUL, 0xa3bc0074UL, 0xd4bb30e2UL,
  0x4adfa541UL, 0x3dd895d7UL, 0xa4d1c46dUL, 0xd3d6f4fbUL, 0x4369e96aUL,
  0x346ed9fcUL, 0xad678846UL, 0xda60b8d0UL, 0x44042d73UL, 0x33031de5UL,
  0xaa0a4c5fUL, 0xdd0d7cc9UL, 0x5005713cUL, 0x270241aaUL, 0xbe0b1010UL,
  0xc90c2086UL, 0x5768b525UL, 0x206f85b3UL, 0xb966d409UL, 0xce61e49fUL,
  0x5edef90eUL, 0x29d9c998UL, 0xb0d09822UL, 0xc7d7a8b4UL, 0x59b33d17UL,
  0x2eb40d81UL, 0xb7bd5c3bUL, 0xc0ba6cadUL, 0xedb88320UL, 0x9abfb3b6UL,
  0x03b6e20cUL, 0x74b1d29aUL, 0xead54739UL, 0x9dd277afUL, 0x04db2615UL,
  0x73dc1683UL, 0xe3630b12UL, 0x94643b84UL, 0x0d6d6a3eUL, 0x7a6a5aa8UL,
  0xe40ecf0bUL, 0x9309ff9dUL, 0x0a00ae27UL, 0x7d079eb1UL, 0xf00f9344UL,
  0x8708a3d2UL, 0x1e01f268UL, 0x6906c2feUL, 0xf762575dUL, 0x806567cbUL,
  0x196c3671UL, 0x6e6b06e7UL, 0xfed41b76UL, 0x89d32be0UL, 0x10da7a5aUL,
  0x67dd4accUL, 0xf9b9df6fUL, 0x8ebeeff9UL, 0x17b7be43UL, 0x60b08ed5UL,
  0xd6d6a3e8UL, 0xa1d1937eUL, 0x38d8c2c4UL, 0x4fdff252UL, 0xd1bb67f1UL,
  0xa6bc5767UL, 0x3fb506ddUL, 0x48b2364bUL, 0xd80d2bdaUL, 0xaf0a1b4cUL,
  0x36034af6UL, 0x41047a60UL, 0xdf60efc3UL, 0xa867df55UL, 0x316e8eefUL,
  0x4669be79UL, 0xcb61b38cUL, 0xbc66831aUL, 0x256fd2a0UL, 0x5268e236UL,
  0xcc0c7795UL, 0xbb0b4703UL, 0x220216b9UL, 0x5505262fUL, 0xc5ba3bbeUL,
  0xb2bd0b28UL, 0x2bb45a92UL, 0x5cb36a04UL, 0xc2d7ffa7UL, 0xb5d0cf31UL,
  0x2cd99e8bUL, 0x5bdeae1dUL, 0x9b64c2b0UL, 0xec63f226UL, 0x756aa39cUL,
  0x026d930aUL, 0x9c0906a9UL, 0xeb0e363fUL, 0x72076785UL, 0x05005713UL,
  0x95bf4a82UL, 0xe2b87a14UL, 0x7bb12baeUL, 0x0cb61b38UL, 0x92d28e9bUL,
  0xe5d5be0dUL, 0x7cdcefb7UL, 0x0bdbdf21UL, 0x86d3d2d4UL, 0xf1d4e242UL,
  0x68ddb3f8UL, 0x1fda836eUL, 0x81be16cdUL, 0xf6b9265bUL, 0x6fb077e1UL,
  0x18b74777UL, 0x88085ae6UL, 0xff0f6a70UL, 0x66063bcaUL, 0x11010b5cUL,
  0x8f659effUL, 0xf862ae69UL, 0x616bffd3UL, 0x166ccf45UL, 0xa00ae278UL,
  0xd70dd2eeUL, 0x4e048354UL, 0x3903b3c2UL, 0xa7672661UL, 0xd06016f7UL,
  0x4969474dUL, 0x3e6e77dbUL, 0xaed16a4aUL, 0xd9d65adcUL, 0x40df0b66UL,
  0x37d83bf0UL, 0xa9bcae53UL, 0xdebb9ec5UL, 0x47b2cf7fUL, 0x30b5ffe9UL,
  0xbdbdf21cUL, 0xcabac28aUL, 0x53b39330UL, 0x24b4a3a6UL, 0xbad03605UL,
  0xcdd70693UL, 0x54de5729UL, 0x23d967bfUL, 0xb3667a2eUL, 0xc4614ab8UL,
  0x5d681b02UL, 0x2a6f2b94UL, 0xb40bbe37UL, 0xc30c8ea1UL, 0x5a05df1bUL,
  0x2d02ef8d
};

/*****************************************************************************/
/*! Create a CRC32 value from the given buffer data
*   \param ulCRC Continued CRC32 value
*   \param pabBuffer Buffer to create the CRC from
*   \param ulLength Buffer length
*   \return CRC32 value                                                      */
/*****************************************************************************/
static uint32_t CreateCRC32(uint32_t ulCRC, unsigned char* pabBuffer, unsigned long ulLength)
{
  if( (0 == pabBuffer) || (0 == ulLength) ) return ulCRC;
  ulCRC = ulCRC ^ 0xffffffff;
  for(;ulLength > 0; --ulLength)
  {
    ulCRC = (Crc32Table[((ulCRC) ^ (*(pabBuffer++)) ) & 0xff] ^ ((ulCRC) >> 8));
  }
  return ulCRC ^ 0xffffffff;
}

/*****************************************************************************/
/*! Download a file to the hardware
*   \param pvChannel          Channel instance the download is performed on
*   \param ulChannel          Channel number the download is for
*   \param ulMailboxSize      Size of the mailbox
*   \param ulTransferType     Type of transfer (see RCX_FILE_XFER_XXX defines)
*   \param szFileName         Short file name (needed by firmware to create the file by name)
*   \param ulFileLength       Length of the file to download
*   \param pvData             File data being downloaded
*   \param pfnTransferPacket  Function used for transferring packets
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
static int32_t DownloadFile(void*                 pvChannel,
                                 unsigned long         ulChannel,
                                 unsigned long         ulMailboxSize,
                                 unsigned long         ulTransferType,
                                 char*                 szFileName,
                                 void*                 pvFile,
                                 unsigned long         ulFileLength,
                                 PFN_TRANSFER_PACKET   pfnTransferPacket)
{
  /* Usually one brace should be enough, but GNU wants to have a second brace
     to initialize the structure. On GCC 4.0.3 the whole structure is initialized
     as described in ISOC90 */
  RCX_PACKET* ptBaseSendPkt = OS_Memalloc(sizeof(RCX_PACKET));//{{0}};
  RCX_PACKET* ptBaseRecvPkt = OS_Memalloc(sizeof(RCX_PACKET));//{{0}};

  /* Set download state informations */
  uint32_t ulMaxDataLength     = ulMailboxSize -  /* Maximum possible user data length */
                                 sizeof(RCX_FILE_DOWNLOAD_DATA_REQ_T);
  uint32_t ulSendLen           = 0;
  uint32_t ulTransferedLength  = 0;
  uint32_t ulCRC               = 0;
  uint32_t ulBlockNumber       = 0;
  uint32_t ulState             = RCX_FILE_DOWNLOAD_REQ;
  uint32_t ulCmdDataState      = RCX_PACKET_SEQ_NONE;
  int      fStopDownload       = 0;
  int32_t  lRetAbort           = NETANA_NO_ERROR;
  int32_t  lRet                = NETANA_NO_ERROR;
  uint32_t ulCurrentId         = 0;
  uint32_t ulCurrentOffset     = 0;

  /* Performce download */
  do
  {
    switch (ulState)
    {
      /* Send download request */
      case RCX_FILE_DOWNLOAD_REQ:
      {
        RCX_FILE_DOWNLOAD_REQ_T*  ptSendPkt = (RCX_FILE_DOWNLOAD_REQ_T*)ptBaseSendPkt;
        RCX_FILE_DOWNLOAD_CNF_T*  ptRecvPkt = (RCX_FILE_DOWNLOAD_CNF_T*)ptBaseRecvPkt;

        /* Validate filename length to fit mailbox/packet */
        unsigned long ulFileNameLength = min( (unsigned long)OS_Strlen(szFileName) + 1,
                                              ulMailboxSize - sizeof(RCX_FILE_DOWNLOAD_REQ_T));

        /* Insert packet data */
        ++ulCurrentId;
        ptSendPkt->tHead.ulDest   = HOST_TO_LE32(RCX_PACKET_DEST_SYSTEM);
        ptSendPkt->tHead.ulSrc    = 0;
        ptSendPkt->tHead.ulDestId = HOST_TO_LE32(0);
        ptSendPkt->tHead.ulSrcId  = HOST_TO_LE32(0);
        ptSendPkt->tHead.ulLen    = HOST_TO_LE32((uint32_t)(sizeof(RCX_FILE_DOWNLOAD_REQ_DATA_T) +
                                                                 ulFileNameLength));
        ptSendPkt->tHead.ulId     = HOST_TO_LE32(ulCurrentId);
        ptSendPkt->tHead.ulSta    = HOST_TO_LE32(0);
        ptSendPkt->tHead.ulCmd    = HOST_TO_LE32(RCX_FILE_DOWNLOAD_REQ);
        ptSendPkt->tHead.ulExt    = HOST_TO_LE32(ulCmdDataState);
        ptSendPkt->tHead.ulRout   = HOST_TO_LE32(0);

        /* Insert command data (extended data) */
        ptSendPkt->tData.ulFileLength     = HOST_TO_LE32(ulFileLength);
        ptSendPkt->tData.ulMaxBlockSize   = HOST_TO_LE32(ulMaxDataLength);
        ptSendPkt->tData.ulXferType       = HOST_TO_LE32(ulTransferType);
        ptSendPkt->tData.ulChannelNo      = HOST_TO_LE32(ulChannel);
        ptSendPkt->tData.usFileNameLength = HOST_TO_LE16((uint16_t)ulFileNameLength);
        OS_Strncpy( (char*)(ptSendPkt + 1),
                    szFileName,
                    ulFileNameLength) ;

        /* Transfer packet */
        lRet = pfnTransferPacket(pvChannel,
                                 ptBaseSendPkt,
                                 ptBaseRecvPkt,
                                 (uint32_t)sizeof(*ptBaseRecvPkt),
                                 TO_SEND_PACKET);

        if( (NETANA_NO_ERROR != lRet)                                 ||
            (RCX_S_OK        != (lRet = LE32_TO_HOST((int32_t)ptRecvPkt->tHead.ulSta))) )
        {
          /* Error during first packet, end download */
          /* Send abort request on unusable data */
          ulState = RCX_FILE_DOWNLOAD_ABORT_REQ;

        } else if( LE32_TO_HOST(ptRecvPkt->tData.ulMaxBlockSize) == 0)
        {
          /* Error in device information, stop download (Device returned illegal block size */
          lRet = NETANA_DOWNLOAD_FAILED;

          /* Send abort request on unusable data */
          ulState = RCX_FILE_DOWNLOAD_ABORT_REQ;

        } else
        {
          /* Everything went ok, so start transmitting file data now */

          /* Get download packet size from the device confirmation.
             If the devices packet size is smaller than our size, use the length from the device.
             Otherwise use our length. */
          if( ulMaxDataLength > LE32_TO_HOST(ptRecvPkt->tData.ulMaxBlockSize))
            ulMaxDataLength = LE32_TO_HOST(ptRecvPkt->tData.ulMaxBlockSize);

          /* Check if the file fits into one packet or if we have to send multiple packets */
          ulSendLen = ulMaxDataLength;
          if(ulFileLength <= ulSendLen)
          {
            /* We have only one packet to send */
            ulSendLen       = ulFileLength;
            ulCmdDataState  = RCX_PACKET_SEQ_NONE;
          } else
          {
            /* We have to send multiple packets */
            ulCmdDataState  = RCX_PACKET_SEQ_FIRST;
          }

          /* Goto next state */
          ulState = RCX_FILE_DOWNLOAD_DATA_REQ;
        }
      }
      break;

      /* Data download packets */
      case RCX_FILE_DOWNLOAD_DATA_REQ:
      {
        RCX_FILE_DOWNLOAD_DATA_REQ_T* ptSendPkt = (RCX_FILE_DOWNLOAD_DATA_REQ_T*)ptBaseSendPkt;
        RCX_FILE_DOWNLOAD_DATA_CNF_T* ptRecvPkt = (RCX_FILE_DOWNLOAD_DATA_CNF_T*)ptBaseRecvPkt;

        ++ulCurrentId;
        ptSendPkt->tHead.ulDest     = HOST_TO_LE32(RCX_PACKET_DEST_SYSTEM);
        ptSendPkt->tHead.ulSrc      = 0;
        ptSendPkt->tHead.ulCmd      = HOST_TO_LE32(RCX_FILE_DOWNLOAD_DATA_REQ);
        ptSendPkt->tHead.ulId       = HOST_TO_LE32(ulCurrentId);
        ptSendPkt->tHead.ulExt      = HOST_TO_LE32(ulCmdDataState);

        /* Copy file data to packet */
        OS_Memcpy( (&(ptSendPkt->tData) + 1), (void*)(((uint8_t*)pvFile)+ulCurrentOffset), ulSendLen);
        {
          /* Adjust packet length */
          ptSendPkt->tHead.ulLen      = HOST_TO_LE32((uint32_t)(sizeof(RCX_FILE_DOWNLOAD_DATA_REQ_DATA_T) +
                                                                     ulSendLen));

          /* Create continued CRC */
          ulCRC = CreateCRC32( ulCRC, (uint8_t*)(&(ptSendPkt->tData) + 1), ulSendLen);
          ptSendPkt->tData.ulChksum   = HOST_TO_LE32(ulCRC);
          ptSendPkt->tData.ulBlockNo  = HOST_TO_LE32(ulBlockNumber);
          ++ulBlockNumber;

          /* Transfer packet */
          lRet = pfnTransferPacket(pvChannel,
                                  (RCX_PACKET*)ptSendPkt,
                                  (RCX_PACKET*)ptRecvPkt,
                                  (uint32_t)sizeof(*ptBaseRecvPkt),
                                  TO_SEND_PACKET);
        }

        if( (NETANA_NO_ERROR != lRet)                                   ||
            (RCX_S_OK        != (lRet = LE32_TO_HOST((int32_t)(ptRecvPkt->tHead.ulSta)))) )
        {
          /* Driver error during transfer packet, end download */
          /* Always try to send an abort request */
          ulState = RCX_FILE_DOWNLOAD_ABORT_REQ;
        } else
        {
          /* Add send size to transfered size */
          ulTransferedLength += ulSendLen;

          /* Check if we are done with the download */
          if( (RCX_PACKET_SEQ_LAST == ulCmdDataState) ||
              (RCX_PACKET_SEQ_NONE == ulCmdDataState) )
          {
            /* No more packets to send, end download */
            fStopDownload = 1;
          } else
          {
            /* Move data pointer to next data */
            ulCurrentOffset += ulSendLen;

            /* Calculate next message length */
            if ( ulFileLength <= (ulSendLen + ulTransferedLength))
            {
              /* Set the send length to rest of data,
                 This will be the last packet */
              ulSendLen = ulFileLength - ulTransferedLength;
              ulCmdDataState = RCX_PACKET_SEQ_LAST;
            } else
            {
              ulCmdDataState = RCX_PACKET_SEQ_MIDDLE;
            }

            /* Goto next state */
            ulState = RCX_FILE_DOWNLOAD_DATA_REQ;
          }
        }
      }
      break;

      /* Abort active download */
      case RCX_FILE_DOWNLOAD_ABORT_REQ:
      {
        RCX_FILE_DOWNLOAD_ABORT_REQ_T*  ptSendPkt = (RCX_FILE_DOWNLOAD_ABORT_REQ_T*)ptBaseSendPkt;
        RCX_FILE_DOWNLOAD_ABORT_CNF_T*  ptRecvPkt = (RCX_FILE_DOWNLOAD_ABORT_CNF_T*)ptBaseRecvPkt;

        ++ulCurrentId;
        ptSendPkt->tHead.ulDest   = HOST_TO_LE32(RCX_PACKET_DEST_SYSTEM);
        ptSendPkt->tHead.ulSrc    = 0;
        ptSendPkt->tHead.ulDestId = HOST_TO_LE32(0);
        ptSendPkt->tHead.ulSrcId  = HOST_TO_LE32(0);
        ptSendPkt->tHead.ulLen    = HOST_TO_LE32(0);
        ptSendPkt->tHead.ulId     = HOST_TO_LE32(ulCurrentId);
        ptSendPkt->tHead.ulSta    = HOST_TO_LE32(0);
        ptSendPkt->tHead.ulCmd    = HOST_TO_LE32(RCX_FILE_DOWNLOAD_ABORT_REQ);
        ptSendPkt->tHead.ulExt    = HOST_TO_LE32(RCX_PACKET_SEQ_NONE);
        ptSendPkt->tHead.ulRout   = HOST_TO_LE32(0);

        /* Transfer packet */
        lRetAbort = pfnTransferPacket(pvChannel,
                                      ptBaseSendPkt,
                                      ptBaseRecvPkt,
                                      (uint32_t)sizeof(*ptBaseRecvPkt),
                                      TO_SEND_PACKET);

        if( lRetAbort == NETANA_NO_ERROR)
        {
          /* Return packet state if function succeeded */
          lRetAbort = LE32_TO_HOST((int32_t)ptRecvPkt->tHead.ulSta);
        }

        /* End download */
        fStopDownload = 1;
      }
      break;

      default:
        /* unkonwn, leave command */
        lRet = NETANA_FUNCTION_FAILED;

        /* End download */
        fStopDownload = 1;
        break;
    }

  } while(!fStopDownload);

  OS_Memfree(ptBaseSendPkt);
  OS_Memfree(ptBaseRecvPkt);

  /* Always return lRet first, thean abort error */
  if( NETANA_NO_ERROR != lRet)
    return lRet;
  else if( NETANA_NO_ERROR != lRetAbort)
    return lRetAbort;
  else
    return NETANA_NO_ERROR;
}

/*****************************************************************************/
/*! Create all IRQ events (16)
*   \param ptDevInstance  Device to create buffers for
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
static int32_t CreateIrqEvents(struct NETANA_DEVINSTANCE_T* ptDevInstance)
{
  uint32_t ulIdx;
  int32_t  lRet = NETANA_NO_ERROR;;

  if(NULL == (ptDevInstance->hStopComplete = OS_CreateEvent()))
  {
    lRet = NETANA_IRQEVENT_CREATION_FAILED;

  } else
  {
    for(ulIdx = 0;
        ulIdx < sizeof(ptDevInstance->ahHandshakeBitEvents) / sizeof(ptDevInstance->ahHandshakeBitEvents[0]);
        ++ulIdx)
    {
      if(NULL == (ptDevInstance->ahHandshakeBitEvents[ulIdx] = OS_CreateEvent()))
      {
        /* Creation of event failed */
        lRet = NETANA_IRQEVENT_CREATION_FAILED;
        break;
      }
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Create all IRQ events (16)
*   \param ptDevInstance  Device to create buffers for                       */
/*****************************************************************************/
static void DeleteIrqEvents(struct NETANA_DEVINSTANCE_T* ptDevInstance)
{
  uint32_t ulIdx;

  if(NULL != ptDevInstance->hStopComplete)
  {
    OS_DeleteEvent(ptDevInstance->hStopComplete);
    ptDevInstance->hStopComplete = NULL;
  }

  for(ulIdx = 0;
      ulIdx < sizeof(ptDevInstance->ahHandshakeBitEvents) / sizeof(ptDevInstance->ahHandshakeBitEvents[0]);
      ++ulIdx)
  {
    if(NULL != ptDevInstance->ahHandshakeBitEvents[ulIdx])
    {
      OS_DeleteEvent(ptDevInstance->ahHandshakeBitEvents[ulIdx]);
      ptDevInstance->ahHandshakeBitEvents[ulIdx] = NULL;
    }
  }
}

/*****************************************************************************/
/*! Performs a hardware reset on the given device
*   \param ptDevInstance Instance to reset
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
static int32_t netx_reset(struct NETANA_DEVINSTANCE_T* ptDevInstance)
{
  static const uint32_t s_aulResetSequence[] =
  {
    0x00000000, 0x00000001,0x00000003, 0x00000007,
    0x0000000F, 0x0000001F,0x0000003F, 0x0000007F,
    0x000000FF
  };

  void*         pvPCIConfig = NULL;
  unsigned long ulIdx = 0;
  long          lRet  = NETANA_HWRESET_ERROR;

  pvPCIConfig = OS_ReadPCIConfig(ptDevInstance->pvOSDependent);

  /* Perform netX Hardware Reset */
  for(ulIdx = 0; ulIdx < sizeof(s_aulResetSequence) / sizeof(s_aulResetSequence[0]); ++ulIdx)
    ptDevInstance->ptGlobalRegisters->ulHostReset = HOST_TO_LE32(s_aulResetSequence[ulIdx]);

  /* Wait until netX is in reset */
  OS_Sleep(NET_BOOTLOADER_RESET_TIME);

  /* Write PCI config */
  OS_WritePCIConfig(ptDevInstance->pvOSDependent, pvPCIConfig);

  /* Wait for romloader to signal PCI Boot State */
  for(ulIdx = 0; ulIdx < NET_BOOTLOADER_STARTUP_CYCLES; ++ulIdx)
  {
    unsigned long ulState;
    OS_Sleep(NET_BOOTLOADER_STARTUP_WAIT);

    ulState = LE32_TO_HOST(ptDevInstance->ptGlobalRegisters->ulSystemState);

    /* Check if state not 0xFFFFFFFF. This happens if memory is not available. */
    if( (ulState == NETX_DPM_INVALID_CONTENT)   ||
        (ulState == NETX_DPM_NOMEMORY_ASSIGNED)  )
    {
      /* Error, register block not available */
      lRet = NETANA_MEMORY_MAPPING_FAILED;

      if(g_ulTraceLevel & NETANA_TRACELEVEL_ERROR)
      {
        USER_Trace(ptDevInstance,
                   NETANA_TRACELEVEL_ERROR,
                   "DPM Content invalid after Reset (Data=0x%08X)!",
                   ulState);
      }
      break;

    } else
    {
      uint32_t ulCookie = *(uint32_t*)ptDevInstance->pvDPM;

      /* Check for netX50 reset complete */
      if(NETX50_BOOTID_DPM == ulCookie)
      {
        ptDevInstance->eChip = eCHIP_NETX50;
        lRet = NETANA_NO_ERROR;
        break;
      }

      /* After a RESET the SYSSTA state bit is 0 and so it must become 1
         to signal ROM loader is active on PCI. */
      if( (ulState & (MSK_SYSSTA_BOOT_ACTIVE | MSK_SYSSTA_LED_READY)) == (MSK_SYSSTA_BOOT_ACTIVE | MSK_SYSSTA_LED_READY) )
      {
        ptDevInstance->eChip = eCHIP_NETX100_500;
        lRet = NETANA_NO_ERROR;
        break;
      }
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Requests the device class.
*   \param ptDevInstance Device instance
*   \return the device class (-> != 0 on success)                            */
/*****************************************************************************/
uint16_t RequestDeviceClass( struct NETANA_DEVINSTANCE_T* ptDevInstance)
{
  RCX_PACKET*            ptBaseSendPkt = OS_Memalloc(sizeof(RCX_PACKET));
  RCX_PACKET*            ptBaseRecvPkt = OS_Memalloc(sizeof(RCX_PACKET));
  RCX_HW_IDENTIFY_REQ_T* ptSendPkt     = (RCX_HW_IDENTIFY_REQ_T*)ptBaseSendPkt;
  RCX_HW_IDENTIFY_CNF_T* ptRecvPkt     = (RCX_HW_IDENTIFY_CNF_T*)ptBaseRecvPkt;
  int32_t                lRet          = NETANA_NO_ERROR;
  uint16_t               usRet         = 0;

  if ((ptBaseSendPkt != NULL) && (ptBaseRecvPkt != NULL))
  {
    /* Create start request */
    ptSendPkt->tHead.ulDest = HOST_TO_LE32(RCX_PACKET_DEST_SYSTEM);
    ptSendPkt->tHead.ulLen  = HOST_TO_LE32(sizeof(RCX_HW_IDENTIFY_REQ_T));
    ptSendPkt->tHead.ulCmd  = HOST_TO_LE32(RCX_HW_IDENTIFY_REQ);
    ptSendPkt->tHead.ulLen  = 0;

    lRet = TransferPacket( ptDevInstance,
                           (RCX_PACKET*)ptSendPkt,
                           (RCX_PACKET*)ptRecvPkt,
                           sizeof(*ptBaseRecvPkt),
                           100);

    /* check for errors */
    if( NETANA_NO_ERROR != lRet)
    {
      if(g_ulTraceLevel & NETANA_TRACELEVEL_ERROR)
      {
        USER_Trace(ptDevInstance,
                   NETANA_TRACELEVEL_INFO,
                   "Error sending device class request 0x%X!", lRet);
      }
    } else if (RCX_S_OK != (lRet = LE32_TO_HOST(ptRecvPkt->tHead.ulSta)))
    {
      if(g_ulTraceLevel & NETANA_TRACELEVEL_ERROR)
      {
        USER_Trace(ptDevInstance,
                   NETANA_TRACELEVEL_INFO,
                   "Error requesting device class (0x%X)!", lRet);
      }
    } else
    {
      usRet = ptRecvPkt->tData.usDeviceClass;
    }
  }
  if (ptBaseSendPkt != NULL)
    OS_Memfree(ptBaseSendPkt);
  if (ptBaseRecvPkt != NULL)
    OS_Memfree(ptBaseRecvPkt);

  return usRet;
}

/*****************************************************************************/
/*! Downloads and starts the bootloader on netX100
*   \param ptDevInstance Instance to download the bootloader to (needs a reset
*                        before downloading)
*   \param pbFileData    Pointer to bootloader file data
*   \param ulFileDataLen Length of bootloader file
*   \return NETANA_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t netx100_start_bootloader(struct NETANA_DEVINSTANCE_T* ptDevInstance, void* hFile, unsigned long ulFileLen)
{
  int32_t lRet = NETANA_DOWNLOAD_FAILED;

  /* Copy file to DPM */
  if(ulFileLen != OS_FileRead(hFile, 0, ulFileLen, ptDevInstance->pvDPM))
  {
    lRet = NETANA_DOWNLOAD_FAILED;
  } else
  {
    int32_t   lStartTime;
    uint32_t  ulDiffTime;

    /* Toggle Start bit to let the second stage loader get started by netX ROMloader
      Set bit 7 (Host) equal to Bit 3 (netX) */
    uint32_t ulState = LE32_TO_HOST(ptDevInstance->ptGlobalRegisters->ulSystemState);

    if( ulState & MSK_SYSSTA_BOOT_ACTIVE)
      /* Bit 3 is set, set Bit 7 */
      ulState |= MSK_SYSSTA_BOOT_START;
    else
      /* Bit 3 is 0, clear Bit 7 */
      ulState &= ~MSK_SYSSTA_BOOT_START;

    ptDevInstance->ptGlobalRegisters->ulSystemState = HOST_TO_LE32(ulState);

    /* Wait for Cookie and NSF_READY Flag */
    lStartTime = OS_GetMilliSecCounter();

    do
    {
      NETX_SYSTEM_CHANNEL*    ptSysChannel  = (NETX_SYSTEM_CHANNEL*)ptDevInstance->pvDPM;
      NETX_HANDSHAKE_CHANNEL* ptHskChannel  = (NETX_HANDSHAKE_CHANNEL*)(ptSysChannel + 1);
      NETX_HANDSHAKE_CELL     tSysHskCell;
      uint32_t                ulCookie;

      OS_Sleep(10);

      OS_Memcpy(&ulCookie, ptSysChannel->tSystemInfo.abCookie, sizeof(ulCookie));
      tSysHskCell.ulValue = ptHskChannel->tSysFlags.ulValue;

      if((DPMSIGNATURE_BSL_VAL == ulCookie) || (DPMSIGNATURE_FW_VAL == ulCookie) )
      {
        if( (tSysHskCell.ulValue != DPM_NO_MEMORY_ASSIGNED) &&
            (tSysHskCell.ulValue != DPM_INVALID_CONTENT)    &&
            (tSysHskCell.t8Bit.bNetxFlags & NSF_READY) )
        {
          /* We are done with starting the netX */
          lRet = NETANA_NO_ERROR;
          break;
        }
      }

      ulDiffTime = OS_GetMilliSecCounter() - lStartTime;

    } while(ulDiffTime < NET_BOOTLOADER_STARTUP_TIME);
  }

  return lRet;
}

/*****************************************************************************/
/*! Bootloader download wrapper (netX50/100/500)
*   \param ptDevInstance  Devince instance
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
static int32_t netx_download_bootloader(struct NETANA_DEVINSTANCE_T* ptDevInstance)
{
  int32_t                   lRet;
  NETANA_TKIT_FILE_INFO_T   tFileInfo  = {{0}};
  void*                     hFile      = NULL;
  uint64_t                  ullFileLen = 0;

  switch(ptDevInstance->eChip)
  {
  case eCHIP_NETX100_500:
      USER_GetBootloaderFile(ptDevInstance, &tFileInfo);

      if(NULL == (hFile = OS_FileOpen(tFileInfo.szFullFileName, &ullFileLen, OS_FILE_FLAG_OPENEXISTING)))
      {
        if(g_ulTraceLevel & NETANA_TRACELEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                    NETANA_TRACELEVEL_ERROR,
                    "Error opening bootloader file '%s'!",
                    tFileInfo.szFullFileName);
        }

        lRet = NETANA_FILE_OPEN_ERROR;
      } else
      {
        lRet = netx100_start_bootloader(ptDevInstance, hFile, (uint32_t)ullFileLen);

        if(NETANA_NO_ERROR == lRet)
        {
          if(g_ulTraceLevel & NETANA_TRACELEVEL_INFO)
          {
            USER_Trace(ptDevInstance,
                      NETANA_TRACELEVEL_INFO,
                      "Successfully downloaded and started bootloader file '%s'!",
                      tFileInfo.szFullFileName);
          }
        } else if(g_ulTraceLevel & NETANA_TRACELEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                    NETANA_TRACELEVEL_ERROR,
                    "Error downloading bootloader file '%s' (lRet=%08X)!",
                    tFileInfo.szFullFileName,
                    lRet);
        }

        OS_FileClose(hFile);
      }
    break;

  case eCHIP_NETX50:
    /* netX50 is not a supported netANALYZER target. */
    lRet = NETANA_CHIP_NOT_SUPPORTED;
    break;

  default:
    /* unknown chip type */
    lRet = NETANA_CHIP_NOT_SUPPORTED;
    break;
  }

  return lRet;
}


/*****************************************************************************/
/*! update ident info
*   \param ptDevInstance  Devince instance
*   \param plError  the parameter contains the return value. Note: if this
                    parameter is given the the function returns always success.
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
static int32_t netx_update_ident_info(struct NETANA_DEVINSTANCE_T* ptDevInstance, int32_t* plError)
{
  NETANA_TKIT_FILE_INFO_T tFileInfo  = {{0}};
  void*                   hFile      = NULL;
  uint64_t                ullFileLen = 0;
  int32_t                 lRet       = NETANA_NO_ERROR;
  void*                   pvFileData;

  /* check if download is requested */
  if (ptDevInstance->tIdentInfoFile.fIdentUpdateRequest)
  {
    ptDevInstance->tIdentInfoFile.fIdentUpdateRequest = 0;

    /* Download and start firmware */
    USER_GetIdentInfoFile( ptDevInstance, &tFileInfo);

    if(NULL == (hFile = OS_FileOpen(tFileInfo.szFullFileName,
                                  &ullFileLen,
                                  OS_FILE_FLAG_OPENEXISTING)))
    {
      if(g_ulTraceLevel & NETANA_TRACELEVEL_ERROR)
      {
        USER_Trace(ptDevInstance,
                  NETANA_TRACELEVEL_ERROR,
                  "Error opening ident info file '%s'!",
                  tFileInfo.szFullFileName);
      }
      lRet = NETANA_FILE_OPEN_ERROR;
    } else
    {
      if (NULL != (pvFileData = OS_Memalloc((unsigned long)ullFileLen)))
      {
        OS_Memset( pvFileData, 0, (unsigned long)ullFileLen);
        if(ullFileLen != OS_FileRead( hFile, 0, (unsigned long)ullFileLen, pvFileData))
        {
          lRet = NETANA_FILE_READ_FAILED;
        } else
        {
          if(NETANA_NO_ERROR != (lRet = DownloadFile(ptDevInstance,
                                                      0,
                                                      NETX_SYSTEM_MAILBOX_MIN_SIZE,
                                                      RCX_FILE_XFER_LICENSE_CODE,
                                                      tFileInfo.szShortFileName,
                                                      pvFileData,
                                                      (uint32_t)ullFileLen,
                                                      TransferPacket)))
          {
            /* Error downloading firmware */
            if(g_ulTraceLevel & NETANA_TRACELEVEL_ERROR)
            {
              USER_Trace(ptDevInstance,
                       NETANA_TRACELEVEL_ERROR,
                       "Error downloading license file '%s' (lRet=0x%08X)!",
                       tFileInfo.szShortFileName,
                       lRet);
            }
          } else
          {
            if(g_ulTraceLevel & NETANA_TRACELEVEL_INFO)
            {
              USER_Trace(ptDevInstance,
                        NETANA_TRACELEVEL_INFO,
                        "Successfully downloaded license file!");
            }
          }
        }
        OS_Memfree(pvFileData);
      } else
      {
        lRet = NETANA_DOWNLOAD_FAILED;
      }
      OS_FileClose(hFile);
    }
    if (lRet)
    {
      USER_Trace(ptDevInstance,
                        NETANA_TRACELEVEL_INFO,
                        "Skipping download of license file!");
    }
  }
  ptDevInstance->tIdentInfoFile.lError = lRet;
  if (plError)
  {
    *plError                             = lRet;
    lRet                                 = NETANA_NO_ERROR;
  }
  return lRet;
}

/*****************************************************************************/
/*! Start netANALYZER Firmware
*   \param ptDevInstance Device instance
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
static int32_t StartFirmware(struct NETANA_DEVINSTANCE_T* ptDevInstance)
{
  RCX_CHANNEL_INSTANTIATE_REQ_T* ptSendPkt = OS_Memalloc( sizeof(RCX_CHANNEL_INSTANTIATE_REQ_T));//{{0}};
  RCX_PACKET*                    ptRecvPkt = OS_Memalloc( sizeof(RCX_PACKET));//{{0}};
  int32_t                        lRet;

  /* Create start request */
  ptSendPkt->tHead.ulDest      = HOST_TO_LE32(RCX_PACKET_DEST_SYSTEM);
  ptSendPkt->tHead.ulLen       = HOST_TO_LE32(sizeof(RCX_CHANNEL_INSTANTIATE_REQ_DATA_T));
  ptSendPkt->tHead.ulCmd       = HOST_TO_LE32(RCX_CHANNEL_INSTANTIATE_REQ);
  ptSendPkt->tData.ulChannelNo = HOST_TO_LE32(0);

  /* Transfer packet */
  lRet = TransferPacket( ptDevInstance,
                         (RCX_PACKET*)ptSendPkt,
                         (RCX_PACKET*)ptRecvPkt,
                         sizeof(*ptRecvPkt),
                         TO_SEND_PACKET);

  /* BSL will start our firmware after we have received the answer */
  if( (NETANA_NO_ERROR != lRet)                   ||
      (RCX_S_OK        != (lRet = LE32_TO_HOST(ptRecvPkt->tHeader.ulState))) )
  {
    /* Error starting the firmware */
    if(g_ulTraceLevel & NETANA_TRACELEVEL_ERROR)
    {
      USER_Trace(ptDevInstance,
                NETANA_TRACELEVEL_ERROR,
                "Error sending Start request for Firmware (lRet=0x%08X)!",
                lRet);
    }
  } else
  {
    /* Wait for netANALYZER cookie */
    int32_t  lStartTime = OS_GetMilliSecCounter();
    uint32_t ulDiffTime;

    lRet = NETANA_FW_START_FAILED;

    do
    {
      NETANA_BASE_DPM_T* ptDpm = (NETANA_BASE_DPM_T*)ptDevInstance->pvDPM;

      if(OS_Memcmp(NETANA_COOKIE,
                   (void*)ptDpm->tSystemInfoBlock.abCookie,
                   sizeof(ptDpm->tSystemInfoBlock.abCookie)) == 0)
      {
        /* We are done with starting the firmware */
        lRet = NETANA_NO_ERROR;
        break;
      }

      OS_Sleep(10);

      ulDiffTime = OS_GetMilliSecCounter() - lStartTime;

    } while(ulDiffTime < NET_BOOTLOADER_STARTUP_WAIT);
  }

  OS_Memfree(ptSendPkt);
  OS_Memfree(ptRecvPkt);

  return lRet;
}

/*****************************************************************************/
/*! Download the netANALYZER firmware using rcX packets, and start it afterwards
*   \param ptDevInstance Device instance
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
static int32_t DownloadAndStartFirmware(struct NETANA_DEVINSTANCE_T* ptDevInstance)
{
  NETANA_TKIT_FILE_INFO_T tFileInfo  = {{0}};
  void*                   hFile      = NULL;
  uint64_t                ullFileLen = 0;
  int32_t                 lRet;
  void*                   pvFileData;

  /* Download and start firmware */
  USER_GetFirmwareFile(ptDevInstance, &tFileInfo);

  if(NULL == (hFile = OS_FileOpen(tFileInfo.szFullFileName,
                                  &ullFileLen,
                                  OS_FILE_FLAG_OPENEXISTING)))
  {
    if(g_ulTraceLevel & NETANA_TRACELEVEL_ERROR)
    {
      USER_Trace(ptDevInstance,
                NETANA_TRACELEVEL_ERROR,
                "Error opening firmware file '%s'!",
                tFileInfo.szFullFileName);
    }

    lRet = NETANA_FILE_OPEN_ERROR;
  } else
  {
    pvFileData = OS_Memalloc((unsigned long)ullFileLen);
    OS_Memset( pvFileData, 0, (unsigned long)ullFileLen);

    if(ullFileLen != OS_FileRead( hFile, 0, (unsigned long)ullFileLen, pvFileData))
    {
      lRet = NETANA_FILE_READ_FAILED;
    } else
    {
      if(NETANA_NO_ERROR != (lRet = DownloadFile(ptDevInstance,
                                                   0,
                                                   NETX_SYSTEM_MAILBOX_MIN_SIZE,
                                                   RCX_FILE_XFER_MODULE,
                                                   tFileInfo.szShortFileName,
                                                   pvFileData,
                                                   (uint32_t)ullFileLen,
                                                   TransferPacket)))
      {
        /* Error downloading firmware */
        if(g_ulTraceLevel & NETANA_TRACELEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                    NETANA_TRACELEVEL_ERROR,
                    "Error downloading firmware file '%s' (lRet=0x%08X)!",
                    tFileInfo.szFullFileName,
                    lRet);
        }
      } else
      {
        if(g_ulTraceLevel & NETANA_TRACELEVEL_INFO)
        {
          USER_Trace(ptDevInstance,
                    NETANA_TRACELEVEL_INFO,
                    "Successfully downloaded firmware file '%s'!",
                    tFileInfo.szFullFileName);
        }

        if(NETANA_NO_ERROR != (lRet = StartFirmware(ptDevInstance)))
        {
          /* Error starting firmware */
          if(g_ulTraceLevel & NETANA_TRACELEVEL_ERROR)
          {
            USER_Trace(ptDevInstance,
                      NETANA_TRACELEVEL_ERROR,
                      "Error starting firmware file '%s' (lRet=0x%08X)!",
                      tFileInfo.szFullFileName,
                      lRet);
          }
        } else if(g_ulTraceLevel & NETANA_TRACELEVEL_INFO)
        {
          USER_Trace(ptDevInstance,
                    NETANA_TRACELEVEL_INFO,
                    "Successfully started firmware file '%s'!",
                    tFileInfo.szFullFileName);
        }
      }
    }
    OS_Memfree(pvFileData);
    OS_FileClose(hFile);
  }

  return lRet;
}

/*****************************************************************************/
/*! Add a device to toolkit control. Minimum parameters are name and DPM pointer.
*   \param ptDevInstance Device to add (needed paramers : pvDPM, szDeviceName)
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t netana_tkit_deviceadd(struct NETANA_DEVINSTANCE_T* ptDevInstance, uint32_t ulDevNo)
{
  int32_t lRet;
  int32_t lErrorIdentUpdate;

  /* NOTE: Unique device name creation needs to be done by user */

  /* Create Lock for handshake bit access */
  if(NULL == (ptDevInstance->pvLock = OS_CreateLock()))
  {
    lRet = NETANA_IRQLOCK_CREATION_FAILED;

  /* Create DMA Buffers */
  } else if(NETANA_NO_ERROR != (lRet = CreateIrqEvents(ptDevInstance)))
  {
    if(g_ulTraceLevel & NETANA_TRACELEVEL_ERROR)
    {
      USER_Trace(ptDevInstance, NETANA_TRACELEVEL_ERROR,
                 "Error creating IRQ events!");
    }
  } else
  {
    ptDevInstance->ptGlobalRegisters = (PNETX_GLOBAL_REG_BLOCK)( (uint8_t*)ptDevInstance->pvDPM +
                                                                 ptDevInstance->ulDPMSize -
                                                                 sizeof(*ptDevInstance->ptGlobalRegisters));

    /* Disable Interrupts on Device physically if they are still enabled */
    ptDevInstance->ptGlobalRegisters->ulIRQEnable_0 = 0;

    /* Reset device */
    if(NETANA_NO_ERROR != (lRet = netx_reset(ptDevInstance)))
    {
      if(g_ulTraceLevel & NETANA_TRACELEVEL_ERROR)
      {
        USER_Trace(ptDevInstance, NETANA_TRACELEVEL_ERROR,
                   "Error resetting netX chip (lRet=0x%08X)!",
                   lRet);
      }

      /* Download 2nd Stage Bootloader */
    } else if(NETANA_NO_ERROR != (lRet = netx_download_bootloader(ptDevInstance)))
    {
      if(g_ulTraceLevel & NETANA_TRACELEVEL_ERROR)
      {
        USER_Trace(ptDevInstance, NETANA_TRACELEVEL_ERROR,
                   "Error downloading bootloader to device (lRet=0x%08X)!",
                   lRet);
      }
      /* retrieve device class (required to be able to select the correct firmware) */
    } else if (0 == (ptDevInstance->usDeviceClass = RequestDeviceClass(ptDevInstance)))
    {
      lRet = NETANA_FUNCTION_FAILED;
      if(g_ulTraceLevel & NETANA_TRACELEVEL_ERROR)
      {
        USER_Trace(ptDevInstance, NETANA_TRACELEVEL_ERROR,
                   "Error while identifying device!");
      }
    } else if(NETANA_NO_ERROR != (lRet = netx_update_ident_info(ptDevInstance, &lErrorIdentUpdate)))
    {
      if(g_ulTraceLevel & NETANA_TRACELEVEL_ERROR)
      {
        USER_Trace(ptDevInstance, NETANA_TRACELEVEL_ERROR,
                   "Error downloading licensefile to device (lRet=0x%08X)!",
                   lRet);
      }
      /* Download and Start Firmware */
    } else if(NETANA_NO_ERROR != (lRet = DownloadAndStartFirmware(ptDevInstance)))
    {
      if(g_ulTraceLevel & NETANA_TRACELEVEL_ERROR)
      {
        USER_Trace(ptDevInstance, NETANA_TRACELEVEL_ERROR,
                   "Error starting firmware on device (lRet=0x%08X)!",
                   lRet);
      }

    } else
    {
      /* Read out DPM Layout.
         DPM always starts with a Base DPM followed by dynamically sized blocks in
         the following order:
         1. GPIO Config Block (Count is given through SystemInfoBlock.ulGPIOCount
         2. Filter Block      (Count is given through SystemInfoBlock.ulPortCount
                               Size is given through FilterSize * 4 (Mask and Value for
                                 each of both filters)
         3. Counter Block     (Count is given through SystemInfoBlock.ulPortCount
                               Size is given through CounterSize)
      */
      NETANA_BASE_DPM_T*    ptDpm          = (NETANA_BASE_DPM_T*)ptDevInstance->pvDPM;
      uint8_t*              pbCurrentBlock = (uint8_t*)(ptDpm + 1);
      uint32_t              ulIdx;
      NETANA_HANDSHAKE_CELL tHskCell;

      /* check if ident update succeded, if not log warning */
      if (NETANA_NO_ERROR != lErrorIdentUpdate)
      {
        USER_Trace(ptDevInstance, NETANA_TRACELEVEL_ERROR,
                   "Error downloading licensefile to device (lRet=0x%08X)!",
                   lErrorIdentUpdate);
      }

      ptDevInstance->ptBaseDPM = ptDpm;

      ptDevInstance->ulDeviceNr   = ptDpm->tSystemInfoBlock.ulDeviceNr;
      ptDevInstance->ulSerialNr   = ptDpm->tSystemInfoBlock.ulSerialNr;

      ptDevInstance->ulGpioCnt    = ptDpm->tSystemInfoBlock.ulGPIOCount;

      /* Read all GPIO Blocks */
      for(ulIdx = 0; ulIdx < ptDevInstance->ulGpioCnt; ++ulIdx)
      {
        ptDevInstance->aptGPIOs[ulIdx] = (NETANA_GPIO_BLOCK_T*)pbCurrentBlock;
        pbCurrentBlock += sizeof(NETANA_GPIO_BLOCK_T);
        /* store default configuration of the gpios                               */
        /* if the device becomes opended always reset gpios to this configuration */
        OS_Memcpy( &ptDevInstance->atDefaultGPIOs[ulIdx], ptDevInstance->aptGPIOs[ulIdx], sizeof(NETANA_GPIO_BLOCK_T));
      }

      ptDevInstance->ulPortCnt    = ptDpm->tSystemInfoBlock.ulPortCount;
      ptDevInstance->ulFilterSize = ptDpm->tSystemInfoBlock.ulFilterSize;

      /* Read all Filter Blocks */
      for(ulIdx = 0; ulIdx < ptDevInstance->ulPortCnt; ++ulIdx)
      {
        NETANA_DPM_FILTER_T* ptFilter = &ptDevInstance->atFilters[ulIdx];
        uint32_t             ulFilter;

        /* Store offset of Filter Blocks */
        ptDevInstance->atFilters[ulIdx].ptBase = (NETANA_FILTER_BASE_T*)pbCurrentBlock;
        pbCurrentBlock += sizeof(*ptDevInstance->atFilters[ulIdx].ptBase);

        for(ulFilter = 0; ulFilter < NETANA_MAX_FILTERS; ++ulFilter)
        {
          /* Each filter consists of a mask and value --> 2 * FilterSize needed */
          ptFilter->atFilter[ulFilter].pbMask = pbCurrentBlock;
          pbCurrentBlock += ptDevInstance->ulFilterSize;

          ptFilter->atFilter[ulFilter].pbValue = pbCurrentBlock;
          pbCurrentBlock += ptDevInstance->ulFilterSize;

          /* store default configuration of the filter                               */
          /* if the device becomes opended always reset filter to this configuration */
          ptDevInstance->atDefaultFilter[ulIdx].atFilter[ulFilter].pbMask  = OS_Memalloc( ptDevInstance->ulFilterSize);
          ptDevInstance->atDefaultFilter[ulIdx].atFilter[ulFilter].pbValue = OS_Memalloc( ptDevInstance->ulFilterSize);

          if (NULL != ptDevInstance->atDefaultFilter[ulIdx].atFilter[ulFilter].pbMask)
            OS_Memcpy( ptDevInstance->atDefaultFilter[ulIdx].atFilter[ulFilter].pbMask, ptFilter->atFilter[ulFilter].pbMask, ptDevInstance->ulFilterSize);
          if (NULL != ptDevInstance->atDefaultFilter[ulIdx].atFilter[ulFilter].pbValue)
            OS_Memcpy( ptDevInstance->atDefaultFilter[ulIdx].atFilter[ulFilter].pbValue, ptFilter->atFilter[ulFilter].pbValue, ptDevInstance->ulFilterSize);
        }
      }

      ptDevInstance->ulCounterSize = ptDpm->tSystemInfoBlock.ulCounterSize;
      /* Read all counter blocks */
      for(ulIdx = 0; ulIdx < ptDevInstance->ulPortCnt; ++ulIdx)
      {
        ptDevInstance->aptCounters[ulIdx] = (NETANA_COUNTER_BLOCK_T*)pbCurrentBlock;
        pbCurrentBlock += ptDevInstance->ulCounterSize;
      }
      //TODO:
      ptDevInstance->tIdentInfoBlock.pbIdentInfoBlock = pbCurrentBlock;
      pbCurrentBlock += IDENT_INFO_BLOCK_SIZE;

      ptDevInstance->ptHandshakeBlock = (NETANA_HANDSHAKE_BLOCK_T*)(((uint8_t*)ptDpm) +
                                                                    ptDpm->tSystemInfoBlock.ulDPMUsedSize -
                                                                    sizeof(*ptDevInstance->ptHandshakeBlock));

      /* Read Handshake flags for the first time */
      tHskCell.ulValue = ptDevInstance->ptHandshakeBlock->atHandshake[0].ulValue;

      ptDevInstance->ulNetxFlags = tHskCell.t16Bit.usNetxFlags;
      ptDevInstance->ulHostFlags = tHskCell.t16Bit.usHostFlags;

      /* Handle all pending state change indications here, as firmware might
         already have signalled the first state change (CAPTURE_STOP) */
      if((ptDevInstance->ulNetxFlags ^ ptDevInstance->ulHostFlags) & NETANA_HSK_HOST_NEWSTATUS_IND_MSK)
        netana_tkit_ToggleBit(ptDevInstance, NETANA_HSK_HOST_NEWSTATUS_IND);

      switch (ptDevInstance->usDeviceClass)
      {
        case NSCP_DEV_CLASS:
        {
          char szAliasName[NETANA_MAX_DEVICENAMESIZE];
          USER_GetAliasName( NSCP_DEV_CLASS, szAliasName, NETANA_MAX_DEVICENAMESIZE);
          OS_Snprintf( ptDevInstance->szAliasName, sizeof(ptDevInstance->szAliasName), "%s%s%d", szAliasName,"_", ulDevNo);
          ptDevInstance->ulDevNo = ulDevNo;
        }
        break;
        case NANL_DEV_CLASS:
        {
          char szAliasName[NETANA_MAX_DEVICENAMESIZE];
          USER_GetAliasName( NANL_DEV_CLASS, szAliasName, NETANA_MAX_DEVICENAMESIZE);
          OS_Snprintf( ptDevInstance->szAliasName, sizeof(ptDevInstance->szAliasName), "%s%s%d", szAliasName, "_", ulDevNo);
          ptDevInstance->ulDevNo = ulDevNo;
        }
        break;
        case CIFX_DEV_CLASS:
        default:
        {
          char szAliasName[NETANA_MAX_DEVICENAMESIZE];
          USER_GetAliasName( CIFX_DEV_CLASS, szAliasName, NETANA_MAX_DEVICENAMESIZE);
          OS_Snprintf( ptDevInstance->szAliasName, sizeof(ptDevInstance->szAliasName), "%s%s%d", szAliasName, "_", ulDevNo);
          ptDevInstance->ulDevNo = ulDevNo;
        }
        break;
      }
      /* setup mailbox system */
      /* send mailbox resources */
      ptDevInstance->tSendMbx.ptSendMailboxStart  = (NETANA_SEND_MAILBOX_BLOCK*)((uint8_t*)ptDevInstance->pvDPM + DPM_SEND_MBX_OFFSET);
      ptDevInstance->tSendMbx.ulSendMailboxLength = NETANA_CHANNEL_MAILBOX_SIZE;      /*!< Length of send mailbox in bytes           */
      ptDevInstance->tSendMbx.ulSendCMDBitmask    = NETANA_HSK_HOST_SEND_MBX_CMD_MSK; /*!< Bitmask for Handshakeflags to send packet */
      ptDevInstance->tSendMbx.bSendCMDBitoffset   = NETANA_HSK_HOST_SEND_MBX_CMD;     /*!< Bitnumber for send packet flag (used for notification array indexing) */
      ptDevInstance->tSendMbx.pvSendMBXMutex      = OS_CreateMutex();            /*!< Synchronization object for the send mailbox    */
      ptDevInstance->tSendMbx.ulSendPacketCnt     = 0;                           /*!< Number of packets sent on this mailbox         */
      ptDevInstance->tSendMbx.pfnCallback         = NULL;                        /*!< reset notification callback                    */
      ptDevInstance->tSendMbx.pvUser              = NULL;                        /*!< clear user parameter                           */
      ptDevInstance->tSendMbx.ptSendMailboxStart->usPackagesAccepted  = 0;

      /* receive mailbox resources */
      ptDevInstance->tRecvMbx.ptRecvMailboxStart  = (NETANA_RECV_MAILBOX_BLOCK*)((uint8_t*)ptDevInstance->pvDPM + DPM_RECV_MBX_OFFSET);
      ptDevInstance->tRecvMbx.ulRecvMailboxLength = NETANA_CHANNEL_MAILBOX_SIZE;
      ptDevInstance->tRecvMbx.ulRecvACKBitmask    = NETANA_HSK_HOST_RECV_MBX_IND_ACK_MSK;
      ptDevInstance->tRecvMbx.bRecvACKBitoffset   = NETANA_HSK_HOST_RECV_MBX_IND_ACK;
      ptDevInstance->tRecvMbx.pvRecvMBXMutex      = OS_CreateMutex();
      ptDevInstance->tRecvMbx.ulRecvPacketCnt     = 0;
      ptDevInstance->tRecvMbx.pfnCallback         = NULL;
      ptDevInstance->tRecvMbx.pvUser              = NULL;
      ptDevInstance->tRecvMbx.ptRecvMailboxStart->usWaitingPackages = 0;
    }
  }

  if(NETANA_NO_ERROR == lRet)
  {
    OS_EnterLock(g_pvCardLock);

    STAILQ_INSERT_TAIL(&g_tCardList, ptDevInstance, tList);
    ++g_ulCardCnt;

    OS_LeaveLock(g_pvCardLock);

    if(NETANA_TKIT_DSR_PROCESSING_REQUEST == netana_tkit_isr_handler( ptDevInstance, 1))
        netana_tkit_dsr_handler(ptDevInstance);

    OS_EnableDeviceInterrupts(ptDevInstance->pvOSDependent);

    /* We will not enable Interrupts here. This breaks KMDF and must be
       done in EvtInterruptEnable after D1->D0 transition.
       The User needs to call netana_tkit_enable_hwinterrupts / netana_tkit_disable_hwinterrupts
       one the Interrupts have been successfully configured/enabled */
  } else
  {
    /* Error */
    DeleteIrqEvents(ptDevInstance);

    if(NULL != ptDevInstance->pvLock)
    {
      OS_DeleteLock(ptDevInstance->pvLock);
      ptDevInstance->pvLock = NULL;
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Remove a device from toolkit control
*   \param ptDevInstance Device instance
*   \param fForceRemove  !=0 to remove a device which is still accessed by an application
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t netana_tkit_deviceremove(struct NETANA_DEVINSTANCE_T* ptDevInstance, int fForceRemove)
{
  int32_t  lRet = NETANA_NO_ERROR;
  uint32_t ulFile;

  if((ptDevInstance->ulOpenCnt > 0) && !fForceRemove)
  {
    /* Device is still accessed */
    lRet = NETANA_DEVICE_STILL_OPEN;
  } else
  {
    lRet = NETANA_FUNCTION_FAILED;

    /* Signal Clockout disable to Firmware,
       NOTE: This will result the firmware in entering a safe state. Restart
             is only possible after re-downloading the firmware */
    netana_tkit_ToggleBit(ptDevInstance, NETANA_HSK_HOST_CLKOFF_CMD);

    /* Wait for acknowledge, if we disable the device without waiting, */
    /* the PHYs might be in an inoperable state after a reset.         */
    if(netana_tkit_WaitForBitState( ptDevInstance,
                                    NETANA_HSK_NETX_CLKOFF_ACK,
                                    NETANA_BITSTATE_EQUAL,
                                    NETANA_DEFAULT_TIMEOUT))
    {
      lRet = NETANA_NO_ERROR;
    }
    /* Disable Interrupts on Device physically */
    ptDevInstance->ptGlobalRegisters->ulIRQEnable_0 = 0;

    OS_DisableDeviceInterrupts(ptDevInstance->pvOSDependent);

    DeleteIrqEvents(ptDevInstance);

    if(NULL != ptDevInstance->pvLock)
    {
      OS_DeleteLock(ptDevInstance->pvLock);
      ptDevInstance->pvLock = NULL;
    }

    for(ulFile = 0; ulFile < ptDevInstance->ulFileCount; ++ulFile)
    {
      NETANA_FILE_DATA_T* ptFile = &ptDevInstance->atFiles[ulFile];

      if(NULL != ptFile->hFile)
      {
        OS_FileClose(ptFile->hFile);
        ptFile->hFile = NULL;
      }
    }

    OS_EnterLock(g_pvCardLock);

    STAILQ_REMOVE(&g_tCardList, ptDevInstance, NETANA_DEVINSTANCE_T, tList);
    --g_ulCardCnt;

    OS_LeaveLock(g_pvCardLock);
  }

  return lRet;
}

/*****************************************************************************/
/*! Function to enable hardware interrupts on the netANALYZER card physically.
*   This function should be called, when the OS is ready to handle our interrupts
*   \param ptDevInstance Device instance                                     */
/*****************************************************************************/
void netana_tkit_enable_hwinterrupts(struct NETANA_DEVINSTANCE_T* ptDevInstance)
{
  ptDevInstance->ptGlobalRegisters->ulIRQEnable_0 = MSK_IRQ_EN0_INT_REQ |
                                                    0x00000001;
}

/*****************************************************************************/
/*! Function to disable hardware interrupts on the netANALYZER card physically.
*   This function should be called, before the OS disconnects it's ISR
*   \param ptDevInstance Device instance                                     */
/*****************************************************************************/
void netana_tkit_disable_hwinterrupts(struct NETANA_DEVINSTANCE_T* ptDevInstance)
{
  ptDevInstance->ptGlobalRegisters->ulIRQEnable_0 = 0;
}
