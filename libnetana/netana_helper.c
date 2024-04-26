/**************************************************************************************

Copyright (c) Hilscher GmbH. All Rights Reserved.

**************************************************************************************

Filename:
$Workfile: netana_helper.c $
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
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>

#include "netana_private.h"
#include "libnetana.h"

NETANA_THREAD_PRIO_T g_tKThreadSetting = {0};
NETANA_THREAD_PRIO_T g_tThreadSetting  = {0};

uint32_t g_ulTraceLevel       = 0;
char*    g_szDriverBaseDir    = NULL;

void netana_lib_init( uint32_t ulSize, NETANA_LIB_INIT_PARAM_T* ptInitParam)
{
        netana_set_tracelevel( ptInitParam->ulTracelevel, ptInitParam->szBaseDirectory);

        /* update priority settings of user space thread */
        g_tThreadSetting.fThreadPrioSettings = ptInitParam->tUSDataThread.fValid;
        g_tThreadSetting.iPolicy             = ptInitParam->tUSDataThread.iThreadPolicy;
        g_tThreadSetting.iPriority           = ptInitParam->tUSDataThread.iThreadPriority;
        /* update priority settings of kernel space thread */
        g_tKThreadSetting.fThreadPrioSettings = ptInitParam->tKSDataThread.fValid;
        g_tKThreadSetting.iPolicy             = ptInitParam->tKSDataThread.iThreadPolicy;
        g_tKThreadSetting.iPriority           = ptInitParam->tKSDataThread.iThreadPriority;
}

void netana_set_tracelevel( uint32_t ulTracelevel, char *szBaseDirectory)
{
        g_ulTraceLevel    = ulTracelevel;
        g_szDriverBaseDir = szBaseDirectory;
}

int create_log_file( struct NETANA_DEVINSTANCE_T* ptDevInst)
{
        int fRet = 0;

        ptDevInst->log_file = NULL;

        if(g_ulTraceLevel > 0)
        {
                if (g_szDriverBaseDir) {
                        size_t pathlen     = strlen(g_szDriverBaseDir) + sizeof(ptDevInst->szDeviceName) + 2 + 4; /* +2 for 1x NUL and 1 additional '/' +4 for extension ".log" */
                        char*  logfilepath = malloc(pathlen);

                        snprintf(logfilepath, pathlen, "%s/%s.log", g_szDriverBaseDir, ptDevInst->szDeviceName);

                        ptDevInst->log_file = fopen(logfilepath, "w+");

                        if( NULL == ptDevInst->log_file)
                                perror("Error opening logfile. Traces will be printed to 'stderr'!");
                        else
                                fRet = 1;

                        free(logfilepath);
                }
                /* Insert header into log file */
                USER_Trace( ptDevInst, 0, "----- Analyzer user space log started ---------------------");
                USER_Trace( ptDevInst, 0, " Name : %s", ptDevInst->szDeviceName);
                USER_Trace( ptDevInst, 0, "-----------------------------------------------------------");
        }
        return fRet;
}

void close_log_file( struct NETANA_DEVINSTANCE_T* ptDevInst)
{
        if(g_ulTraceLevel > 0)
                USER_Trace( ptDevInst, 0, "----- Analyzer user space log stopped ---------------------");

        if( NULL != ptDevInst->log_file) {
                fclose( ptDevInst->log_file);
                ptDevInst->log_file = NULL;
        }
}

void USER_Trace( struct NETANA_DEVINSTANCE_T* ptDevInst, uint32_t ulTraceLevel, const char* szFormat, ...)
{
        va_list                 vaList;
        struct timeval          time;
        struct tm               *local_tm;

        UNREFERENCED_PARAMETER(ulTraceLevel);

        gettimeofday(&time, NULL);
        local_tm = localtime(&time.tv_sec);

        va_start(vaList, szFormat);

        if(NULL != ptDevInst->log_file){
                fprintf(ptDevInst->log_file,
                "%.2d.%.2d.%.4d %.2d:%.2d:%.2d.%.3ld.%.3ld: ",
                local_tm->tm_mday, local_tm->tm_mon + 1, local_tm->tm_year + 1900,
                local_tm->tm_hour, local_tm->tm_min, local_tm->tm_sec,
                time.tv_usec / 1000, time.tv_usec % 1000);

                /* log file is given, so add this trace to our logfile */
                vfprintf(ptDevInst->log_file, szFormat, vaList);
                fprintf(ptDevInst->log_file, "\n");
                fflush(ptDevInst->log_file);

        } else{
                /* No logfile, so print to stderr */
                fprintf( stderr, "%.2d.%.2d.%.4d %.2d:%.2d:%.2d.%.3ld.%3ld: ",
                local_tm->tm_mday, local_tm->tm_mon + 1, local_tm->tm_year + 1900,
                local_tm->tm_hour, local_tm->tm_min, local_tm->tm_sec,
                time.tv_usec / 1000, time.tv_usec % 1000);
                vfprintf(stderr, szFormat, vaList);
                fprintf(stderr, "\n");
        }
}

