/* SPDX-License-Identifier: MIT */

#ifndef __NETANA_USER__H
#define __NETANA_USER__H

#ifdef __KERNEL__
  #include "OS_Includes.h"
#else
  #include <stdint.h>
#endif

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

#ifndef APIENTRY
  #define APIENTRY
#endif

/*****************************************************************************/
/*! General parameters                                                       */
/*****************************************************************************/

#define NETANA_MAX_ERRORSIZE      256 /*!< Maximum Buffer Size for returned error descriptions */
#define NETANA_MAX_DEVICENAMESIZE 64  /*!< Maximum length of device name                       */


/*****************************************************************************/
/*! General definitions                                                      */
/*****************************************************************************/

typedef void*  NETANA_HANDLE;


/*****************************************************************************/
/*! HEA file handling                                                        */
/*****************************************************************************/

#define NETANA_FILE_HEADER_COOKIE         0x4145482e
#define NETANA_FILE_HEADER_SIZE_INVALID   0xFFFFFFFFFFFFFFFF

/* hea file header */
typedef __NETANA_PACKED_PRE struct NETANA_FILE_HEADER_Ttag
{
  uint32_t  ulCookie;
  uint32_t  ulIdx;
  uint64_t  ullCaptureStart;      /*!< Reference time form start of capture */
  uint64_t  ullDataSize;
  int32_t   lTimezoneCorrection;
  uint32_t  ulReserved;

} __NETANA_PACKED_POST NETANA_FILE_HEADER_T;

/* File information structure (netana_file_info) */
typedef __NETANA_PACKED_PRE struct NETANA_FILE_INFORMATION_Ttag
{
  uint32_t  ulActualFileNr;       /*!< Actual File number being written */
  uint32_t  ulMaxFileCnt;         /*!< Maximum number of files          */

  uint64_t  ullTotalBytesWritten; /*!< Total Bytes currently written    */
  uint64_t  ullMaxBytes;          /*!< Maximum Number of bytes that can be written */

  uint32_t  ulRingBufferMode;     /*!< !=0 --> Ring buffer mode */

} __NETANA_PACKED_POST NETANA_FILE_INFORMATION_T;

/* function prototypes */
int32_t APIENTRY netana_file_info(NETANA_HANDLE hDev, uint32_t ulFileInfoSize, NETANA_FILE_INFORMATION_T* ptFileInfo);
typedef int32_t (APIENTRY *PFN_NETANA_FILE_INFO)(NETANA_HANDLE hDev, uint32_t ulFileInfoSize, NETANA_FILE_INFORMATION_T* ptFileInfo);


/*****************************************************************************/
/*! frame definitions                                                        */
/*****************************************************************************/

#define NETANA_FRAME_HEADER_ERROR_CODE_SRT  0           /*!< Frame error code (bitnumber)                     */
#define NETANA_FRAME_HEADER_ERROR_CODE_MSK  0x000000FF  /*!< Frame error code (bitmask)                       */

#define NETANA_FRAME_HEADER_GPIOEVT_SRT     8           /*!< !=0 if event is a GPIO event (bitnumber)         */
#define NETANA_FRAME_HEADER_GPIOEVT_MSK     0x00000100  /*!< !=0 if event is a GPIO event (bitmask)           */

#define NETANA_FRAME_HEADER_MACMODE_SRT     9           /*!< MAC Mode (0=Ethernet, 1=Transparent) (bitnumber) */
#define NETANA_FRAME_HEADER_MACMODE_MSK     0x00000200  /*!< MAC Mode (0=Ethernet, 1=Transparent) (bitmask)   */

#define NETANA_FRAME_HEADER_VERSION_SRT     10          /*!< HEA Version indicator (0 = old, 1 = new format(1.3)  */
#define NETANA_FRAME_HEADER_VERSION_MSK     0x00003C00

#define NETANA_FRAME_HEADER_PORT_SRT        14          /*!< Port number (bitnumber)                          */
#define NETANA_FRAME_HEADER_PORT_MSK        0x0000C000  /*!< Port number (bitmask)                            */

#define NETANA_FRAME_HEADER_LENGTH_SRT      16          /*!< Length of following data (bitnumber)             */
#define NETANA_FRAME_HEADER_LENGTH_MSK      0x0FFF0000  /*!< Length of following data (bitmask)               */

#define NETANA_FRAME_HEADER_TYPE_SRT        28
#define NETANA_FRAME_HEADER_TYPE_MSK        0xF0000000

#define NETANA_FRAME_HEADER_ERROR_CODE_MSK_RX_ERR             0x01  /*!< MII RX_ER error                 */
#define NETANA_FRAME_HEADER_ERROR_CODE_MSK_ALIGN_ERR          0x02  /*!< Alignment error                 */
#define NETANA_FRAME_HEADER_ERROR_CODE_MSK_FCS_ERROR          0x04  /*!< FCS error                       */
#define NETANA_FRAME_HEADER_ERROR_CODE_MSK_TOO_LONG           0x08  /*!< Frame too long                  */
#define NETANA_FRAME_HEADER_ERROR_CODE_MSK_SFD_ERROR          0x10  /*!< No valid SFD found              */
#define NETANA_FRAME_HEADER_ERROR_CODE_MSK_SHORT_FRAME        0x20  /*!< Frame smaller 64 bytes          */
#define NETANA_FRAME_HEADER_ERROR_CODE_MSK_SHORT_PREAMBLE     0x40  /*!< Preamble shorter than 7 bytes   */
#define NETANA_FRAME_HEADER_ERROR_CODE_MSK_LONG_PREAMBLE      0x80  /*!< Preamble longer than 7 bytes    */

