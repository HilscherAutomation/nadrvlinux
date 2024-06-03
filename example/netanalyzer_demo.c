/* SPDX-License-Identifier: MIT */
#include <netana_user.h>
#include <netana_errors.h>

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include <termios.h>

/* Sleep (Windows) helper from https://stackoverflow.com/questions/1157209/is-there-an-alternative-sleep-function-in-c-to-milliseconds */
void Sleep(uint32_t milliseconds) {
#if _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
#else
    usleep(milliseconds * 1000);
#endif
}

/* kbhit (Windows) helper from http://www.undertec.de/blog/2009/05/kbhit-und-getch-fur-linux.html */
static int kbhit(void) {
  struct termios term, oterm;
  int fd = 0;
  int c = 0;

  tcgetattr(fd, &oterm);
  memcpy(&term, &oterm, sizeof(term));

  term.c_lflag = term.c_lflag & (!ICANON);
  term.c_cc[VMIN] = 0;
  term.c_cc[VTIME] = 1;

  tcsetattr(fd, TCSANOW, &term);
  c = getchar();
  tcsetattr(fd, TCSANOW, &oterm);

  if (c != -1)
    ungetc(c, stdin);

  return ((c != -1) ? 1 : 0);
}

/*=========================================================================
* Configure GPIOs on a netANALYZER Card
*==========================================================================*/
void ConfigureGPIOs(NETANA_HANDLE hDevice)
{
  int32_t lResult;

  NETANA_GPIO_MODE_T tGpio = {0};

  /* Setup GPIO 0 to be captured on rising edge */
  tGpio.uData.tTrigger.ulCaptureTriggers = NETANA_GPIO_TRIGGER_NONE;
  tGpio.ulMode                           = NETANA_GPIO_MODE_RISING_EDGE;

  if(NETANA_NO_ERROR != (lResult = netana_set_gpio_mode(hDevice, 0, &tGpio)))
  {
    printf("\nError configuring GPIO 0. ErrorCode=0x%08X\r\n", (uint32_t)lResult);
  } else
  {
    printf("\nConfigured GPIO 0 for rising edge detection\r\n");
  }

  /* Setup GPIO 1 to stop capturing (after 2 us) if falling edge occurs */
  tGpio.uData.tTrigger.ulCaptureTriggers = NETANA_GPIO_TRIGGER_STOP;
  tGpio.ulMode                           = NETANA_GPIO_MODE_FALLING_EDGE;
  tGpio.uData.tTrigger.ulEndDelay        = 200;

  if(NETANA_NO_ERROR != (lResult = netana_set_gpio_mode(hDevice, 1, &tGpio)))
  {
    printf("Error configuring GPIO 1. ErrorCode=0x%08X\r\n", (uint32_t)lResult);
  } else
  {
    printf("Configured GPIO 1 to stop capturing after 2 us when a falling edge is detected\r\n");
  }
}

/*==========================================================================
* Configure Filters on a netANALYZER Card
*===========================================================================*/
void ConfigureFilters(NETANA_HANDLE hDevice)
{
  NETANA_FILTER_T tFilterA = {0};
  uint8_t         abFilterMaskA[16]  = {0};
  uint8_t         abFilterValueA[16] = {0};

  NETANA_FILTER_T tFilterB = {0};
  uint8_t         abFilterMaskB[16]  = {0};
  uint8_t         abFilterValueB[16] = {0};

  uint32_t        ulRelation;

  int32_t         lResult;

  /* Filter A matches TCP/IP Frames (Frametype = 0x0800) */
  abFilterMaskA[12]  = 0xFF;
  abFilterMaskA[13]  = 0xFF;
  abFilterValueA[12] = 0x08;
  abFilterValueA[13] = 0x00;
  tFilterA.ulFilterSize = sizeof(abFilterMaskA);
  tFilterA.pbMask  = abFilterMaskA;
  tFilterA.pbValue = abFilterValueA;

  /* Filter B matches ARP Frames (Frametype = 0x0806) */
  abFilterMaskB[12]  = 0xFF;
  abFilterMaskB[13]  = 0xFF;
  abFilterValueB[12] = 0x08;
  abFilterValueB[13] = 0x06;
  tFilterB.ulFilterSize = sizeof(abFilterMaskB);
  tFilterB.pbMask  = abFilterMaskB;
  tFilterB.pbValue = abFilterValueB;

  ulRelation =  NETANA_FILTER_RELATION_FILTER_A_ENABLE |
                NETANA_FILTER_RELATION_FILTER_B_ENABLE |
                NETANA_FILTER_RELATION_A_OR_B          |
                NETANA_FILTER_RELATION_ACCEPT_FILTER;

  /* Setup filters on Port 0 to only capture TCP/IP Frames 
    (Frametype = 0x0800) and ARP (Type 0x0806) frames */
  if(NETANA_NO_ERROR != (lResult = netana_set_filter(hDevice, 
                                                     0,
                                                     &tFilterA,
                                                     &tFilterB,
                                                     ulRelation)))
  {
    printf("\nError setting filters on port 0. ErrorCode=0x%08X\r\n", (uint32_t)lResult);

  } else
  {
    printf("\nSuccessfully set filters on Port 0\r\n");
  }
}

