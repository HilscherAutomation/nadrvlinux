/* SPDX-License-Identifier: MIT */

#pragma once
#ifndef __NETANA_ERRORS__H
#define __NETANA_ERRORS__H
/*******************************************************************************
* netANALYZER Device Driver Errors
*******************************************************************************/
/*  */
/*   Values are 32 bit values laid out as follows: */
/*  */
/*    3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 */
/*    1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 */
/*   +---+-+-+-----------------------+-------------------------------+ */
/*   |Sev|C|R|     Facility          |               Code            | */
/*   +---+-+-+-----------------------+-------------------------------+ */
/*  */
/*   where */
/*  */
/*       Sev - is the severity code */
/*  */
/*           00 - Success */
/*           01 - Informational */
/*           10 - Warning */
/*           11 - Error */
/*  */
/*       C - is the Customer code flag */
/*  */
/*       R - is a reserved bit */
/*  */
/*       Facility - is the facility code */
/*  */
/*       Code - is the facility's status code */
/*  */
/*  */
/*  Define the facility codes */
/*  */


/*  */
/*  Define the severity codes */
/*  */


/*  */
/*  MessageId: NETANA_NO_ERROR */
/*  */
/*  MessageText: */
/*  */
/*  No Error */
/*  */
#define NETANA_NO_ERROR                  ((int32_t)0x00000000L)

/*******************************************************************************
* Generic Errors
*******************************************************************************/

/*  */
/*  MessageId: NETANA_INVALID_PARAMETER */
/*  */
/*  MessageText: */
/*  */
/*  Invalid parameter */
/*  */
#define NETANA_INVALID_PARAMETER         ((int32_t)0x80200001L)

/*  */
/*  MessageId: NETANA_INVALID_PORT */
/*  */
/*  MessageText: */
/*  */
/*  Invalid port number given */
/*  */
#define NETANA_INVALID_PORT              ((int32_t)0x80200002L)

/*  */
/*  MessageId: NETANA_OUT_OF_MEMORY */
/*  */
/*  MessageText: */
/*  */
/*  Out of memory */
/*  */
#define NETANA_OUT_OF_MEMORY             ((int32_t)0x80200003L)

/*  */
/*  MessageId: NETANA_FUNCTION_FAILED */
/*  */
/*  MessageText: */
/*  */
/*  Function failed */
/*  */
#define NETANA_FUNCTION_FAILED           ((int32_t)0x80200004L)

/*  */
/*  MessageId: NETANA_INVALID_POINTER */
/*  */
/*  MessageText: */
/*  */
/*  Invalid pointer */
/*  */
#define NETANA_INVALID_POINTER           ((int32_t)0x80200005L)


/*  */
/*  MessageId: NETANA_INVALID_HANDLE */
/*  */
/*  MessageText: */
/*  */
/*  Invalid HANDLE */
/*  */
#define NETANA_INVALID_HANDLE           ((int32_t)0x80200006L)


/*  */
/*  MessageId: NETANA_NO_WORKING_DIRECTORY */
/*  */
/*  MessageText: */
/*  */
/*  No working direktory */
/*  */
#define NETANA_NO_WORKING_DIRECTORY     ((int32_t)0x80200007L)  

/*  */
/*  MessageId: NETANA_NO_ENTRY_FOUND */
/*  */
/*  MessageText: */
/*  */
/*  No Entry found */
/*  */
#define NETANA_NO_ENTRY_FOUND           ((int32_t)0x80200008L)


/*  */
/*  MessageId: NETANA_FUNCTION_NOT_AVAILABLE */
/*  */
/*  MessageText: */
/*  */
/*  Function not available */
/*  */
#define NETANA_FUNCTION_NOT_AVAILABLE   ((int32_t)0x80200009L)  


/*  */
/*  MessageId: NETANA_INVALID_BUFFERSIZE */
/*  */
/*  MessageText: */
/*  */
/*  Size of given buffer is invalid */
/*  */
#define NETANA_INVALID_BUFFERSIZE       ((int32_t)0x8020000AL)