#define NETANA_FRAME_HEADER_TYPE_VAL_ETHERNET             0           /*!< Packet contains Ethernet frame  */
#define NETANA_FRAME_HEADER_TYPE_VAL_PROFIBUS             1           /*!< Packet contains PROFIBUS frame  */
#define NETANA_FRAME_HEADER_TYPE_VAL_FIFO_STATE           2           /*!< Packet contains FIFO entry      */
#define NETANA_FRAME_HEADER_TYPE_VAL_TIMETICK             3           /*!< Packet contains time tick entry */

#define NETANA_FRAME_HEADER_OVERFLOW_SRC_SRT              4           /*!< source of FIFO state pseudo frame (bitnumber)  */
#define NETANA_FRAME_HEADER_OVERFLOW_SRC_MSK              0x000000f0  /*!< source of FIFO state pseudo frame (bitmask)    */

#define NETANA_FRAME_HEADER_OVERFLOW_STATE_SRT            0           /*!< state of FIFO state pseudo frame (bitnumber)  */
#define NETANA_FRAME_HEADER_OVERFLOW_STATE_MSK            0x00000001  /*!< state of FIFO state pseudo frame (bitmask)    */

#define NETANA_FRAME_HEADER_OVERFLOW_SRC_VAL_BACKEND      0           /*!< backend RX FIFO  */
#define NETANA_FRAME_HEADER_OVERFLOW_SRC_VAL_URX          1           /*!< netX URX FIFO  */
#define NETANA_FRAME_HEADER_OVERFLOW_SRC_VAL_INTRAMBUF    2           /*!< netX INTRAM buffer */
#define NETANA_FRAME_HEADER_OVERFLOW_SRC_VAL_HOSTBUF      3           /*!< host buffer */
#define NETANA_FRAME_HEADER_OVERFLOW_SRC_VAL_CAPDRV       4           /*!< capture driver (WinPCAP) */

#define NETANA_FRAME_HEADER_OVERFLOW_STATE_VAL_START      0x0         /*!< overflow state is present     */
#define NETANA_FRAME_HEADER_OVERFLOW_STATE_VAL_END        0x1         /*!< recovered from overflow state */


typedef __NETANA_PACKED_PRE struct NETANA_FRAME_HEADER_Ttag
{
  uint32_t  ulHeader;               /*!< Element information          */
  uint64_t  ullTimestamp;           /*!< Timestamp (64Bit ns counter) */

} __NETANA_PACKED_POST NETANA_FRAME_HEADER_T;


/*****************************************************************************/
/*! Driver handling                                                          */
/*****************************************************************************/

typedef __NETANA_PACKED_PRE struct NETANA_DRIVER_INFORMATION_Ttag
{
  uint32_t ulCardCnt;                   /*!< Number of cards handled by driver / toolkit */

  uint32_t ulVersionMajor;              /*!< Major version number    */
  uint32_t ulVersionMinor;              /*!< Minor version number    */
  uint32_t ulVersionBuild;              /*!< Build version number    */
  uint32_t ulVersionRevision;           /*!< Revision version number */

  uint32_t ulMaxFileCount;              /*!< Maximum number of files supported          */
  uint32_t ulDMABufferSize;             /*!< Size of a single DMA buffer                */
  uint32_t ulDMABufferCount;            /*!< Number of allocated DMA buffers per device */

  uint32_t ulToolkitVersionMajor;       /*!< Toolkit version information */
  uint32_t ulToolkitVersionMinor;
  uint32_t ulToolkitVersionBuild;
  uint32_t ulToolkitVersionRevision;

  int32_t  lMarshallerStatus;           /*!< Error code status of Marshaller-API interface  */

  uint32_t ulMarshallerVersionMajor;    /*!< Marshaller version information */
  uint32_t ulMarshallerVersionMinor;
  uint32_t ulMarshallerVersionBuild;
  uint32_t ulMarshallerVersionRevision;

} __NETANA_PACKED_POST NETANA_DRIVER_INFORMATION_T;

/* function prototypes */
int32_t APIENTRY netana_driver_information(uint32_t ulDrvInfoSize, NETANA_DRIVER_INFORMATION_T* ptDrvInfo);
typedef int32_t (APIENTRY *PFN_NETANA_DRIVER_INFORMATION)(uint32_t ulDrvInfoSize, NETANA_DRIVER_INFORMATION_T*  ptDrvInfo);


/*****************************************************************************/
/*! Device handling                                                          */
/*****************************************************************************/

/* definition of the netAnalyzer variants */
#define NETANA_GBE_VARIANT 0x00476245 /* GBE variant */

