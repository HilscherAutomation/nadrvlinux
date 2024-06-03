/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __OS_INCLUDES__H
#define __OS_INCLUDES__H

#include <linux/llist.h>
#ifdef __KERNEL__
        #include <asm/io.h>
        #include <linux/types.h>
#else
        #include <stdint.h>
#endif

#define _MSC_VER 0

#define STAILQ_ENTRY(a) struct llist_node

#define OS_PATH_SEPERATOR   "\\"

#ifndef NULL
        #define NULL  ((void*)0)
#endif

#undef SLIST_ENTRY

#define UNREFERENCED_PARAMETER(a)  (void)(a)

#endif /*  __OS_INCLUDES__H */
