/* SPDX-License-Identifier: MIT */

#ifndef __NETANA_DPMLAYOUT__H
#define __NETANA_DPMLAYOUT__H

#include <OS_Includes.h>

#ifdef __cplusplus
  extern "C" {
#endif  /* _cplusplus */

#ifdef _MSC_VER
  #if _MSC_VER >= 1000
    #define __NETANA_PACKED_PRE
    #define __NETANA_PACKED_POST
    #pragma once
    #pragma pack(1)            /* Always align structures to 1Byte boundery */
    #ifndef STRICT             /* Check Typedefinition */
      #define STRICT
    #endif
  #endif /* _MSC_VER >= 1000 */
#endif /* _MSC_VER */

/* support for GNU compiler */
#ifdef __GNUC__
  #define __NETANA_PACKED_PRE
  #define __NETANA_PACKED_POST  __attribute__((packed))
#endif

#define NETANA_COOKIE       "netANALYZER"

#define NETANA_MAX_DMA_BUFFERS  32

#define NETANA_NEWDATA_BIT       (0)
#define NETANA_NEWSTATUS_BIT     (1)
#define NETANA_STARTSTOP_BIT     (2)
#define NETANA_MDIO_BIT          (3)
#define NETANA_UPDATE_BIT        (4)
#define NETANA_DRV_ERROR_BIT     (5)
#define NETANA_BLINK_BIT         (6)
#define NETANA_RESYNC_BIT        (7)
#define NETANA_LICENSE_CHECK_BIT (8)
#define NETANA_SEND_MBX_BIT      (9)
#define NETANA_RECV_MBX_BIT      (10)

#define NETANA_CLKOFF_BIT        (15)

/* netX --> Host Handshake bits */
#define NETANA_HSK_NETX_NEWDATA_IND       NETANA_NEWDATA_BIT
#define NETANA_HSK_NETX_NEWDATA_IND_MSK   (1 << NETANA_NEWDATA_BIT)
#define NETANA_HSK_NETX_NEWSTATUS_IND     NETANA_NEWSTATUS_BIT
#define NETANA_HSK_NETX_NEWSTATUS_IND_MSK (1 << NETANA_NEWSTATUS_BIT)
#define NETANA_HSK_NETX_STARTSTOP_ACK     NETANA_STARTSTOP_BIT
#define NETANA_HSK_NETX_STARTSTOP_ACK_MSK (1 << NETANA_STARTSTOP_BIT)
#define NETANA_HSK_NETX_MDIO_ACK          NETANA_MDIO_BIT
#define NETANA_HSK_NETX_MDIO_ACK_MSK      (1 << NETANA_MDIO_BIT)
#define NETANA_HSK_NETX_UPDATE_ACK        NETANA_UPDATE_BIT
#define NETANA_HSK_NETX_UPDATE_ACK_MSK    (1 << NETANA_UPDATE_BIT)
#define NETANA_HSK_NETX_DRV_ERROR_ACK     NETANA_DRV_ERROR_BIT
#define NETANA_HSK_NETX_DRV_ERROR_ACK_MSK (1 << NETANA_DRV_ERROR_BIT)
#define NETANA_HSK_NETX_BLINK_ACK         NETANA_BLINK_BIT
#define NETANA_HSK_NETX_BLINK_ACK_MSK     (1 << NETANA_BLINK_BIT)
#define NETANA_HSK_NETX_CLKOFF_ACK        NETANA_CLKOFF_BIT
#define NETANA_HSK_NETX_CLKOFF_ACK_MSK    (1 << NETANA_CLKOFF_BIT)
#define NETANA_HSK_NETX_RESYNC_ACK        NETANA_RESYNC_BIT
#define NETANA_HSK_NETX_RESYNC_ACK_MSK    (1 << NETANA_RESYNC_BIT)
#define NETANA_HSK_NETX_LICENSE_ACK       NETANA_LICENSE_CHECK_BIT
#define NETANA_HSK_NETX_LICENSE_ACK_MSK   (1 << NETANA_LICENSE_CHECK_BIT)
#define NETANA_HSK_NETX_SEND_MBX_ACK      NETANA_SEND_MBX_BIT
#define NETANA_HSK_NETX_SEND_MBX_ACK_MSK  (1 << NETANA_SEND_MBX_BIT)
#define NETANA_HSK_NETX_RECV_MBX_IND      NETANA_RECV_MBX_BIT
#define NETANA_HSK_NETX_RECV_MBX_IND_MSK  (1 << NETANA_RECV_MBX_BIT)

/* Host --> netX Handshake bits */
#define NETANA_HSK_HOST_NEWDATA_IND          NETANA_NEWDATA_BIT
#define NETANA_HSK_HOST_NEWDATA_IND_MSK      (1 << NETANA_NEWDATA_BIT)
#define NETANA_HSK_HOST_NEWSTATUS_IND        NETANA_NEWSTATUS_BIT
#define NETANA_HSK_HOST_NEWSTATUS_IND_MSK    (1 << NETANA_NEWSTATUS_BIT)
#define NETANA_HSK_HOST_STARTSTOP_CMD        NETANA_STARTSTOP_BIT
#define NETANA_HSK_HOST_STARTSTOP_CMD_MSK    (1 << NETANA_STARTSTOP_BIT)
#define NETANA_HSK_HOST_MDIO_CMD             NETANA_MDIO_BIT
#define NETANA_HSK_HOST_MDIO_CMD_MSK         (1 << NETANA_MDIO_BIT)
#define NETANA_HSK_HOST_UPDATE_CMD           NETANA_UPDATE_BIT
#define NETANA_HSK_HOST_UPDATE_CMD_MSK       (1 << NETANA_UPDATE_BIT)
#define NETANA_HSK_HOST_DRV_ERROR_CMD        NETANA_DRV_ERROR_BIT
#define NETANA_HSK_HOST_DRV_ERROR_CMD_MSK    (1 << NETANA_DRV_ERROR_BIT)
#define NETANA_HSK_HOST_BLINK_CMD            NETANA_BLINK_BIT
#define NETANA_HSK_HOST_BLINK_CMD_MSK        (1 << NETANA_BLINK_BIT)
#define NETANA_HSK_HOST_CLKOFF_CMD           NETANA_CLKOFF_BIT
#define NETANA_HSK_HOST_CLKOFF_CMD_MSK       (1 << NETANA_CLKOFF_BIT)
#define NETANA_HSK_HOST_RESYNC_CMD           NETANA_RESYNC_BIT
#define NETANA_HSK_HOST_RESYNC_CMD_MSK       (1 << NETANA_RESYNC_BIT)
#define NETANA_HSK_HOST_LICENSE_CMD          NETANA_LICENSE_CHECK_BIT
#define NETANA_HSK_HOST_LICENSE_CMD_MSK      (1 << NETANA_LICENSE_CHECK_BIT)
#define NETANA_HSK_HOST_SEND_MBX_CMD         NETANA_SEND_MBX_BIT
#define NETANA_HSK_HOST_SEND_MBX_CMD_MSK     (1 << NETANA_SEND_MBX_BIT)
#define NETANA_HSK_HOST_RECV_MBX_IND_ACK     NETANA_RECV_MBX_BIT
#define NETANA_HSK_HOST_RECV_MBX_IND_ACK_MSK (1 << NETANA_RECV_MBX_BIT)


typedef __NETANA_PACKED_PRE struct NETANA_SYSTEM_INFO_BLOCK_Ttag
{
  volatile uint8_t   abCookie[12];        /*!< 0x00 : DPM Identification Cookie (must be NETANA_COOKIE) */
  volatile uint32_t  ulSystemError;       /*!< 0x0C : System Error, e.g. Security Memory error          */
  volatile uint32_t  ulDPMLayoutVersion;  /*!< 0x10 : Version of the DPM Layout                         */
  volatile uint32_t  ulDPMTotalSize;      /*!< 0x14 : Total DPM size                                    */
  volatile uint32_t  ulDPMUsedSize;       /*!< 0x18 : Used DPM in bytes                                 */
  uint32_t           aulReserved1[2];
  volatile uint32_t  ulDeviceNr;          /*!< 0x20 : Device Number                                     */
  volatile uint32_t  ulSerialNr;          /*!< 0x24 : Serial Number                                     */
  volatile uint16_t  ausHwOptions[4];     /*!< 0x28 : Hardware Options                                  */
  volatile uint16_t  usManufacturer;      /*!< 0x30 : Manufacturer Code                                 */
  volatile uint16_t  usProductionDate;    /*!< 0x32 : Production date                                   */
  volatile uint16_t  usDeviceClass;       /*!< 0x34 : Device class                                      */
  volatile uint8_t   bHwRevision;         /*!< 0x36 : Hardware revision                                 */
  volatile uint8_t   bHwCompatibility;    /*!< 0x37 : Hardware compatibility                            */
  uint32_t           aulReserved2[2];
  
  volatile uint8_t   szFirmwareName[32];  /*!< 0x40 : Firmware name */
  volatile uint32_t  ulVersionMajor;      /*!< 0x60 : Firmware version : Major */
  volatile uint32_t  ulVersionMinor;      /*!< 0x64 : Firmware version : Minor */
  volatile uint32_t  ulVersionBuild;      /*!< 0x68 : Firmware version : Build */
  volatile uint32_t  ulVersionRevision;   /*!< 0x6C : Firmware version : Revision */

  volatile uint32_t  ulPortCount;         /*!< 0x70 : Number of analyzer ports                          */
  volatile uint32_t  ulGPIOCount;         /*!< 0x74 : Number of GPIO ports                              */
  volatile uint32_t  ulFilterSize;        /*!< 0x78 : Size of the hardware filters                      */
  volatile uint32_t  ulCounterSize;       /*!< 0x7C : Size of the counter area (per port)               */
  
  volatile uint32_t  ulLicenseFlags1;
  volatile uint32_t  ulLicenseFlags2;
  volatile uint16_t  usNetXLicenseID;
  volatile uint16_t  usNetXLicenseFlags;  
  uint32_t           aulReserved3[2];

  volatile uint32_t  ulPhysType;
  volatile uint32_t  ulPhysTapPresent;
  volatile uint32_t  ulPhysForwardingSupport;
  volatile uint32_t  ulPhysPortSpeedSupport;
  volatile uint32_t  ulPhysTransparentModeSupport;
  volatile uint32_t  ulGpioInputRisingSupport;
  volatile uint32_t  ulGpioInputFallingSupport;
  volatile uint32_t  ulGpioOutputModeSupport;
  volatile uint32_t  ulGpioOutputPWMSupport;
  volatile uint32_t  ulGpioSyncInSupport;
  volatile uint32_t  ulGpioTriggerStartSupport;
  volatile uint32_t  ulGpioTriggerStopSupport;
  volatile uint32_t  ulGpioVoltage3VSupport;
  volatile uint32_t  ulGpioVoltage24VSupport;
  volatile uint32_t  ulSyncSupport;

  uint32_t           aulReserved4[11];

} __NETANA_PACKED_POST NETANA_SYSTEM_INFO_BLOCK_T;

typedef __NETANA_PACKED_PRE struct NETANA_HOST_DMA_BUFFER_Ttag
{
  volatile uint32_t ulPhysicalAddressHigh;  /*!< Currently unused, as netX can only access 32 Bit */
  volatile uint32_t ulPhysicalAddressLow;   /*!< Physical buffer address of host                  */
  volatile uint32_t ulBufferSize;
  volatile uint32_t ulReserved;
  
} __NETANA_PACKED_POST NETANA_HOST_DMA_BUFFER_T;

typedef __NETANA_PACKED_PRE struct NETANA_HOST_CONTROL_BLOCK_Ttag
{
  volatile uint32_t         ulBufferFlags;                        /*!< 0x000: Bitmask for used buffers (toggle bit with NETX STATUS BLOCK) */
  volatile uint32_t         ulBufferCount;                        /*!< 0x004: Number of valid host buffers */
  volatile uint32_t         ulHostError;                          /*!< 0x008: Host Error */
  uint32_t                  ulReserved ;                          /*!< 0x00C: Number of valid host buffers */
  NETANA_HOST_DMA_BUFFER_T  atDmaBuffers[NETANA_MAX_DMA_BUFFERS]; /*!< 0x010: Host DMA Buffers */
  volatile uint32_t         ulCaptureMode;                        /*!< 0x200: Capture Mode */
  volatile uint32_t         ulPortEnableMask;                     /*!< 0x204: Bitmask of enabled Ports */
  volatile uint32_t         ulMacMode;                            /*!< 0x208: MAC Capturing mode */
  //volatile uint64_t         ullReferenceTime;                   /*!< 0x20C: Capture reference time */
  volatile uint32_t         ulRefSeconds;                         /*!< 0x20C: Capture reference time -> seconds    */
  volatile uint32_t         ulRefNanoseconds;                     /*!< 0x210: Capture reference time -> nanseconds */
  volatile uint32_t         ulResyncP;                            /*!< 0x214: P part of clock resync PI controller */
  volatile uint32_t         ulResyncI;                            /*!< 0x218: I part of clock resync PI controller */
  volatile uint32_t         ulHostBufferTimeout;                  /*!< 0x21C: Timeout in 10 ms */
  uint32_t                  aulReserved1[0x34];                   /*!< 0x220: */
  
} __NETANA_PACKED_POST NETANA_HOST_CONTROL_BLOCK_T;

typedef __NETANA_PACKED_PRE struct NETANA_NETX_STATUS_BLOCK_Ttag
{
  volatile uint32_t ulBufferFlags;                              /*!< 0x000: Bitmask for used buffers (toggle bit with HOST CONTROL BLOCK) */
  volatile uint32_t ulCaptureState;                             /*!< 0x004: Capture State */
  volatile uint32_t ulCaptureError;                             /*!< 0x008: Capture Error */
  uint32_t          ulReserved;                             
  volatile uint32_t aulUsedBufferSize[NETANA_MAX_DMA_BUFFERS];  /*!< 0x010: Data length transferred to buffer */
  uint32_t          aulReserved1[0x1C];                         /*!< 0x090: Data length transferred to buffer */
  
} __NETANA_PACKED_POST NETANA_NETX_STATUS_BLOCK_T;

typedef __NETANA_PACKED_PRE struct NETANA_MDIO_BLOCK_Ttag
{
  volatile uint32_t ulDirection;
  volatile uint32_t ulPhyNum;
  volatile uint32_t ulPhyReg;
  volatile uint32_t ulValue;
  
} __NETANA_PACKED_POST NETANA_MDIO_BLOCK_T;

typedef __NETANA_PACKED_PRE struct NETANA_BASE_DPM_Ttag
{
  NETANA_SYSTEM_INFO_BLOCK_T  tSystemInfoBlock;   /*!< 0x000: */
  NETANA_HOST_CONTROL_BLOCK_T tHostControlBlock;  /*!< 0x100: */
  NETANA_NETX_STATUS_BLOCK_T  tNetxStatusBlock;   /*!< 0x400: */
  NETANA_MDIO_BLOCK_T         tMdioBlock;         /*!< 0x500: */
  uint32_t                    aulReserved[0x3C];  /*!< 0x510-0x600: */

} __NETANA_PACKED_POST NETANA_BASE_DPM_T;

typedef __NETANA_PACKED_PRE struct NETANA_GPIO_BLOCK_Ttag
{
  volatile uint32_t ulMode;            /*!< NETANA_GPIO_MODE_XXX              */
  volatile uint32_t ulVoltage;         /*!< NETANA_GPIO_VOLTAGE_XXX           */
  union
  {
    struct /* valid for Modes: NETANA_GPIO_MODE_RISING_EDGE and NETANA_GPIO_MODE_FALLING_EDGE */
    {
      volatile uint32_t ulCaptureTriggers; /*!< Set triggers actions (start/stop) */
      volatile uint32_t ulEndDelay;        /*!< Delay in multiple of 10 ns after capturing is stopped. */
      uint32_t          ulReserved;
    } tTrigger;

    struct /* valid for Mode: NETANA_GPIO_MODE_OUTPUT */
    {
      volatile uint32_t ulLevel;          /*!< Output level for GPIO, 0 or 1 */
      uint32_t          ulReserved1;
      uint32_t          ulReserved2;
    } tOutput;

    struct /* valid for Mode: NETANA_GPIO_MODE_OUTPUT_PWM */
    {
      volatile uint32_t ulHiPeriod;      /*!< High period of signal generator in 10 ns. Min 1 max 100.000.000 */
      volatile uint32_t ulLoPeriod;      /*!< Low period of signal generator in 10 ns. Min 1 max 100.000.000 */
      uint32_t          ulReserved;
    } tPwm;

    struct /* valid for Mode: NETANA_GPIO_MODE_SYNC*/
    {
      volatile uint32_t ulPeriod;         /*!< Period of sync input signal in ms. Min 1 max 500 */
      uint32_t          ulReserved1;
      uint32_t          ulReserved2;
    } tSync;
  } uData;
} __NETANA_PACKED_POST NETANA_GPIO_BLOCK_T;

#define NETANA_MAX_FILTERS            2

/* Base structure of the filter, followed by
   MaskA[ulFilterSize]
   ValueA[ulFilterSize]
   MaskB[ulFilterSize]
   ValueB[ulFilterSize]
*/
typedef __NETANA_PACKED_PRE struct NETANA_FILTER_BASE_Ttag
{
  uint32_t  ulRelationShip;
} __NETANA_PACKED_POST NETANA_FILTER_BASE_T;

typedef __NETANA_PACKED_PRE struct NETANA_COUNTER_BLOCK_Ttag
{
  uint32_t ulLinkState;                     /*!< Link state (Up/Down and Speed) */
  uint64_t ullFramesReceivedOk;             /*!< Received ok                */
  uint64_t ullRXErrors;                     /*!< Phy RX Errors              */
  uint64_t ullAlignmentErrors;              /*!< Alignment error            */
  uint64_t ullFrameCheckSequenceErrors;     /*!< check sequence error       */
  uint64_t ullFrameTooLongErrors;           /*!< Frame too long             */
  uint64_t ullSFDErrors;                    /*!< SFD errors                 */
  uint64_t ullShortFrame;                   /*!< Short frames               */
  uint64_t ullFramesRejected;               /*!< Rejected Frame             */
  uint64_t ullLongPreambleCnt;              /*!< Preamble to long           */
  uint64_t ullShortPreambleCnt;             /*!< Preamble to short          */
  uint64_t ullBytesLineBusy;                /*!<                            */
  uint32_t ullMinIFG;                       /*!< Min IFG                    */
  uint64_t ullTime;                         /*!< Timestamp of last counter update */
} __NETANA_PACKED_POST NETANA_COUNTER_BLOCK_T;

typedef union NETANA_HANDSHAKE_CELLtag
{
  struct __NETANA_PACKED_PRE
  {
    volatile uint8_t abData[2];        /*!< Data value, not belonging to handshake */
    volatile uint8_t bNetxFlags;       /*!< Device status flags (8Bit Mode) */
    volatile uint8_t bHostFlags;       /*!< Device command flags (8Bit Mode) */
  } __NETANA_PACKED_POST t8Bit;

  struct __NETANA_PACKED_PRE
  {
    volatile uint16_t usNetxFlags;     /*!< Device status flags (16Bit Mode) */
    volatile uint16_t usHostFlags;     /*!< Device command flags (16Bit Mode)*/
  } __NETANA_PACKED_POST t16Bit;
  volatile uint32_t ulValue;            /*!< Handshake cell value */
  
} __NETANA_PACKED_POST NETANA_HANDSHAKE_CELL;

typedef __NETANA_PACKED_PRE struct NETANA_HANDSHAKE_BLOCK_Ttag
{
  NETANA_HANDSHAKE_CELL atHandshake[16];
  uint32_t              aulReserved[0x30];
} __NETANA_PACKED_POST NETANA_HANDSHAKE_BLOCK_T;

#define IDENT_INFO_BLOCK_SIZE 30

typedef __NETANA_PACKED_PRE struct NETANA_IDENT_INFO_BLOCK_Ttag
{
  uint8_t* pbIdentInfoBlock;
} __NETANA_PACKED_POST NETANA_IDENT_INFO_BLOCK_T;

#define NETANA_CHANNEL_MAILBOX_SIZE 1596
#define DPM_SEND_MBX_OFFSET         0x2900
#define DPM_RECV_MBX_OFFSET         0x2F40
#define NETANA_MSK_PACKET_ANSWER    0x00000001 /*!< Packet answer bit */

/*****************************************************************************/
/*! Channel send packet mailbox block (Size 1600 Byte)                       */
/*****************************************************************************/
typedef __NETANA_PACKED_PRE struct NETANA_SEND_MAILBOX_BLOCKtag
{
  uint16_t  usPackagesAccepted;                                 /*!< 0x00 Number of packages that can be accepted */
  uint16_t  usReserved;                                         /*!< 0x02 Reserved */
  uint8_t   abSendMailbox[NETANA_CHANNEL_MAILBOX_SIZE];         /*!< 0x04 Send mailbox packet buffer */
} __NETANA_PACKED_POST NETANA_SEND_MAILBOX_BLOCK;

/*****************************************************************************/
/*! Channel receive packet mailbox block (Size 1600 Byte)                    */
/*****************************************************************************/
typedef __NETANA_PACKED_PRE struct NETANA_RECV_MAILBOX_BLOCKtag
{
  uint16_t  usWaitingPackages;                                  /*!< 0x00 Number of packages waiting to be processed */
  uint16_t  usReserved;                                         /*!< 0x02 Reserved */
  uint8_t   abRecvMailbox[NETANA_CHANNEL_MAILBOX_SIZE];         /*!< 0x04 Receive mailbox packet buffer */
} __NETANA_PACKED_POST NETANA_RECV_MAILBOX_BLOCK;

#ifdef __cplusplus
}
#endif

#ifdef _MSC_VER
  #if _MSC_VER >= 1000
    #pragma pack()            /* Always align structures to default boundery */
  #endif /* _MSC_VER >= 1000 */
#endif /* _MSC_VER */
  
#undef __NETANA_PACKED_PRE
#undef __NETANA_PACKED_POST

#endif /*  __NETANA_DPMLAYOUT__H */