/* Extended information structure */
typedef __NETANA_PACKED_PRE struct NETANA_EXTENDED_GBE_DEV_INFO_Ttag
{
  uint8_t  abIpAddr[4];                            /* IP Address of the remote device             */
  uint8_t  abSubnetMask[4];                        /* Subnet Mask of the remote device            */
  uint8_t  abMacAddr[6];                           /* MAC Address                                 */

  uint32_t ulServerVersionMajor;                   /* Marhsaller version of the remote device     */
  uint32_t ulServerVersionMinor;                   /* | */
  uint32_t ulServerVersionBuild;                   /* | */
  uint32_t ulServerVersionRevision;                /* v */

  uint32_t ulDrvVersionMajor;                      /* Driver version of the remote device         */
  uint32_t ulDrvVersionMinor;                      /* | */
  uint32_t ulDrvVersionBuild;                      /* | */
  uint32_t ulDrvVersionRevision;                   /* v */

  uint32_t ulToolkitVersionMajor;                  /* Toolkit version information */
  uint32_t ulToolkitVersionMinor;                  /* | */
  uint32_t ulToolkitVersionBuild;                  /* | */
  uint32_t ulToolkitVersionRevision;               /* v */

  uint32_t ulOSVersionInfoSize;                    /* OS version information of the remote device */
  uint32_t ulOSMajorVersion;                       /* | */
  uint32_t ulOSMinorVersion;                       /* | */
  uint32_t ulOSBuildNumber;                        /* | */
  uint32_t ulOSPlatformId;                         /* | */
  char     szCSDVersion[128];                      /* v */

} __NETANA_PACKED_POST NETANA_EXTENDED_GBE_DEV_INFO_T;

/* extended information of the different types */
typedef __NETANA_PACKED_PRE union NETANA_EXTENDED_DEV_INFO_Ttag
{
  NETANA_EXTENDED_GBE_DEV_INFO_T tGBEExtendedInfo; /* GBE variant */

} __NETANA_PACKED_POST NETANA_EXTENDED_DEV_INFO_T;

/* basic information structure */
typedef __NETANA_PACKED_PRE struct NETANA_DEVICE_INFORMATION_Ttag
{
  char     szDeviceName[NETANA_MAX_DEVICENAMESIZE]; /*!< Name of device (needed for opening) */
  uint32_t ulPhysicalAddress;                       /*!< Physical address of DPM      */
  uint32_t ulDPMSize;                               /*!< Size of DPM                  */
  uint32_t ulUsedDPMSize;                           /*!< Used size of DPM             */
  uint32_t ulDPMLayoutVersion;                      /*!< Version of the DPM Layout    */
  uint32_t ulInterrupt;                             /*!< Interrupt vector        */

  uint32_t ulDeviceNr;                              /*!< Device number          */
  uint32_t ulSerialNr;                              /*!< Serial number          */
  uint16_t ausHwOptions[4];                         /*!< Hardware Options       */
  uint16_t usManufacturer;                          /*!< Manufacturer Code      */
  uint16_t usProductionDate;                        /*!< Production date        */
  uint16_t usDeviceClass;                           /*!< Device class           */
  uint8_t  bHwRevision;                             /*!< Hardware revision      */
  uint8_t  bHwCompatibility;                        /*!< Hardware compatibility */

  uint32_t ulOpenCnt;                               /*!< Users currently accessing this device */

  uint32_t ulPortCnt;                               /*!< Number of ports */
  uint32_t ulGpioCnt;                               /*!< Number of GPIOs */
  uint32_t ulFilterSize;                            /*!< Size of hardware filters*/
  uint32_t ulCounterSize;                           /*!< Size of counters       */

  uint8_t  szFirmwareName[32];                      /*!< Firmware name               */
  uint32_t ulVersionMajor;                          /*!< Firmware version : Major    */
  uint32_t ulVersionMinor;                          /*!< Firmware version : Minor    */
  uint32_t ulVersionBuild;                          /*!< Firmware version : Build    */
  uint32_t ulVersionRevision;                       /*!< Firmware version : Revision */

  uint32_t ulExtendedInfoSize;                     /* Size of the extended info field in bytes                */
                                                   /* = sizeof(NETANA_EXTENDED_DEV_INFO_T)+2*sizeof(uint32_t) */
  uint32_t ulExtendedInfoType;                     /* Type of extended info                       */
                                                   /* (=0x00476245 "GbE" for Gbe variant)         */
  NETANA_EXTENDED_DEV_INFO_T tExtendedInfo;        /* Extended information block                  */
                                                   /* (depends on ulExtendedInfoType)             */

} __NETANA_PACKED_POST NETANA_DEVICE_INFORMATION_T;

/* function prototypes */
int32_t APIENTRY netana_enum_device(uint32_t ulCardNr, uint32_t ulDevInfoSize, NETANA_DEVICE_INFORMATION_T* ptDevInfo);
typedef int32_t (APIENTRY *PFN_NETANA_ENUM_DEVICE)(uint32_t ulCardNr, uint32_t ulDevInfoSize, NETANA_DEVICE_INFORMATION_T* ptDevInfo);

int32_t APIENTRY netana_open_device(char* szDevice, NETANA_HANDLE* phDev);
typedef int32_t (APIENTRY *PFN_NETANA_OPEN_DEVICE)( char* szDevice, NETANA_HANDLE* phDev);

