/****************************************************************************************
   
    Copyright (c) Hilscher GmbH. All Rights Reserved.
  
*****************************************************************************************
  
    Filename:
     $Id: netx_romloader.h 5432 2013-07-01 07:20:14Z sebastiand $
    Last Modification:
     $Author: sebastiand $
     $Date: 2013-07-01 09:20:14 +0200 (Mon, 01 Jul 2013) $
     $Revision: 5432 $
    
    Targets:
      Win32/ANSI   : yes
      Win32/Unicode: yes
      WinCE        : yes
  
    Description:
    
      NetX_Romloader.h : netX romloader bootblock definitions

    Changes:
  
      Revision  Date        Author      Description
      -----------------------------------------------------------------------------------
      2         06.12.06    RM          - obsoleted #pragma once removed
      1         06.03.06    MT          created

****************************************************************************************/

/*****************************************************************************/
/*! \file
*     netX romloader bootblock definitions                                   */
/*****************************************************************************/

/*****************************************************************************/
/*! \addtogroup NETANA_TK_DPMSTRUCTURE DPM Structure Definition
*   \{                                                                       */
/*****************************************************************************/

#ifndef __NETX_ROMLOADER_H
#define __NETX_ROMLOADER_H

#define MSK_SYSSTA_LED_READY     0x00000001UL /*!< Bitmask for the system state READY LED */
#define MSK_SYSSTA_LED_RUN       0x00000002UL /*!< Bitmask for the system state RUN LED */

#define MSK_SYSSTA_BOOT_ACTIVE   0x00000008UL /*!< Bitmask for bootloader is active */
#define MSK_SYSSTA_BOOT_START    0x00000080UL /*!< Bitmask to toggle/xor when requesting bootloader to start image */

#define BOOTBLOCK_COOKIE_PCI     0xF8BEAF00UL /*!< Bootblock cookie used for PCI mode     */
#define BOOTBLOCK_COOKIE_8BIT    0xF8BEAF08UL /*!< Bootblock cookie used for 8Bit Memory  */
#define BOOTBLOCK_COOKIE_16BIT   0xF8BEAF16UL /*!< Bootblock cookie used for 16Bit Memory */
#define BOOTBLOCK_COOKIE_32BIT   0xF8BEAF32UL /*!< Bootblock cookie used for 32Bit Memory */

#define BOOTBLOCK_FILE_SIGNATURE 0x5854454EUL /*!< Bootblock signature ('NETX')           */

#pragma pack(4)

/*****************************************************************************/
/*! Bootblock expected by netX Romloader                                     */
/*****************************************************************************/
typedef struct NETX_BOOTBLOCK_Ttag 
{
  unsigned long ulCookie;           /*!< Cookie identifying bus width and valid bootblock               */
  
  union 
  {
    unsigned long ulMemCtrl;        /*!< Parallel/Serial Flash Mode for setting up timing parameters    */
    unsigned long ulSpeed;          /*!< I2C/SPI Mode for identifying speed of device                   */
    unsigned long ulReserved;       /*!< PCI/DPM mode                                                   */
  } unCtrl;

  unsigned long ulApplEntrypoint;   /*!< Entrypoint to application after relocation                     */
  unsigned long ulApplChecksum;     /*!< Checksum of application                                        */
  unsigned long ulApplSize;         /*!< size of application in DWORDs                                  */
  unsigned long ulApplStartAddr;    /*!< Relocation address of application                              */
  unsigned long ulSignature;        /*!< Bootblock signature ('NETX')                                   */
  
  union {
    unsigned long ulSdramGeneralCtrl;      /*!< SDRam General control value                             */
    unsigned long ulExpBusReg;             /*!< Expension bus register value (EXPBus Bootmode)          */
  } unCtrl0;
  
  union {
    unsigned long ulSdramTimingCtrl;       /*!< SDRam Timing control register value                     */
    unsigned long ulIoRegMode0;            /*!< IORegmode0 register value (EXPBus Bootmode)             */
  } unCtrl1;
  
  union {
    unsigned long ulIoRegMode1;            /*!< IORegmode1 register value (EXPBus Bootmode)             */
    unsigned long ulRes0;                  /*!< unused/reserved                                         */
  } unCtrl2;
  
  union {
    unsigned long ulIfConf1;               /*!< IfConfig1 register value (EXPBus Bootmode)              */
    unsigned long ulRes0;                  /*!< unused/reserved                                         */
  } unCtrl3;
  
  union {
    unsigned long ulIfConf2;               /*!< IfConfig2 register value (EXPBus Bootmode)              */
    unsigned long ulRes0;                  /*!< unused/reserved                                         */
  } unCtrl4;
  
  unsigned long ulMiscAsicCtrl;            /*!< ASIC CTRL register value                                */
  unsigned long ulRes[2];                  /*!< unused/reserved                                         */
  unsigned long ulBootChecksum;            /*!< Bootblock checksum                                      */
} NETX_BOOTBLOCK_T, *PNETX_BOOTBLOCK_T;

#pragma pack()

/*****************************************************************************/
/*! \}                                                                       */
/*****************************************************************************/

#endif
