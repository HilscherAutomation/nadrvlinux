;#ifndef __NETANA_ERRORS__H
;#define __NETANA_ERRORS__H
;

LanguageNames =
    (
        English = 0x0409:netana_errors_ENU
    )


;/*******************************************************************************
;* netANALYZER Device Driver Errors
;*******************************************************************************/

OutputBase      = 16
MessageIdTypedef= long

SeverityNames   = (
                    Error       = 3
                    Warning     = 2
                    Information = 1
                    Success     = 0
                  )

FacilityNames   = (
                    FACILITY_NULL    = 0
                    NETANA_GENERIC   = 32
                    NETANA_TOOLKIT   = 33
                    NETANA_DRIVER    = 34
                    NETANA_TRANSPORT = 35
                  )

MessageId       = 0x0000
Severity        = Success
Facility        = FACILITY_NULL
SymbolicName    = NETANA_NO_ERROR
Language        = English
No Error
.

;/*******************************************************************************
;* Generic Errors
;*******************************************************************************/

MessageId       = 0x0001
Severity        = Warning
Facility        = NETANA_GENERIC
SymbolicName    = NETANA_INVALID_PARAMETER
Language        = English
Invalid parameter
.

MessageId       = 0x0002
Severity        = Warning
Facility        = NETANA_GENERIC
SymbolicName    = NETANA_INVALID_PORT
Language        = English
Invalid port number given
.

MessageId       = 0x0003
Severity        = Warning
Facility        = NETANA_GENERIC
SymbolicName    = NETANA_OUT_OF_MEMORY
Language        = English
Out of memory
.

MessageId       = 0x0004
Severity        = Warning
Facility        = NETANA_GENERIC
SymbolicName    = NETANA_FUNCTION_FAILED
Language        = English
Function failed
.

MessageId       = 0x0005
Severity        = Warning
Facility        = NETANA_GENERIC
SymbolicName    = NETANA_INVALID_POINTER
Language        = English
Invalid pointer
.

MessageId       = 0x0006
Severity        = Warning
Facility        = NETANA_GENERIC
SymbolicName    = NETANA_INVALID_HANDLE
Language        = English
Out of memory
.

MessageId       = 0x0007
Severity        = Warning
Facility        = NETANA_GENERIC
SymbolicName    = NETANA_NO_WORKING_DIRECTORY
Language        = English
Function failed
.

MessageId       = 0x0008
Severity        = Warning
Facility        = NETANA_GENERIC
SymbolicName    = NETANA_NO_ENTRY_FOUND
Language        = English
Invalid pointer
.

MessageId       = 0x0009
Severity        = Warning
Facility        = NETANA_GENERIC
SymbolicName    = NETANA_FUNCTION_NOT_AVAILABLE
Language        = English
Invalid pointer
.


;/*******************************************************************************
;* Toolkit Errors
;*******************************************************************************/

MessageId       = 0x0001
Severity        = Warning
Facility        = NETANA_TOOLKIT
SymbolicName    = NETANA_TKIT_INITIALIZATION_FAILED
Language        = English
Toolkit initialization failed
.

MessageId       = 0x0002
Severity        = Warning
Facility        = NETANA_TOOLKIT
SymbolicName    = NETANA_DMABUFFER_CREATION_FAILED
Language        = English
Error creating DMA buffers
.

MessageId       = 0x0003
Severity        = Warning
Facility        = NETANA_TOOLKIT
SymbolicName    = NETANA_HWRESET_ERROR
Language        = English
netX hardware reset failed
.

MessageId       = 0x0004
Severity        = Warning
Facility        = NETANA_TOOLKIT
SymbolicName    = NETANA_CHIP_NOT_SUPPORTED
Language        = English
Detected netX chip is not supported by toolkit
.

MessageId       = 0x0005
Severity        = Warning
Facility        = NETANA_TOOLKIT
SymbolicName    = NETANA_DOWNLOAD_FAILED
Language        = English
Error downloading firmware
.

MessageId       = 0x0006
Severity        = Warning
Facility        = NETANA_TOOLKIT
SymbolicName    = NETANA_FW_START_FAILED
Language        = English
Error starting firmware
.

MessageId       = 0x0007
Severity        = Warning
Facility        = NETANA_TOOLKIT
SymbolicName    = NETANA_DEV_MAILBOX_FULL
Language        = English
Device mailbox is full
.

MessageId       = 0x0008
Severity        = Warning
Facility        = NETANA_TOOLKIT
SymbolicName    = NETANA_DEV_NOT_READY
Language        = English
Device not ready
.

MessageId       = 0x0009
Severity        = Warning
Facility        = NETANA_TOOLKIT
SymbolicName    = NETANA_DEV_MAILBOX_TOO_SHORT
Language        = English
Device mailbox too small for packet
.

MessageId       = 0x000A
Severity        = Warning
Facility        = NETANA_TOOLKIT
SymbolicName    = NETANA_DEV_GET_NO_PACKET
Language        = English
No packet available on device
.