int32_t APIENTRY netana_close_device(NETANA_HANDLE hDev);
typedef int32_t (APIENTRY *PFN_NETANA_CLOSE_DEVICE)(NETANA_HANDLE hDev);

int32_t APIENTRY netana_device_info(NETANA_HANDLE hDev, uint32_t ulDevInfoSize, NETANA_DEVICE_INFORMATION_T* ptDevInfo);
typedef int32_t (APIENTRY *PFN_NETANA_DEVICE_INFO)(NETANA_HANDLE hDev, uint32_t ulDevInfoSize, NETANA_DEVICE_INFORMATION_T* ptDevInfo);


/*****************************************************************************/
/*! hardware filter handling                                                 */
/*****************************************************************************/

#define NETANA_FILTER_RELATION_FILTER_A_ENABLE      0x80000000U  /*!< Enable Filter A */
#define NETANA_FILTER_RELATION_FILTER_A_NOT         0x01000000U  /*!< Negate Filter A */

#define NETANA_FILTER_RELATION_FILTER_B_ENABLE      0x00800000U  /*!< Enable Filter B */
#define NETANA_FILTER_RELATION_FILTER_B_NOT         0x00010000U  /*!< Negate Filter B */

#define NETANA_FILTER_RELATION_ACCEPT_ERROR_FRAMES  0x00008000U  /*!< Also accept erroneous frames    */

#define NETANA_FILTER_RELATION_ACCEPT_FILTER        0x00000000U  /*!< Accept frames if filter matches */
#define NETANA_FILTER_RELATION_REJECT_FILTER        0x00000080U  /*!< Reject frames if filter matches */
#define NETANA_FILTER_RELATION_A_AND_B              0x00000000U  /*!< Result = FilterA AND Filter B   */
#define NETANA_FILTER_RELATION_A_OR_B               0x00000001U  /*!< Result = FilterA OR Filter B    */

typedef __NETANA_PACKED_PRE struct NETANA_FILTER_Ttag
{
  uint32_t ulFilterSize;  /*!< Size of the following filter */
  uint8_t* pbMask;        /*!< Filter mask (byte array)     */
  uint8_t* pbValue;       /*!< Filter value (byte array)    */

} __NETANA_PACKED_POST NETANA_FILTER_T;

/* function prototypes */
int32_t APIENTRY netana_get_filter(NETANA_HANDLE hDev, uint32_t ulPort,
                                   NETANA_FILTER_T* ptFilterA, NETANA_FILTER_T* ptFilterB,
                                   uint32_t* pulRelationShip);
typedef int32_t (APIENTRY *PFN_NETANA_GET_FILTER)(NETANA_HANDLE hDev, uint32_t ulPort,
                                                  NETANA_FILTER_T* ptFilterA, NETANA_FILTER_T* ptFilterB,
                                                  uint32_t* pulRelationShip);

int32_t APIENTRY netana_set_filter(NETANA_HANDLE hDev, uint32_t ulPort,
                                   NETANA_FILTER_T* ptFilterA, NETANA_FILTER_T* ptFilterB,
                                   uint32_t ulRelationShip);
typedef int32_t (APIENTRY *PFN_NETANA_SET_FILTER)(NETANA_HANDLE hDev, uint32_t ulPort,
                                                  NETANA_FILTER_T* ptFilterA, NETANA_FILTER_T* ptFilterB,
                                                  uint32_t ulRelationShip);


/*****************************************************************************/
/*! Device ident handling                                                    */
/*****************************************************************************/

typedef __NETANA_PACKED_PRE struct NETANA_DEVICE_IDENT_Ttag
{
  uint32_t ulIdentVersion;                          /*!< Version of this structure */
  uint32_t ulDeviceNr;                              /*!< Device number          */
  uint32_t ulSerialNr;                              /*!< Serial number          */
  uint16_t usManufacturer;                          /*!< Manufacturer Code      */
  uint32_t ulFlags1;
  uint32_t ulFlags2;
  uint16_t usNetXid;
  uint16_t usNetXflags;

} __NETANA_PACKED_POST NETANA_DEVICE_IDENT_T;

/* function prototypes */
int32_t APIENTRY netana_set_device_ident( NETANA_HANDLE hDev, char* szFileName);
typedef int32_t (APIENTRY *PFN_NETANA_SET_DEVICE_IDENT)( NETANA_HANDLE hDev, char* szFileName);

int32_t APIENTRY netana_get_device_ident( NETANA_HANDLE hDev, uint32_t ulDevIdentInfoSize, NETANA_DEVICE_IDENT_T *ptIdentInfo);
typedef int32_t (APIENTRY *PFN_NETANA_GET_DEVICE_IDENT)( NETANA_HANDLE hDev, uint32_t ulDevIdentInfoSize, NETANA_DEVICE_IDENT_T *ptIdentInfo);

int32_t APIENTRY netana_check_device_ident( NETANA_HANDLE hDev, uint32_t ulDevIdentInfoSize, uint8_t* pbDevIdentInfo);
typedef int32_t (APIENTRY *PFN_NETANA_CHECK_DEVICE_IDENT)( NETANA_HANDLE hDev, uint32_t ulDevIdentInfoSize, uint8_t* pbDevIdentInfo);