static bool s_fCaptureStopPending = false;

/*===========================================================================
* Status change callback. Called by the driver if a function pointer was
* passed in netana_start_capture and the state of the card has changed
*============================================================================*/
static void APIENTRY StatusCallback(uint32_t ulCaptureState, uint32_t ulCaptureError, void* pvUser)
{
  switch(ulCaptureState)
  {
  case NETANA_CAPTURE_STATE_OFF:
    printf("\n-> Capture Stopped. ErrorCode=0x%08X (Status-Callback)\r\n", ulCaptureError);
    break;

  case NETANA_CAPTURE_STATE_START_PENDING:
    printf("\n-> Preparing Capture Start (Status-Callback).\r\n");
    break;

  case NETANA_CAPTURE_STATE_RUNNING:
    printf("\n-> Capture Running (Status-Callback).\r\n");
    break;

  case NETANA_CAPTURE_STATE_STOP_PENDING:
    printf("\n-> Capture Stop Pending. ErrorCode=0x%08X (Status-Callback)\r\n", ulCaptureError);
    s_fCaptureStopPending = true;
    break;

  default:
    printf("\n-> Unknown Capture State (%u). ErrorCode=0x%08X (Status-Callback)\r\n", ulCaptureState, ulCaptureError);
    break;
  }
}

/*============================================================================
* New data indication callback. Called by the driver when new capture data has
* arrived.
*=============================================================================*/
static void APIENTRY DataCallback(void* pvBuffer, uint32_t ulDataSize, void* pvUser)
{
  uint8_t*  pbBuffer   = (uint8_t*)pvBuffer;
  uint32_t  ulOffset   = 0;
  uint32_t  ulFrameCnt = 0;

  while(ulOffset < ulDataSize)
  {
    NETANA_FRAME_HEADER_T* ptFrame  = (NETANA_FRAME_HEADER_T*)(pbBuffer + ulOffset);
    uint32_t ulFrameLen = (ptFrame->ulHeader & NETANA_FRAME_HEADER_LENGTH_MSK) >> NETANA_FRAME_HEADER_LENGTH_SRT;

    ulFrameCnt++;

    /* Adjust Offset to next DWORD aligned address */
    ulOffset += sizeof(*ptFrame) + ulFrameLen;
    while(ulOffset % 4)
      ++ulOffset;
  }

  printf("Received %u frames: DataLen=%u\r\n", ulFrameCnt, ulDataSize);
}

/*============================================================================
* Start a capture
*=============================================================================*/
void DoCapture(NETANA_HANDLE hDevice)
{
  int32_t  lResult;
  /* Reference time for wireshark needs to be UNIX Timestamp (seconds sind 1.1.1970)
     and as we are using a nanosecond timestamp, we need to multiply it with 1000000000 */
  uint64_t ullReferenceTime = time(NULL) * 1000 * 1000 * 1000;

  /* NOTE: If you want to capture to a file, you will need to call netana_set_filelist before
           starting the capture.
           e.g.
           netana_set_filelist(hDevice, "C:\\", "Capture", 1, 1 * 1024 * 1024 * 1024);
  */
  if(NETANA_NO_ERROR != (lResult = netana_start_capture( hDevice,
                                                         0,
                                                         0xF,
                                                         NETANA_MACMODE_ETHERNET,
                                                         ullReferenceTime,
                                                         StatusCallback,
                                                         DataCallback,
                                                         NULL)))
  {
    printf("Error starting capture. ErrorCode=0x%08X\r\n", (uint32_t)lResult);

  } else
  {
    printf("\n!!!Press any key to stop capturing!!!\r\n");

    while(!kbhit())
    {
      if(s_fCaptureStopPending)
      {
        printf("netANALYZER Firmware detected an error and stopped capturing!\r\n");
        printf("Aborting capture!\r\n");

        break;
      }
      Sleep(10);
    }

    /* NOTE: We need to call netana_stop_capture even if the firmware stopped automatically,
             to tell the firmware we've understood that capturing was automatically stopped */
    netana_stop_capture(hDevice);
    printf("\nStopped Capturing...\r\n");
  }
}