MessageId       = 0x000B
Severity        = Warning
Facility        = NETANA_TOOLKIT
SymbolicName    = NETANA_BUFFER_TOO_SHORT
Language        = English
Given buffer is too short for packet
.

MessageId       = 0x000C
Severity        = Warning
Facility        = NETANA_TOOLKIT
SymbolicName    = NETANA_TRANSFER_TIMEOUT
Language        = English
Packet transfer timed out
.

MessageId       = 0x000D
Severity        = Warning
Facility        = NETANA_TOOLKIT
SymbolicName    = NETANA_IRQEVENT_CREATION_FAILED
Language        = English
Error creating interrupt events
.

MessageId       = 0x000E
Severity        = Warning
Facility        = NETANA_TOOLKIT
SymbolicName    = NETANA_IRQLOCK_CREATION_FAILED
Language        = English
Error creating internal IRQ locks
.

;/*******************************************************************************
;* Driver Errors
;*******************************************************************************/

MessageId       = 0x0001
Severity        = Warning
Facility        = NETANA_DRIVER
SymbolicName    = NETANA_IOCTL_FAILED
Language        = English
Error sending IOCTL to driver
.

MessageId       = 0x0002
Severity        = Warning
Facility        = NETANA_DRIVER
SymbolicName    = NETANA_DRIVER_NOT_RUNNING
Language        = English
netANALYZER Device Driver is not running
.

MessageId       = 0x0003
Severity        = Warning
Facility        = NETANA_DRIVER
SymbolicName    = NETANA_DEVICE_NOT_FOUND
Language        = English
Device with the given name does not exist
.

MessageId       = 0x0004
Severity        = Warning
Facility        = NETANA_DRIVER
SymbolicName    = NETANA_DEVICE_STILL_OPEN
Language        = English
Device is still in use by another process
.

MessageId       = 0x0005
Severity        = Warning
Facility        = NETANA_DRIVER
SymbolicName    = NETANA_DEVICE_NOT_OPEN
Language        = English
Device was not opened
.

MessageId       = 0x0006
Severity        = Warning
Facility        = NETANA_DRIVER
SymbolicName    = NETANA_MEMORY_MAPPING_FAILED
Language        = English
Error mapping memory to user application
.

MessageId       = 0x0007
Severity        = Warning
Facility        = NETANA_DRIVER
SymbolicName    = NETANA_FILE_OPEN_ERROR
Language        = English
Error opening file
.

MessageId       = 0x0008
Severity        = Warning
Facility        = NETANA_DRIVER
SymbolicName    = NETANA_FILE_READ_FAILED
Language        = English
Error reading file
.

MessageId       = 0x0009
Severity        = Warning
Facility        = NETANA_DRIVER
SymbolicName    = NETANA_FILE_CREATION_FAILED
Language        = English
Creation of file failed.
.

MessageId       = 0x000A
Severity        = Warning
Facility        = NETANA_DRIVER
SymbolicName    = NETANA_FILE_WRITE_FAILED
Language        = English
Error writing file.
.

MessageId       = 0x000B
Severity        = Warning
Facility        = NETANA_DRIVER
SymbolicName    = NETANA_CAPTURE_ACTIVE
Language        = English
Capturing is currently active
.

MessageId       = 0x000C
Severity        = Warning
Facility        = NETANA_DRIVER
SymbolicName    = NETANA_CAPTURE_NOT_ACTIVE
Language        = English
Capturing is currently stopped
.

MessageId       = 0x000D
Severity        = Warning
Facility        = NETANA_DRIVER
SymbolicName    = NETANA_FILECAPTURE_NOT_ACTIVE
Language        = English
Capturing to file is not enabled
.

MessageId       = 0x000E
Severity        = Warning
Facility        = NETANA_DRIVER
SymbolicName    = NETANA_CONFIGURATION_ERROR
Language        = English
Capture configuration error
.

MessageId       = 0x000F
Severity        = Warning
Facility        = NETANA_DRIVER
SymbolicName    = NETANA_THREAD_CREATION_FAILED
Language        = English
Error creating worker thread
.

MessageId       = 0x0010
Severity        = Warning
Facility        = NETANA_DRIVER
SymbolicName    = NETANA_NO_BUFFER_DATA
Language        = English
No new DMA buffer data available
.

MessageId       = 0x0011
Severity        = Warning
Facility        = NETANA_DRIVER
SymbolicName    = NETANA_NO_STATE_CHANGE
Language        = English
No state change data available
.

MessageId       = 0x0012
Severity        = Warning
Facility        = NETANA_DRIVER
SymbolicName    = NETANA_NO_PLUGIN_FOUND
Language        = English
No state change data available
.


;/*******************************************************************************
;* Marshaller Errors
;*******************************************************************************/

MessageId       = 0x0001
Severity        = Warning
Facility        = NETANA_TRANSPORT
SymbolicName    = NETANA_TRANSPORT_CHECKSUM_ERROR
Language        = English
No state change data available
.