/*  */
/*  MessageId: NETANA_ACCESS_DENIED */
/*  */
/*  MessageText: */
/*  */
/*  Resource not accessible */
/*  */
#define NETANA_ACCESS_DENIED            ((int32_t)0x8020000BL)



/*******************************************************************************
* Toolkit Errors
*******************************************************************************/
/*  */
/*  MessageId: NETANA_TKIT_INITIALIZATION_FAILED */
/*  */
/*  MessageText: */
/*  */
/*  Toolkit initialization failed */
/*  */
#define NETANA_TKIT_INITIALIZATION_FAILED ((int32_t)0x80210001L)

/*  */
/*  MessageId: NETANA_DMABUFFER_CREATION_FAILED */
/*  */
/*  MessageText: */
/*  */
/*  Error creating DMA buffers */
/*  */
#define NETANA_DMABUFFER_CREATION_FAILED ((int32_t)0x80210002L)

/*  */
/*  MessageId: NETANA_HWRESET_ERROR */
/*  */
/*  MessageText: */
/*  */
/*  netX hardware reset failed */
/*  */
#define NETANA_HWRESET_ERROR             ((int32_t)0x80210003L)

/*  */
/*  MessageId: NETANA_CHIP_NOT_SUPPORTED */
/*  */
/*  MessageText: */
/*  */
/*  Detected netX chip is not supported by toolkit */
/*  */
#define NETANA_CHIP_NOT_SUPPORTED        ((int32_t)0x80210004L)

/*  */
/*  MessageId: NETANA_DOWNLOAD_FAILED */
/*  */
/*  MessageText: */
/*  */
/*  Error downloading firmware */
/*  */
#define NETANA_DOWNLOAD_FAILED           ((int32_t)0x80210005L)

/*  */
/*  MessageId: NETANA_FW_START_FAILED */
/*  */
/*  MessageText: */
/*  */
/*  Error starting firmware */
/*  */
#define NETANA_FW_START_FAILED           ((int32_t)0x80210006L)

/*  */
/*  MessageId: NETANA_DEV_MAILBOX_FULL */
/*  */
/*  MessageText: */
/*  */
/*  Device mailbox is full */
/*  */
#define NETANA_DEV_MAILBOX_FULL          ((int32_t)0x80210007L)

/*  */
/*  MessageId: NETANA_DEV_NOT_READY */
/*  */
/*  MessageText: */
/*  */
/*  Device not ready */
/*  */
#define NETANA_DEV_NOT_READY             ((int32_t)0x80210008L)

/*  */
/*  MessageId: NETANA_DEV_MAILBOX_TOO_SHORT */
/*  */
/*  MessageText: */
/*  */
/*  Device mailbox too small for packet */
/*  */
#define NETANA_DEV_MAILBOX_TOO_SHORT     ((int32_t)0x80210009L)

/*  */
/*  MessageId: NETANA_DEV_GET_NO_PACKET */
/*  */
/*  MessageText: */
/*  */
/*  No packet available on device */
/*  */
#define NETANA_DEV_GET_NO_PACKET         ((int32_t)0x8021000AL)

/*  */
/*  MessageId: NETANA_BUFFER_TOO_SHORT */
/*  */
/*  MessageText: */
/*  */
/*  Given buffer is too short for packet */
/*  */
#define NETANA_BUFFER_TOO_SHORT          ((int32_t)0x8021000BL)

/*  */
/*  MessageId: NETANA_TRANSFER_TIMEOUT */
/*  */
/*  MessageText: */
/*  */
/*  Packet transfer timed out */
/*  */
#define NETANA_TRANSFER_TIMEOUT          ((int32_t)0x8021000CL)

/*  */
/*  MessageId: NETANA_IRQEVENT_CREATION_FAILED */
/*  */
/*  MessageText: */
/*  */
/*  Error creating interrupt events */
/*  */
#define NETANA_IRQEVENT_CREATION_FAILED  ((int32_t)0x8021000DL)

/*  */
/*  MessageId: NETANA_IRQLOCK_CREATION_FAILED */
/*  */
/*  MessageText: */
/*  */
/*  Error creating internal IRQ locks */
/*  */
#define NETANA_IRQLOCK_CREATION_FAILED   ((int32_t)0x8021000EL)

