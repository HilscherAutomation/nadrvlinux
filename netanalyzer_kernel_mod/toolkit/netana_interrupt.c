/**************************************************************************************

   Copyright (c) Hilscher GmbH. All Rights Reserved.

 **************************************************************************************

   Filename:
    $Id: netana_interrupt.c 6020 2013-11-25 11:40:59Z sebastiand $
   Last Modification:
    $Author: sebastiand $
    $Date: 2013-11-25 12:40:59 +0100 (Mon, 25 Nov 2013) $
    $Revision: 6020 $

   Targets:
     Win32/ANSI   : yes

   Description:
    netAnalyzer specific interrupt handling

   Changes:

     Version   Date        Author   Description
     ----------------------------------------------------------------------------------
     4        22.11.2013   SD       removed unecessary irq handling for irq1
     3        31.10.2013   SD       synchronized access to shared buffer of ISR/DSR
     2        05.02.2013   SD       add file header
     1        -/-          MT       initial version

**************************************************************************************/

#include "netana_toolkit.h"

int32_t netana_tkit_isr_handler(struct NETANA_DEVINSTANCE_T* ptDevInstance, int fCheckOtherDevice)
{
  int                       iRet;
  uint32_t                  ulIrqState0;
  int                       iIrq2DsrBuffer   = ptDevInstance->iIrq2DsrBuffer;
  NETANA_IRQ2DSR_BUFFER_T*  ptIsr2DsrBuffer  = &ptDevInstance->atIrq2DsrBuffer[iIrq2DsrBuffer];

  ulIrqState0 = LE32_TO_HOST(ptDevInstance->ptGlobalRegisters->ulIRQState_0);

  /* First check if we have generated this interrupt by reading the global IRQ status bit */
  if(  fCheckOtherDevice &&
      (0 == (ulIrqState0 & MSK_IRQ_STA0_INT_REQ)) )
  {
    /* we have not generated this interrupt, so it must be another device on shared IRQ */
    iRet = NETANA_TKIT_ISR_IRQ_OTHERDEVICE;

  } else
  {
    unsigned long ulCell;

    ptIsr2DsrBuffer->fValid = 1;
    ++ptDevInstance->ulIrqCounter;

    /* Only read first 8 Handshake cells, due to a netX hardware issue. Reading flags 8-15 may
       also confirm IRQs for Handshake cell 0-7 due to an netX internal readahead buffer */
    for(ulCell = 0; ulCell < 8; ++ulCell)
    {
      NETANA_HANDSHAKE_BLOCK_T* ptHskBlock = ptDevInstance->ptHandshakeBlock;
      ptIsr2DsrBuffer->atHskCell[ulCell].ulValue = ptHskBlock->atHandshake[ulCell].ulValue;
    }

    /* we need to check in DSR which handshake bits have changed */
    iRet = NETANA_TKIT_ISR_IRQ_DSR_REQUESTED;
  }

  return iRet;
}

int32_t netana_tkit_dsr_handler(struct NETANA_DEVINSTANCE_T* ptDevInstance)
{
  /* Get actual data buffer index */
  int                       iIrq2DsrBuffer  = 0;
  NETANA_IRQ2DSR_BUFFER_T*  ptIrq2DsrBuffer = NULL;
  int32_t                   lRet            = NETANA_TKIT_DSR_HANDLED;
  uint32_t                  ulChangedBits;
  uint32_t                  ulBitPos;

  /* ATTENTION: The IrqToDsr Buffer handling implies a "always" higher priority */
  /*            of the ISR function. This does usually happens on physical ISR functions */
  /*            but does not work if the ISR and DSR are handled as a threads! */
#ifdef TOOLKIT_ENABLE_DSR_LOCK
  OS_IrqLock( ptDevInstance->pvOSDependent);
#endif
  /* retrieve shared buffer */
  iIrq2DsrBuffer  = ptDevInstance->iIrq2DsrBuffer;
  ptIrq2DsrBuffer = &ptDevInstance->atIrq2DsrBuffer[iIrq2DsrBuffer];

  if(!ptIrq2DsrBuffer->fValid)
  {
#ifdef TOOLKIT_ENABLE_DSR_LOCK
    OS_IrqUnlock( ptDevInstance->pvOSDependent);
#endif
    /* Interrupt did not provide data yet */
    return NETANA_NO_ERROR;
  } else
  {
    /* Flip data buffer so IRQ uses the other buffer */
    ptDevInstance->iIrq2DsrBuffer ^= 0x01;

    /* Invalidate the buffer, we are now handling */
    ptIrq2DsrBuffer->fValid        = 0;
  }
#ifdef TOOLKIT_ENABLE_DSR_LOCK
  OS_IrqUnlock( ptDevInstance->pvOSDependent);
#endif

  /* NOTE: netANALYZER currently only used Handshake cell 0, so we will skip
           the other cells */

  ulChangedBits = ptDevInstance->ulNetxFlags ^ ptIrq2DsrBuffer->atHskCell[0].t16Bit.usNetxFlags;
  ptDevInstance->ulNetxFlags = ptIrq2DsrBuffer->atHskCell[0].t16Bit.usNetxFlags;

  /* Signal all waiting threads that the netX bits have changed */
  for(ulBitPos = 0; ulBitPos < 16; ++ulBitPos)
  {
    uint32_t ulBitMask = (1 << ulBitPos);

    if(ulChangedBits & ulBitMask)
    {
      /* Evaluate which handshake bits, need further processing */
      switch(ulBitPos)
      {
      case NETANA_HSK_HOST_NEWDATA_IND:
      case NETANA_HSK_HOST_NEWSTATUS_IND:
        lRet = NETANA_TKIT_DSR_PROCESSING_REQUEST;
        break;
      }

      OS_SetEvent(ptDevInstance->ahHandshakeBitEvents[ulBitPos]);
    }
  }

  return lRet;
}