/*****************************************************************************/
/*! GPIO handling                                                            */
/*****************************************************************************/

#define NETANA_GPIO_MODE_NONE         0x00000000 /*!< don't capture GPIO   */
#define NETANA_GPIO_MODE_RISING_EDGE  0x00000001 /*!< Capture rising edge  */
#define NETANA_GPIO_MODE_FALLING_EDGE 0x00000002 /*!< Capture falling edge */
#define NETANA_GPIO_MODE_OUTPUT       0x00000003 /*!< Set output mode, level is set by netana_set_gpio_output() */
#define NETANA_GPIO_MODE_OUTPUT_PWM   0x00000004 /*!< Set output to one */
#define NETANA_GPIO_MODE_SYNC         0x00000005 /*!< Use this GPIO as sync input */

#define NETANA_GPIO_TRIGGER_NONE  0x00000000  /*!< GPIO does not trigger capture */
#define NETANA_GPIO_TRIGGER_START 0x00000001  /*!< GPIO triggers start of capture*/
#define NETANA_GPIO_TRIGGER_STOP  0x00010000  /*!< GPIO triggers stop of capture */

#define NETANA_GPIO0_VOLTAGE_MASK      0x00000001
#define NETANA_GPIO0_VOLTAGE_SRT       0x00000000
#define NETANA_GPIO1_VOLTAGE_MASK      0x00000002
#define NETANA_GPIO1_VOLTAGE_SRT       0x00000001
#define NETANA_GPIO2_VOLTAGE_MASK      0x00000004
#define NETANA_GPIO2_VOLTAGE_SRT       0x00000002
#define NETANA_GPIO3_VOLTAGE_MASK      0x00000008
#define NETANA_GPIO3_VOLTAGE_SRT       0x00000003

#define NETANA_GPIO_VOLTAGE_24V     0
#define NETANA_GPIO_VOLTAGE_3V      1

/* GPIO configuration structure */
typedef __NETANA_PACKED_PRE struct NETANA_GPIO_MODE_Ttag
{
  uint32_t ulMode;            /*!< NETANA_GPIO_MODE_XXX              */
  __NETANA_PACKED_PRE union
  {
    __NETANA_PACKED_PRE struct /* valid for Modes: NETANA_GPIO_MODE_RISING_EDGE and NETANA_GPIO_MODE_FALLING_EDGE */
    {
      uint32_t ulCaptureTriggers; /*!< Set triggers actions (start/stop) */
      uint32_t ulEndDelay;        /*!< Delay (Multiple of 10ns) , after capture is stopped by this GPIO */
    } __NETANA_PACKED_POST tTrigger;

    __NETANA_PACKED_PRE struct /* valid for Mode: NETANA_GPIO_MODE_OUTPUT_PWM */
    {
      uint32_t ulHiPeriod;       /*!< High period of signal generator in 10 ns. Min 1 max 100.000.000 */
      uint32_t ulLoPeriod;       /*!< Low period of signal generator in 10 ns. Min 1 max 100.000.000 */
    } __NETANA_PACKED_POST tPwm;

    __NETANA_PACKED_PRE struct /* valid for Mode: NETANA_GPIO_MODE_SYNC */
    {
      uint32_t ulPeriod;         /*!< Period of sync input signal in ms. Min 1 max 500 */
      uint32_t ulReserved;       /*!< reserved */
    } __NETANA_PACKED_POST tSync;
  } __NETANA_PACKED_POST uData;
} __NETANA_PACKED_POST NETANA_GPIO_MODE_T;

/* function prototypes */
int32_t APIENTRY netana_get_gpio_mode(NETANA_HANDLE hDev, uint32_t ulGpio, NETANA_GPIO_MODE_T* ptMode);
typedef int32_t (APIENTRY *PFN_NETANA_GET_GPIO_MODE)(NETANA_HANDLE hDev, uint32_t ulGpio, NETANA_GPIO_MODE_T* ptMode);

int32_t APIENTRY netana_set_gpio_mode(NETANA_HANDLE hDev, uint32_t ulGpio, NETANA_GPIO_MODE_T* ptMode);
typedef int32_t (APIENTRY *PFN_NETANA_SET_GPIO_MODE)(NETANA_HANDLE hDev, uint32_t ulGpio, NETANA_GPIO_MODE_T* ptMode);

int32_t APIENTRY netana_set_gpio_output(NETANA_HANDLE hDev, uint32_t ulGpio, uint32_t ulLevel);
typedef int32_t (APIENTRY *PFN_NETANA_SET_GPIO_OUTPUT)(NETANA_HANDLE hDev, uint32_t ulGpio, uint32_t ulLevel);

int32_t APIENTRY netana_set_gpio_voltage(NETANA_HANDLE hDev, uint32_t ulGpioSelMsk, uint32_t ulGpioVoltageSel);
typedef int32_t (APIENTRY *PFN_NETANA_SET_GPIO_VOLTAGE)(NETANA_HANDLE hDev, uint32_t ulGpioSelMsk, uint32_t ulGpioVoltageSel);