/*******************************************************************************
* Driver Errors
*******************************************************************************/
/*  */
/*  MessageId: NETANA_IOCTL_FAILED */
/*  */
/*  MessageText: */
/*  */
/*  Error sending IOCTL to driver */
/*  */
#define NETANA_IOCTL_FAILED              ((int32_t)0x80220001L)

/*  */
/*  MessageId: NETANA_DRIVER_NOT_RUNNING */
/*  */
/*  MessageText: */
/*  */
/*  netANALYZER Device Driver is not running */
/*  */
#define NETANA_DRIVER_NOT_RUNNING        ((int32_t)0x80220002L)

/*  */
/*  MessageId: NETANA_DEVICE_NOT_FOUND */
/*  */
/*  MessageText: */
/*  */
/*  Device with the given name does not exist */
/*  */
#define NETANA_DEVICE_NOT_FOUND          ((int32_t)0x80220003L)

/*  */
/*  MessageId: NETANA_DEVICE_STILL_OPEN */
/*  */
/*  MessageText: */
/*  */
/*  Device is still in use by another process */
/*  */
#define NETANA_DEVICE_STILL_OPEN         ((int32_t)0x80220004L)

/*  */
/*  MessageId: NETANA_DEVICE_NOT_OPEN */
/*  */
/*  MessageText: */
/*  */
/*  Device was not opened */
/*  */
#define NETANA_DEVICE_NOT_OPEN           ((int32_t)0x80220005L)

/*  */
/*  MessageId: NETANA_MEMORY_MAPPING_FAILED */
/*  */
/*  MessageText: */
/*  */
/*  Error mapping memory to user application */
/*  */
#define NETANA_MEMORY_MAPPING_FAILED     ((int32_t)0x80220006L)

/*  */
/*  MessageId: NETANA_FILE_OPEN_ERROR */
/*  */
/*  MessageText: */
/*  */
/*  Error opening file */
/*  */
#define NETANA_FILE_OPEN_ERROR           ((int32_t)0x80220007L)

/*  */
/*  MessageId: NETANA_FILE_READ_FAILED */
/*  */
/*  MessageText: */
/*  */
/*  Error reading file */
/*  */
#define NETANA_FILE_READ_FAILED          ((int32_t)0x80220008L)

/*  */
/*  MessageId: NETANA_FILE_CREATION_FAILED */
/*  */
/*  MessageText: */
/*  */
/*  Creation of file failed. */
/*  */
#define NETANA_FILE_CREATION_FAILED      ((int32_t)0x80220009L)

/*  */
/*  MessageId: NETANA_FILE_WRITE_FAILED */
/*  */
/*  MessageText: */
/*  */
/*  Error writing file. */
/*  */
#define NETANA_FILE_WRITE_FAILED         ((int32_t)0x8022000AL)

/*  */
/*  MessageId: NETANA_CAPTURE_ACTIVE */
/*  */
/*  MessageText: */
/*  */
/*  Capturing is currently active */
/*  */
#define NETANA_CAPTURE_ACTIVE            ((int32_t)0x8022000BL)

/*  */
/*  MessageId: NETANA_CAPTURE_NOT_ACTIVE */
/*  */
/*  MessageText: */
/*  */
/*  Capturing is currently stopped */
/*  */
#define NETANA_CAPTURE_NOT_ACTIVE        ((int32_t)0x8022000CL)

/*  */
/*  MessageId: NETANA_FILECAPTURE_NOT_ACTIVE */
/*  */
/*  MessageText: */
/*  */
/*  Capturing to file is not enabled */
/*  */
#define NETANA_FILECAPTURE_NOT_ACTIVE    ((int32_t)0x8022000DL)

/*  */
/*  MessageId: NETANA_CONFIGURATION_ERROR */
/*  */
/*  MessageText: */
/*  */
/*  Capture configuration error */
/*  */
#define NETANA_CONFIGURATION_ERROR       ((int32_t)0x8022000EL)

/*  */
/*  MessageId: NETANA_THREAD_CREATION_FAILED */
/*  */
/*  MessageText: */
/*  */
/*  Error creating worker thread */
/*  */
#define NETANA_THREAD_CREATION_FAILED    ((int32_t)0x8022000FL)