/*=============================================================================
* Main
*==============================================================================*/
int main(void)
{
  int32_t                     lResult;
  NETANA_DRIVER_INFORMATION_T tDriverInfo   = {0};
  char*                       szDeviceToUse = NULL;
  uint32_t                    ulFilter      = (NETANA_DEV_CLASS_NANL_500 | NETANA_DEV_CLASS_NSCP_100| NETANA_DEV_CLASS_CIFX);

  printf("******************** netAnalyzer Demo Application ********************\n\n");

  printf("Gathering driver information...\n");
  printf("----------------------------------------\r\n");

  netana_mngmt_exec_cmd(NETANA_MNGMT_CMD_SET_DEV_CLASS_FILTER,
                        &ulFilter, sizeof(ulFilter),
                        NULL, 0);

  /* Try to open the driver */                                                   
  if(NETANA_NO_ERROR != (lResult = netana_driver_information(sizeof(tDriverInfo), &tDriverInfo)))
  {
    printf("Error opening driver. ErrorCode=0x%08X\r\n", (uint32_t)lResult);
  } else
  {
    printf("Driver Version\t: %u.%u.%u.%u\r\n\n", tDriverInfo.ulVersionMajor, 
                                                  tDriverInfo.ulVersionMinor, 
                                                  tDriverInfo.ulVersionBuild, 
                                                  tDriverInfo.ulVersionRevision);

    printf("Toolkit Version\t: %u.%u.%u.%u\r\n\n", tDriverInfo.ulToolkitVersionMajor, 
                                                   tDriverInfo.ulToolkitVersionMinor, 
                                                   tDriverInfo.ulToolkitVersionBuild, 
                                                   tDriverInfo.ulToolkitVersionRevision);

    lResult = netana_driver_information(sizeof(tDriverInfo), &tDriverInfo);

    if (tDriverInfo.ulCardCnt)
    {
      printf("Gathering device information...\n");
      printf("----------------------------------------\r\n");

      printf(" Cards       : %u\r\n", tDriverInfo.ulCardCnt);
      printf(" DMA Buffers : %u x %u Bytes\r\n", tDriverInfo.ulDMABufferCount, tDriverInfo.ulDMABufferSize);
      printf(" Max Files   : %u\r\n", tDriverInfo.ulMaxFileCount);

      printf(" Found cards : %d\r\n", tDriverInfo.ulCardCnt);

      /* Enumerate all available boards and use the first found one, to do our tests */
      for(uint32_t ulCard = 0; ulCard < tDriverInfo.ulCardCnt; ulCard++)
      {
        NETANA_DEVICE_INFORMATION_T tDevInfo = {0};

        if(NETANA_NO_ERROR != (lResult = netana_enum_device(ulCard, sizeof(tDevInfo), &tDevInfo)))
        {
          printf("\n[%u]: Error enumerating card #%u. ErrorCode=0x%08X\r\n", ulCard, ulCard, (uint32_t)lResult);

        } else
        {
          if(NULL == szDeviceToUse)
          {
            /* NOTE: We will always use the first available device for our tests */
            szDeviceToUse = strdup((char*)tDevInfo.szDeviceName);
          }

          printf("\n[%u]:\tDeviceName = '%s'\r\n", ulCard, (char*)tDevInfo.szDeviceName);
          printf("\tDeviceNr   = %u\n\tSerialNr   = %u\r\n", tDevInfo.ulDeviceNr, tDevInfo.ulSerialNr);
          printf("\tFirmware   = %s V%u.%u.%u.%u\r\n",
                 (char*)tDevInfo.szFirmwareName,
                 tDevInfo.ulVersionMajor, 
                 tDevInfo.ulVersionMinor, 
                 tDevInfo.ulVersionBuild, 
                 tDevInfo.ulVersionRevision); 
          printf("\tPorts      = %u\n\tGPIOs      = %u\n\tFilterSize = %u\r\n",
                 tDevInfo.ulPortCnt, 
                 tDevInfo.ulGpioCnt, 
                 tDevInfo.ulFilterSize);

        }
      }
    }

    if(NULL == szDeviceToUse)
    {
      printf("\nNo device found for further testing\r\n");

    } else
    {
      NETANA_HANDLE hDevice = NULL;

      printf("\nStarting tests on Device '%s'\r\n", szDeviceToUse);
      printf("----------------------------------------\r\n");

      /* Get a handle to the device */
      if(NETANA_NO_ERROR != (lResult = netana_open_device(szDeviceToUse, &hDevice)))
      {
        printf("Error opening device '%s'. ErrorCode=0x%08X\r\n", szDeviceToUse, (uint32_t)lResult);

      } else
      {
    /* configure GPIOs for capture detection */
        ConfigureGPIOs(hDevice);
  
        /* configure filter (frame filtering) */
        ConfigureFilters(hDevice);

        /* start capturing */
        DoCapture(hDevice);

        
        printf("\nTest ended!\r\n");
        printf("----------------------------------------\r\n");
        netana_close_device(hDevice);
      }
    }
  }

  getchar();
 
  return 0;
}