int32_t APIENTRY netana_get_gpio_voltage(NETANA_HANDLE hDev, uint32_t ulGpio, uint32_t* pulGpioVoltageSel);
typedef int32_t (APIENTRY *PFN_NETANA_GET_GPIO_VOLTAGE)(NETANA_HANDLE hDev, uint32_t ulGpio, uint32_t* pulGpioVoltageSel);


/*****************************************************************************/
/*! Port state handling                                                      */
/*****************************************************************************/

#define NETANA_LINK_STATE_UP            0x00000001U
#define NETANA_LINK_STATE_SPEED100      0x00000002U
#define NETANA_LINK_STATE_SPEED_VALID   0x00000004U

typedef __NETANA_PACKED_PRE struct NETANA_PORT_STATE_Ttag
{
  uint32_t ulLinkState;                     /*!< Link state (Up/Down and Speed) */
  uint64_t ullFramesReceivedOk;             /*!< Received ok                    */
  uint64_t ullRXErrors;                     /*!< Phy RX Errors                  */
  uint64_t ullAlignmentErrors;              /*!< Alignment error                */
  uint64_t ullFrameCheckSequenceErrors;     /*!< check sequence error           */
  uint64_t ullFrameTooLongErrors;           /*!< Frame too long                 */
  uint64_t ullSFDErrors;                    /*!< SFD errors                     */
  uint64_t ullShortFrames;                  /*!< Short frames                   */
  uint64_t ullFramesRejected;               /*!< Rejected Frame                 */
  uint64_t ullLongPreambleCnt;              /*!< Preamble to long               */
  uint64_t ullShortPreambleCnt;             /*!< Preamble to short              */
  uint64_t ullBytesLineBusy;                /*!<                                */
  uint32_t ulMinIFG;                        /*!< Min IFG                        */
  uint64_t ullTime;                         /*!< Time of last counter update    */

} __NETANA_PACKED_POST NETANA_PORT_STATE_T;

int32_t APIENTRY netana_get_portstat(NETANA_HANDLE hDev, uint32_t ulPort, uint32_t ulSize, NETANA_PORT_STATE_T* ptStatus);
typedef int32_t (APIENTRY *PFN_NETANA_GET_PORTSTAT)(NETANA_HANDLE         hDev,
                                                    uint32_t              ulPort,
                                                    uint32_t              ulSize,
                                                    NETANA_PORT_STATE_T*  ptStatus);


/*****************************************************************************/
/*! capture handling                                                         */
/*****************************************************************************/

/* modes */
#define NETANA_CAPTUREMODE_DATA       0
#define NETANA_CAPTUREMODE_TIMESTAMPS 1
#define NETANA_CAPTUREMODE_HIGHLOAD   2
#define NETANA_CAPTUREMODE_RINGBUFFER 0x80000000

#define NETANA_MACMODE_ETHERNET       0
#define NETANA_MACMODE_TRANSPARENT    1

/* function prototypes */
typedef void(APIENTRY *PFN_DATA_CALLBACK)(void* pvData, uint32_t ulDataLen, void* pvUser);
typedef void(APIENTRY *PFN_STATUS_CALLBACK)(uint32_t ulCaptureState, uint32_t ulCaptureError, void* pvUser);

int32_t APIENTRY netana_set_filelist(NETANA_HANDLE hDev, char* szPath, char* szBaseFilename, uint32_t ulFileCount, uint64_t ullFileSize);
typedef int32_t (APIENTRY *PFN_NETANA_SET_FILELIST)(NETANA_HANDLE hDev, char* szPath, char* szBaseFilename, uint32_t ulFileCount, uint64_t ullFileSize);

int32_t APIENTRY netana_start_capture(NETANA_HANDLE hDev, uint32_t ulCaptureMode, uint32_t ulPortEnableMask,
                           uint32_t ulMacMode, uint64_t ullReferenceTime,
                           PFN_STATUS_CALLBACK pfnStatus, PFN_DATA_CALLBACK pfnData,
                           void* pvUser);
typedef int32_t (APIENTRY *PFN_NETANA_START_CAPTURE)( NETANA_HANDLE       hDev,
                                                      uint32_t            ulCaptureMode,
                                                      uint32_t            ulPortEnableMask,
                                                      uint32_t            ulMacMode,
                                                      uint64_t            ullReferenceTime,
                                                      PFN_STATUS_CALLBACK pfnStatus,
                                                      PFN_DATA_CALLBACK   pfnData,
                                                      void*               pvUser);

int32_t APIENTRY netana_stop_capture(NETANA_HANDLE hDev);
typedef int32_t (APIENTRY *PFN_NETANA_STOP_CAPTURE)(NETANA_HANDLE hDev);


/*****************************************************************************/
/*! status handling                                                         */
/*****************************************************************************/

/* states */
#define NETANA_CAPTURE_STATE_OFF                                 0UL
#define NETANA_CAPTURE_STATE_START_PENDING                       1UL
#define NETANA_CAPTURE_STATE_RUNNING                             2UL
#define NETANA_CAPTURE_STATE_STOP_PENDING                        3UL