MessageId       = 0x0002
Severity        = Warning
Facility        = NETANA_TRANSPORT
SymbolicName    = NETANA_TRANSPORT_LENGTH_INCOMPLETE
Language        = English
No state change data available
.

MessageId       = 0x0003
Severity        = Warning
Facility        = NETANA_TRANSPORT
SymbolicName    = NETANA_TRANSPORT_DATA_TYPE_UNKOWN
Language        = English
No state change data available
.

MessageId       = 0x0004
Severity        = Warning
Facility        = NETANA_TRANSPORT
SymbolicName    = NETANA_TRANSPORT_DEVICE_UNKNOWN
Language        = English
No state change data available
.

MessageId       = 0x0005
Severity        = Warning
Facility        = NETANA_TRANSPORT
SymbolicName    = NETANA_TRANSPORT_CHANNEL_UNKNOWN
Language        = English
No state change data available
.

MessageId       = 0x0006
Severity        = Warning
Facility        = NETANA_TRANSPORT
SymbolicName    = NETANA_TRANSPORT_SEQUENCE
Language        = English
No state change data available
.

MessageId       = 0x0007
Severity        = Warning
Facility        = NETANA_TRANSPORT
SymbolicName    = NETANA_TRANSPORT_BUFFEROVERFLOW
Language        = English
No state change data available
.

MessageId       = 0x0008
Severity        = Warning
Facility        = NETANA_TRANSPORT
SymbolicName    = NETANA_TRANSPORT_KEEPALIVE
Language        = English
No state change data available
.

MessageId       = 0x0009
Severity        = Warning
Facility        = NETANA_TRANSPORT
SymbolicName    = NETANA_TRANSPORT_RESOURCE
Language        = English
No state change data available
.

MessageId       = 0x000A
Severity        = Warning
Facility        = NETANA_TRANSPORT
SymbolicName    = NETANA_TRANSPORT_ERROR_UNKNOWN
Language        = English
No state change data available
.

MessageId       = 0x000B
Severity        = Warning
Facility        = NETANA_TRANSPORT
SymbolicName    = NETANA_TRANSPORT_RECV_TIMEOUT
Language        = English
No state change data available
.

MessageId       = 0x000C
Severity        = Warning
Facility        = NETANA_TRANSPORT
SymbolicName    = NETANA_TRANSPORT_SEND_TIMEOUT
Language        = English
No state change data available
.

MessageId       = 0x000D
Severity        = Warning
Facility        = NETANA_TRANSPORT
SymbolicName    = NETANA_TRANSPORT_CONNECT
Language        = English
No state change data available
.

MessageId       = 0x000E
Severity        = Warning
Facility        = NETANA_TRANSPORT
SymbolicName    = NETANA_TRANSPORT_ABORTED
Language        = English
No state change data available
.

MessageId       = 0x000F
Severity        = Warning
Facility        = NETANA_TRANSPORT
SymbolicName    = NETANA_TRANSPORT_INVALID_RESPONSE
Language        = English
No state change data available
.

MessageId       = 0x0010
Severity        = Warning
Facility        = NETANA_TRANSPORT
SymbolicName    = NETANA_TRANSPORT_UNKNOWN_DATALAYER
Language        = English
No state change data available
.

MessageId       = 0x0011
Severity        = Warning
Facility        = NETANA_TRANSPORT
SymbolicName    = NETANA_CONNECTOR_FUNCTIONS_READ_ERROR
Language        = English
No state change data available
.

MessageId       = 0x0012
Severity        = Warning
Facility        = NETANA_TRANSPORT
SymbolicName    = NETANA_CONNECTOR_IDENTIFIER_TOO_LONG
Language        = English
No state change data available
.

MessageId       = 0x0013
Severity        = Warning
Facility        = NETANA_TRANSPORT
SymbolicName    = NETANA_CONNECTOR_IDENTIFIER_EMPTY
Language        = English
No state change data available
.

MessageId       = 0x0014
Severity        = Warning
Facility        = NETANA_TRANSPORT
SymbolicName    = NETANA_CONNECTOR_DUPLICATE_IDENTIFIER
Language        = English
No state change data available
.

MessageId       = 0x0015
Severity        = Warning
Facility        = NETANA_TRANSPORT
SymbolicName    = NETANA_TRANSPORT_DATA_TOO_SHORT
Language        = English
No state change data available
.

MessageId       = 0x0016
Severity        = Warning
Facility        = NETANA_TRANSPORT
SymbolicName    = NETANA_TRANSPORT_UNSUPPORTED_FUNCTION
Language        = English
No state change data available
.

MessageId       = 0x0017
Severity        = Warning
Facility        = NETANA_TRANSPORT
SymbolicName    = NETANA_TRANSPORT_TIMEOUT
Language        = English
No state change data available
.

MessageId       = 0x0018
Severity        = Warning
Facility        = NETANA_TRANSPORT
SymbolicName    = NETANA_CAPTURE_ERROR_ON_TARGET
Language        = English
No state change data available
.

;/*******************************************************************************/
;
;#endif  /*__NETANA_ERRORS__H */
;