/*  */
/*  MessageId: NETANA_NO_BUFFER_DATA */
/*  */
/*  MessageText: */
/*  */
/*  No new DMA buffer data available */
/*  */
#define NETANA_NO_BUFFER_DATA            ((int32_t)0x80220010L)

/*  */
/*  MessageId: NETANA_NO_STATE_CHANGE */
/*  */
/*  MessageText: */
/*  */
/*  No state change data available */
/*  */
#define NETANA_NO_STATE_CHANGE           ((int32_t)0x80220011L)

/*  */
/*  MessageId: NETANA_NO_PLUGIN_FOUND */
/*  */
/*  MessageText: */
/*  */
/*  No Connector Plugin found */
/*  */
#define NETANA_NO_PLUGIN_FOUND          ((int32_t)0x80220012L)

/*  */
/*  MessageId: NETANA_DRV_CMD_ACTIVE */
/*  */
/*  MessageText: */
/*  */
/*  Another command is still active */
/*  */
#define NETANA_DRV_CMD_ACTIVE           ((int32_t)0x80220013L)

/*******************************************************************************
* Transport Errors
*******************************************************************************/
/*  */
/*  MessageId: NETANA_TRANSPORT_CHECKSUM_ERROR */
/*  */
/*  MessageText: */
/*  */
/*  Checksum incorrect */
/*  */
#define NETANA_TRANSPORT_CHECKSUM_ERROR       ((int32_t)0x80230001L)

/*  */
/*  MessageId: NETANA_TRANSPORT_LENGTH_INCOMPLETE */
/*  */
/*  MessageText: */
/*  */
/*  Transport lenght is incomplete */
/*  */
#define NETANA_TRANSPORT_LENGTH_INCOMPLETE    ((int32_t)0x80230002L)

/*  */
/*  MessageId: NETANA_TRANSPORT_DATA_TYPE_UNKOWN */
/*  */
/*  MessageText: */
/*  */
/*  Unknowen datatype*/
/*  */
#define NETANA_TRANSPORT_DATA_TYPE_UNKOWN     ((int32_t)0x80230003L)

/*  */
/*  MessageId: NETANA_TRANSPORT_DEVICE_UNKNOWN */
/*  */
/*  MessageText: */
/*  */
/*  Device is unknowen  */
/*  */ 
#define NETANA_TRANSPORT_DEVICE_UNKNOWN       ((int32_t)0x80230004L)

/*  */
/*  MessageId: NETANA_TRANSPORT_CHANNEL_UNKNOWN */
/*  */
/*  MessageText: */
/*  */
/*  Channel is unknowen */
/*  */
#define NETANA_TRANSPORT_CHANNEL_UNKNOWN      ((int32_t)0x80230005L)

/*  */
/*  MessageId: NETANA_TRANSPORT_SEQUENCE */
/*  */
/*  MessageText: */
/*  */
/*  Sequence error */
/*  */
#define NETANA_TRANSPORT_SEQUENCE             ((int32_t)0x80230006L)

/*  */
/*  MessageId: NETANA_TRANSPORT_BUFFEROVERFLOW */
/*  */
/*  MessageText: */
/*  */
/*  Bufferoverflow*/
/*  */
#define NETANA_TRANSPORT_BUFFEROVERFLOW       ((int32_t)0x80230007L)

/*  */
/*  MessageId: NETANA_TRANSPORT_KEEPALIVE */
/*  */
/*  MessageText: */
/*  */
/*  Keepalive error */
/*  */
#define NETANA_TRANSPORT_KEEPALIVE            ((int32_t)0x80230008L)

/*  */
/*  MessageId: NETANA_TRANSPORT_RESOURCE */
/*  */
/*  MessageText: */
/*  */
/*  Recource error */
/*  */
#define NETANA_TRANSPORT_RESOURCE             ((int32_t)0x80230009L)

/*  */
/*  MessageId: NETANA_TRANSPORT_ERROR_UNKNOWN */
/*  */
/*  MessageText: */
/*  */
/*  Unknowen error */
/*  */
#define NETANA_TRANSPORT_ERROR_UNKNOWN        ((int32_t)0x8023000AL)