#define NETANA_CAPTURE_ERROR_STOP_TRIGGER                        0UL
#define NETANA_CAPTURE_ERROR_NO_DMACHANNEL              0xC0660004UL
#define NETANA_CAPTURE_ERROR_URX_OVERFLOW               0xC0660005UL
#define NETANA_CAPTURE_ERROR_NO_HOSTBUFFER              0xC066000BUL
#define NETANA_CAPTURE_ERROR_NO_INTRAMBUFFER            0xC066000CUL
#define NETANA_CAPTURE_ERROR_STATUS_FIFO_FULL           0xC066000DUL

#define NETANA_CAPTURE_ERROR_DRIVER_FILE_FULL           0xC0770000UL
#define NETANA_CAPTURE_ERROR_DRIVER_INVALID_BUFFERSIZE  0xC0770001UL

/* function prototypes */
int32_t APIENTRY netana_get_error_description(int32_t iError, uint32_t ulBufferSize, char* szBuffer);
typedef int32_t (APIENTRY *PFN_NETANA_GET_ERROR_DESCRIPTION)(int32_t iError, uint32_t ulBufferSize, char* szBuffer);

int32_t APIENTRY netana_get_state(NETANA_HANDLE hDev, uint32_t* pulCaptureState, uint32_t* pulCaptureError);
typedef int32_t (APIENTRY *PFN_NETANA_GET_STATE)(NETANA_HANDLE hDev, uint32_t* pulCaptureState, uint32_t* pulCaptureError);


/*****************************************************************************/
/*! PHY handling                                                             */
/*****************************************************************************/

#define NETANA_PHY_DIRECTION_READ     0
#define NETANA_PHY_DIRECTION_WRITE    1

int32_t APIENTRY netana_access_phy_reg(NETANA_HANDLE hDev, uint32_t ulDirection,
                                       uint8_t ulPhyNum, uint8_t ulPhyReg, uint16_t* pusValue,
                                       uint32_t ulTimeout);
typedef int32_t (APIENTRY *PFN_NETANA_ACCESS_PHY_REG)(NETANA_HANDLE hDev, uint32_t ulDirection,
                                                      uint8_t ulPhyNum, uint8_t ulPhyReg, uint16_t* pusValue,
                                                      uint32_t ulTimeout);


/*****************************************************************************/
/*! management handling                                                      */
/*****************************************************************************/

/* management commands ( used in netana_mngmt_exec_cmd()) */
#define NETANA_MNGMT_CMD_BLINK                       1
#define NETANA_MNGMT_CMD_DEV_SCAN                    2
#define NETANA_MNGMT_CMD_SET_DEV_CLASS_FILTER        3
#define NETANA_MNGMT_CMD_GET_DEV_FEATURE             4

/*** device scan ***/
/* structures used in (netana_mngmt_exec_cmd()) */
typedef void (APIENTRY  *PFN_SCAN_CALLBACK)(uint8_t bProgress, uint32_t ulFoundNumber, char* szLastFound, void* pvUser);

typedef __NETANA_PACKED_PRE struct NETANA_MNGMT_DEV_SCAN_IN_Ttag
{
  PFN_SCAN_CALLBACK pfnCallBack;
  void* pvUser;

} __NETANA_PACKED_POST NETANA_MNGMT_DEV_SCAN_IN_T, *PNETANA_MNGMT_DEV_SCAN_IN_T;

typedef __NETANA_PACKED_PRE struct NETANA_MNGMT_DEV_SCAN_OUT_Ttag
{
  uint32_t ulNofDevices;

} __NETANA_PACKED_POST NETANA_MNGMT_DEV_SCAN_OUT_T, *PNETANA_MNGMT_DEV_SCAN_OUT_T;


/*** device class filter ***/
#define NETANA_DEV_CLASS_NANL_500   0x00000001UL
#define NETANA_DEV_CLASS_NSCP_100   0x00000002UL
#define NETANA_DEV_CLASS_CIFX       0x00000004UL

typedef __NETANA_PACKED_PRE struct NETANA_MNGMT_SET_DEV_CLASS_FILTER_IN_Ttag
{
  uint32_t ulDeviceClass;

} __NETANA_PACKED_POST NETANA_MNGMT_SET_DEV_CLASS_FILTER_IN_T, *PNETANA_MNGMT_SET_DEV_CLASS_FILTER_IN_T;


/*** device feature ***/
#define NETANA_MNGMT_DEV_FEATURE_PHYS_TYPE_ETH    0x00000000

typedef __NETANA_PACKED_PRE struct NETANA_MNGMT_GET_DEV_FEATURE_IN_Ttag
{
  NETANA_HANDLE hDev;

} __NETANA_PACKED_POST NETANA_MNGMT_GET_DEV_FEATURE_IN_T, *PNETANA_MNGMT_GET_DEV_FEATURE_IN_T;

