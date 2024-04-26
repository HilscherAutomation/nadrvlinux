/**************************************************************************************

   Copyright (c) Hilscher GmbH. All Rights Reserved.

 **************************************************************************************

   Filename:
    $Workfile: libnetana.h $
   Last Modification:
    $Author: sebastiand $
    $Modtime: 30.09.09 14:10 $
    $Revision: 3529 $

   Targets:
     Linux        : yes

   Description:


   Changes:

     Version   Date        Author   Description
     ----------------------------------------------------------------------------------
     1        17.09.2012   SD       - Initial version

**************************************************************************************/
#ifndef __LIBNETANA__H
#define __LIBNETANA__H

#include <stdint.h>

#ifdef __cplusplus
  extern "C" {
#endif  /* _cplusplus */

typedef struct NETANA_THREAD_PARAM_Ttag
{
        int fValid;
        int iThreadPolicy;
        int iThreadPriority;
} NETANA_THREAD_PARAM_T;

typedef struct NETANA_LIB_INIT_PARAM_Ttag
{
        uint32_t              ulTracelevel;
        char*                 szBaseDirectory;
        NETANA_THREAD_PARAM_T tUSDataThread;
        NETANA_THREAD_PARAM_T tKSDataThread;

} NETANA_LIB_INIT_PARAM_T;

void    netana_lib_init( uint32_t ulSize, NETANA_LIB_INIT_PARAM_T* ptInitParam);

void    netana_set_tracelevel         ( uint32_t ulTracelevel, char *szBaseDirectory);
void    netana_set_capture_thread_prio( int iPolicy, int iPriority);

int32_t netana_signal_drv_error( void* hDev, uint32_t ulError);

#ifdef __cplusplus
  }
#endif  /* _cplusplus */

#endif //__LIBNETANA__H