/*  */
/*  MessageId: NETANA_TRANSPORT_ERROR_UNKNOWN */
/*  */
/*  MessageText: */
/*  */
/*  Unknowen error */
/*  */
#define NETANA_TRANSPORT_RECV_TIMEOUT         ((int32_t)0x8023000BL)

//
// MessageId: NETANA_TRANSPORT_SEND_TIMEOUT
//
// MessageText:
//
// Time out while sending data
//
#define NETANA_TRANSPORT_SEND_TIMEOUT      ((int32_t)0x8023000CL)

//
// MessageId: NETANA_TRANSPORT_CONNECT
//
// MessageText:
//
// Unable to communicate to the device / no answer
//
#define NETANA_TRANSPORT_CONNECT           ((int32_t)0x8023000DL)

//
// MessageId: NETANA_TRANSPORT_ABORTED
//
// MessageText:
//
// Transfer has been aborted due to keep alive timeout or interface detachment
//
#define NETANA_TRANSPORT_ABORTED           ((int32_t)0x8023000EL)

//
// MessageId: NETANA_TRANSPORT_INVALID_RESPONSE
//
// MessageText:
//
// The packet response was rejected due to invalid packet data
//
#define NETANA_TRANSPORT_INVALID_RESPONSE  ((int32_t)0x8023000FL)

//
// MessageId: NETANA_TRANSPORT_UNKNOWN_DATALAYER
//
// MessageText:
//
// The data layer provided by the device is not supported
//
#define NETANA_TRANSPORT_UNKNOWN_DATALAYER ((int32_t)0x80230010L)

//
// MessageId: NETANA_CONNECTOR_FUNCTIONS_READ_ERROR
//
// MessageText:
//
// Error reading the connector functions from the DLL
//
#define NETANA_CONNECTOR_FUNCTIONS_READ_ERROR ((int32_t)0x80230011L)

//
// MessageId: NETANA_CONNECTOR_IDENTIFIER_TOO_LONG
//
// MessageText:
//
// Connector delivers an identifier longer than 6 characters
//
#define NETANA_CONNECTOR_IDENTIFIER_TOO_LONG ((int32_t)0x80230012L)

//
// MessageId: NETANA_CONNECTOR_IDENTIFIER_EMPTY
//
// MessageText:
//
// Connector delivers an empty dentifier
//
#define NETANA_CONNECTOR_IDENTIFIER_EMPTY  ((int32_t)0x80230013L)

//
// MessageId: NETANA_CONNECTOR_DUPLICATE_IDENTIFIER
//
// MessageText:
//
// Connector identifier already used
//
#define NETANA_CONNECTOR_DUPLICATE_IDENTIFIER ((int32_t)0x80230014L)


/*******************************************************************************
* NETANA API Transport Header State Errors
*******************************************************************************/
//
// MessageId: NETANA_TRANSPORT_DATA_TOO_SHORT
//
// MessageText:
//
// Received transaction data too short
//
#define NETANA_TRANSPORT_DATA_TOO_SHORT    ((int32_t)0x80230024L)

//
// MessageId: NETANA_TRANSPORT_UNSUPPORTED_FUNCTION
//
// MessageText:
//
// FUNCTION IS NOT SUPPORTED
//
#define NETANA_TRANSPORT_UNSUPPORTED_FUNCTION    ((int32_t)0x80230025L)

//
// MessageId: NETANA_TRANSPORT_TIMEOUT
//
// MessageText:
//
// TRANSPORT TIMOUT 
//
#define NETANA_TRANSPORT_TIMEOUT    ((int32_t)0x80230026L)


/*******************************************************************************
* NETANA API Marshaller Target Error
*******************************************************************************/

//
// MessageId: NETANA_CAPTURE_ERROR_ON_TARGET
//
// MessageText:
//
// TARGET SYSTEM ERROR 
//
#define NETANA_CAPTURE_ERROR_ON_TARGET    ((int32_t)0xC0230001L)


#endif  /*__NETANA_ERRORS__H */