typedef __NETANA_PACKED_PRE struct NETANA_MNGMT_GET_DEV_FEATURE_OUT_Ttag
{
  uint32_t  ulStructVersion;
  uint32_t  ulPhysType;
  uint32_t  ulNumPhysPorts;
  uint32_t  ulPhysTapPresent;
  uint32_t  ulPhysForwardingSupport;
  uint32_t  ulPhysPortSpeedSupport;
  uint32_t  ulPhysTransparentModeSupport;
  uint32_t  ulNumGpios;
  uint32_t  ulGpioInputRisingSupport;
  uint32_t  ulGpioInputFallingSupport;
  uint32_t  ulGpioOutputModeSupport;
  uint32_t  ulGpioOutputPWMSupport;
  uint32_t  ulGpioSyncInSupport;
  uint32_t  ulGpioTriggerStartSupport;
  uint32_t  ulGpioTriggerStopSupport;
  uint32_t  ulGpioVoltage3VSupport;
  uint32_t  ulGpioVoltage24VSupport;
  uint32_t  ulSyncSupport;

} __NETANA_PACKED_POST NETANA_MNGMT_GET_DEV_FEATURE_OUT_T, *PNETANA_MNGMT_GET_DEV_FEATURE_OUT_T;

/* function prototypes */
int32_t APIENTRY netana_mngmt_exec_cmd( uint32_t      ulCommandId,
                                        void*         ptInputParameter,
                                        uint32_t      ulInputParameterSize,
                                        void*         ptOutputParameter,
                                        uint32_t      ulOutputParameterSize);

typedef int32_t (APIENTRY *PFN_NETANA_MNGMT_EXEC_CMD)(uint32_t  ulCommandId,
                                                      void*     ptInputParameter,
                                                      uint32_t  ulInputParameterSize,
                                                      void*     ptOutputParameter,
                                                      uint32_t  ulOutputParameterSize);


/*****************************************************************************/
/*! synchronization handling (preliminary)                                   */
/*****************************************************************************/

int32_t APIENTRY netana_resync_time( NETANA_HANDLE hDev, uint64_t ullReferenceTime);
typedef int32_t (APIENTRY *PFN_NETANA_RESYNC_TIME)( NETANA_HANDLE hDev, uint64_t ullReferenceTime);

int32_t APIENTRY netana_config_pi_controller( NETANA_HANDLE hDev, uint32_t ulP, uint32_t ulI);
typedef int32_t (APIENTRY *PFN_NETANA_CONFIG_PI_CONTROLLER)( NETANA_HANDLE hDev, uint32_t ulP, uint32_t ulI);


/*****************************************************************************/
/*! Packet communication                                                     */
/*****************************************************************************/
#define NETANA_MAX_PACKET_SIZE               1596                                                /*!< Maximum size of the packet in bytes        */
#define NETANA_PACKET_HEADER_SIZE            40                                                  /*!< Maximum size of the packet header in bytes */
#define NETANA_MAX_DATA_SIZE                (NETANA_MAX_PACKET_SIZE - NETANA_PACKET_HEADER_SIZE) /*!< Maximum packet data size                   */

/*****************************************************************************/
/*! Packet header                                                            */
/*****************************************************************************/
typedef __NETANA_PACKED_PRE struct NETANA_PACKET_HEADERtag
{
  uint32_t   ulDest;   /*!< destination of packet, process queue     */
  uint32_t   ulSrc;    /*!< source of packet, process queue          */
  uint32_t   ulDestId; /*!< destination reference of packet          */
  uint32_t   ulSrcId;  /*!< source reference of packet               */
  uint32_t   ulLen;    /*!< length of packet data without header     */
  uint32_t   ulId;     /*!< identification handle of sender          */
  uint32_t   ulState;  /*!< status code of operation                 */
  uint32_t   ulCmd;    /*!< packet command defined in TLR_Commands.h */
  uint32_t   ulExt;    /*!< extension                                */
  uint32_t   ulRout;   /*!< router                                   */
} __NETANA_PACKED_POST NETANA_PACKET_HEADER;

/*****************************************************************************/
/*! Definition of the netana Packet                                          */
/*****************************************************************************/
typedef __NETANA_PACKED_PRE struct NETANA_PACKETtag
{
  NETANA_PACKET_HEADER  tHeader;
  uint8_t               abData[NETANA_MAX_DATA_SIZE];
} __NETANA_PACKED_POST NETANA_PACKET;

typedef void(APIENTRY *PFN_RECV_PKT_CALLBACK)(NETANA_PACKET* ptRecvPkt, void* pvUser);

int32_t APIENTRY netana_get_packet( NETANA_HANDLE hDev, uint32_t ulSize, NETANA_PACKET* ptRecvPkt, uint32_t ulTimeout);
int32_t APIENTRY netana_put_packet( NETANA_HANDLE hDev, NETANA_PACKET*  ptSendPkt, uint32_t ulTimeout);
int32_t APIENTRY netana_get_mbx_state( NETANA_HANDLE hDev, uint32_t* pulRecvPktCount, uint32_t* pulSendPktCount);

#ifdef _MSC_VER
  #if _MSC_VER >= 1000
    #pragma pack()            /* Always align structures to default boundary */
  #endif /* _MSC_VER >= 1000 */
#endif /* _MSC_VER */

#undef __NETANA_PACKED_PRE
#undef __NETANA_PACKED_POST

#ifdef __cplusplus
  }
#endif  /* _cplusplus */

#endif /*  __NETANA_USER__H */
