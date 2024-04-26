/**************************************************************************************
 
   Copyright (c) Hilscher GmbH. All Rights Reserved.
 
 **************************************************************************************
 
   Filename:
    $Id: netx50_romloader_dpm.h 5203 2013-04-24 07:57:34Z MichaelT $
   Last Modification:
    $Author: MichaelT $
    $Date: 2013-04-24 09:57:34 +0200 (Wed, 24 Apr 2013) $
    $Revision: 5203 $
   
   Targets:
     Win32/ANSI   : yes
     Win32/Unicode: yes (define _UNICODE)
     WinCE        : no
 
   Description:
    netX50 ROM Loader DPM layout
       
   Changes:
 
     Version   Date        Author   Description
     ----------------------------------------------------------------------------------
     1         22.01.2009  MT       initial version
 
**************************************************************************************/

#ifndef __NETX50_ROMLOADER_DPM__H
#define __NETX50_ROMLOADER_DPM__H

#define NETX50_BOOTID_DPM             0x4C42584E  /*!< 'NXBL' DPM boot identifier ('NXBL') */
#define NETX50_BOOTID_DPM_STRING      "NXBL"      /*!< 'NXBL' DPM boot identifier ('NXBL') */

#define NETX50_DPM_TONETXMBX_MSK      0x01
#define NETX50_DPM_TOHOSTMBX_MSK      0x02

#define NETX50_DPM_HANDSHAKE_OFFSET   63          /*!< Offset of the command handshake flags            */

#define NETX50_DPM_BLLAYOUT_OFFSET    118         /*!< Offset of the bootloader DPM layout in aulDpmHsRegs */
#define MSK_NETX50_DPM_BLLAYOUT       0xFF00
#define SRT_NETX50_DPM_BLLAYOUT       8

typedef union NETX50_BL_HSREGISTERtag
{
  struct
  {
    unsigned short           usReserved; 
    volatile unsigned char   bNetXFlags; /*!< Flags signalled by netX50 */
    volatile unsigned char   bHostFlags; /*!< Flags signalled by Host   */
  } t8Bit;
  unsigned long ulVal;
  
  
} NETX50_BL_HSREGISTER;

typedef struct NETX50_ROMLOADER_DPMtag
{
  volatile unsigned long  ulDpmBootId;
  volatile unsigned long  ulDpmByteSize;
  volatile unsigned long  ulSdramGeneralCtrl;
  volatile unsigned long  ulSdramTimingCtrl;
  volatile unsigned long  ulSdramByteSize;
  volatile unsigned long  aulReserved14[249];
  volatile unsigned long  ulHostToNetxDataSize;
  volatile unsigned long  ulNetxToHostDataSize;
  volatile unsigned char  abHostToNetxData[4096];
  volatile unsigned char  abNetxToHostData[2048];
  NETX50_BL_HSREGISTER    atHandshakeRegs[128];
  volatile unsigned long  aulDpmHsRegs[128];
  
} NETX50_ROMLOADER_DPM, *PNETX50_ROMLOADER_DPM;

#endif /* __NETX50_ROMLOADER_DPM__H */
