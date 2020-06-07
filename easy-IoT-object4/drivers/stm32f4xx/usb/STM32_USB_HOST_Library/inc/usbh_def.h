/**
 ******************************************************************************
 * @file    usbh_def.h
 * @author  MCD Application Team
 * @version V2.0.0
 * @date    22-July-2011
 * @brief   Definitions used in the USB host library
 ******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */
/** @addtogroup USBH_LIB
 * @{
 */

/** @addtogroup USBH_LIB_CORE
 * @{
 */

/** @defgroup USBH_DEF
 * @brief This file is includes USB descriptors
 * @{
 */


#ifndef USBH_DEF_H
#define USBH_DEF_H

#ifndef USBH_NULL
#define USBH_NULL ((void *)0)
#endif 

#define ValBit(VAR,POS)                               (VAR & (1 << POS))
#define SetBit(VAR,POS)                               (VAR |= (1 << POS))
#define ClrBit(VAR,POS)                               (VAR &= ((1 << POS)^255))

#define LE16(addr)             (((u16)(*((u8 *)(addr))))\
+ (((u16)(*(((u8 *)(addr)) + 1))) << 8))

#define USB_LEN_DESC_HDR                               0x02
#define USB_LEN_DEV_DESC                               0x12
#define USB_LEN_CFG_DESC                               0x09
#define USB_LEN_IF_DESC                                0x09
#define USB_LEN_EP_DESC                                0x07
#define USB_LEN_OTG_DESC                               0x03
#define USB_LEN_SETUP_PKT                              0x08

/* bmRequestType :D7 Data Phase Transfer Direction  */
#define USB_REQ_DIR_MASK                               0x80
#define USB_H2D                                        0x00
#define USB_D2H                                        0x80

/* bmRequestType D6..5 Type */
#define USB_REQ_TYPE_STANDARD                          0x00
#define USB_REQ_TYPE_CLASS                             0x20
#define USB_REQ_TYPE_VENDOR                            0x40
#define USB_REQ_TYPE_RESERVED                          0x60

/* bmRequestType D4..0 Recipient */
#define USB_REQ_RECIPIENT_DEVICE                       0x00
#define USB_REQ_RECIPIENT_INTERFACE                    0x01
#define USB_REQ_RECIPIENT_ENDPOINT                     0x02
#define USB_REQ_RECIPIENT_OTHER                        0x03

/* Table 9-4. Standard Request Codes  */
/* bRequest , Value */
#define USB_REQ_GET_STATUS                             0x00
#define USB_REQ_CLEAR_FEATURE                          0x01
#define USB_REQ_SET_FEATURE                            0x03
#define USB_REQ_SET_ADDRESS                            0x05
#define USB_REQ_GET_DESCRIPTOR                         0x06
#define USB_REQ_SET_DESCRIPTOR                         0x07
#define USB_REQ_GET_CONFIGURATION                      0x08
#define USB_REQ_SET_CONFIGURATION                      0x09
#define USB_REQ_GET_INTERFACE                          0x0A
#define USB_REQ_SET_INTERFACE                          0x0B
#define USB_REQ_SYNCH_FRAME                            0x0C

/* Table 9-5. Descriptor Types of USB Specifications */
#define USB_DESC_TYPE_DEVICE                              1
#define USB_DESC_TYPE_CONFIGURATION                       2
#define USB_DESC_TYPE_STRING                              3
#define USB_DESC_TYPE_INTERFACE                           4
#define USB_DESC_TYPE_ENDPOINT                            5
#define USB_DESC_TYPE_DEVICE_QUALIFIER                    6
#define USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION           7
#define USB_DESC_TYPE_INTERFACE_POWER                     8
#define USB_DESC_TYPE_HID                                 0x21
#define USB_DESC_TYPE_HID_REPORT                          0x22


#define USB_DEVICE_DESC_SIZE                               18
#define USB_CONFIGURATION_DESC_SIZE                        9
#define USB_HID_DESC_SIZE                                  9
#define USB_INTERFACE_DESC_SIZE                            9
#define USB_ENDPOINT_DESC_SIZE                             7

/* Descriptor Type and Descriptor Index  */
/* Use the following values when calling the function USBH_GetDescriptor  */
#define USB_DESC_DEVICE                    ((USB_DESC_TYPE_DEVICE << 8) & 0xFF00)
#define USB_DESC_CONFIGURATION             ((USB_DESC_TYPE_CONFIGURATION << 8) & 0xFF00)
#define USB_DESC_STRING                    ((USB_DESC_TYPE_STRING << 8) & 0xFF00)
#define USB_DESC_INTERFACE                 ((USB_DESC_TYPE_INTERFACE << 8) & 0xFF00)
#define USB_DESC_ENDPOINT                  ((USB_DESC_TYPE_INTERFACE << 8) & 0xFF00)
#define USB_DESC_DEVICE_QUALIFIER          ((USB_DESC_TYPE_DEVICE_QUALIFIER << 8) & 0xFF00)
#define USB_DESC_OTHER_SPEED_CONFIGURATION ((USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION << 8) & 0xFF00)
#define USB_DESC_INTERFACE_POWER           ((USB_DESC_TYPE_INTERFACE_POWER << 8) & 0xFF00)
#define USB_DESC_HID_REPORT                ((USB_DESC_TYPE_HID_REPORT << 8) & 0xFF00)
#define USB_DESC_HID                       ((USB_DESC_TYPE_HID << 8) & 0xFF00)


#define USB_EP_TYPE_CTRL                               0x00
#define USB_EP_TYPE_ISOC                               0x01
#define USB_EP_TYPE_BULK                               0x02
#define USB_EP_TYPE_INTR                               0x03

#define USB_EP_DIR_OUT                                 0x00
#define USB_EP_DIR_IN                                  0x80
#define USB_EP_DIR_MSK                                 0x80

/* supported classes */
#define USB_MSC_CLASS                                   0x08
#define USB_HID_CLASS                                   0x03

/* Interface Descriptor field values for HID Boot Protocol */
#define HID_BOOT_CODE                                  0x01
#define HID_KEYBRD_BOOT_CODE                           0x01
#define HID_MOUSE_BOOT_CODE                            0x02

/* As per USB specs 9.2.6.4 :Standard request with data request timeout: 5sec
Standard request with no data stage timeout : 50ms */
#define DATA_STAGE_TIMEOUT                              5000
#define NODATA_STAGE_TIMEOUT                            50

typedef u32 dma_addr_t;
/**
 * @}
 */
#define URB_SHORT_NOT_OK	0x0001	/* report short reads as errors */
#define URB_ISO_ASAP		0x0002	/* iso-only, urb->start_frame
					 * ignored */
#define URB_NO_TRANSFER_DMA_MAP	0x0004	/* urb->transfer_dma valid on submit */
#define URB_NO_FSBR		0x0020	/* UHCI-specific */
#define URB_ZERO_PACKET		0x0040	/* Finish bulk OUT with short packet */
#define URB_NO_INTERRUPT	0x0080	/* HINT: no non-error interrupt
					 * needed */
#define URB_FREE_BUFFER		0x0100	/* Free transfer buffer with the URB */

/* The following flags are used internally by usbcore and HCDs */
#define URB_DIR_IN		0x0200	/* Transfer from device to host */
#define URB_DIR_OUT		0
#define URB_DIR_MASK		URB_DIR_IN

#define URB_DMA_MAP_SINGLE	0x00010000	/* Non-scatter-gather mapping */
#define URB_DMA_MAP_PAGE	0x00020000	/* HCD-unsupported S-G */
#define URB_DMA_MAP_SG		0x00040000	/* HCD-supported S-G */
#define URB_MAP_LOCAL		0x00080000	/* HCD-local-memory mapping */
#define URB_SETUP_MAP_SINGLE	0x00100000	/* Setup packet DMA mapped */
#define URB_SETUP_MAP_LOCAL	0x00200000	/* HCD-local setup packet */
#define URB_DMA_SG_COMBINED	0x00400000	/* S-G entries were combined */
#define URB_ALIGNED_TEMP_BUFFER	0x00800000	/* Temp buffer was alloc'd */


#define USB_ENDPOINT_NUMBER_MASK	0x0f	/* in bEndpointAddress */
#define USB_ENDPOINT_DIR_MASK		0x80

#define USB_ENDPOINT_SYNCTYPE		0x0c
#define USB_ENDPOINT_SYNC_NONE		(0 << 2)
#define USB_ENDPOINT_SYNC_ASYNC		(1 << 2)
#define USB_ENDPOINT_SYNC_ADAPTIVE	(2 << 2)
#define USB_ENDPOINT_SYNC_SYNC		(3 << 2)

#define USB_ENDPOINT_XFERTYPE_MASK	0x03	/* in bmAttributes */
#define USB_ENDPOINT_XFER_CONTROL	0
#define USB_ENDPOINT_XFER_ISOC		1
#define USB_ENDPOINT_XFER_BULK		2
#define USB_ENDPOINT_XFER_INT		3
#define USB_ENDPOINT_MAX_ADJUSTABLE	0x80

#define USB_DIR_OUT			0		/* to device */
#define USB_DIR_IN			0x80		/* to host */

/*
 * USB types, the second of three bRequestType fields
 */
#define USB_TYPE_MASK			(0x03 << 5)
#define USB_TYPE_STANDARD		(0x00 << 5)
#define USB_TYPE_CLASS			(0x01 << 5)
#define USB_TYPE_VENDOR			(0x02 << 5)
#define USB_TYPE_RESERVED		(0x03 << 5)

/*
 * USB recipients, the third of three bRequestType fields
 */
#define USB_RECIP_MASK			0x1f
#define USB_RECIP_DEVICE		0x00
#define USB_RECIP_INTERFACE		0x01
#define USB_RECIP_ENDPOINT		0x02
#define USB_RECIP_OTHER			0x03
/* From Wireless USB 1.0 */
#define USB_RECIP_PORT			0x04
#define USB_RECIP_RPIPE		0x05

/*
 * Standard requests, for the bRequest field of a SETUP packet.
 *
 * These are qualified by the bRequestType field, so that for example
 * TYPE_CLASS or TYPE_VENDOR specific feature flags could be retrieved
 * by a GET_STATUS request.
 */
#define USB_REQ_GET_STATUS		0x00
#define USB_REQ_CLEAR_FEATURE		0x01
#define USB_REQ_SET_FEATURE		0x03
#define USB_REQ_SET_ADDRESS		0x05
#define USB_REQ_GET_DESCRIPTOR		0x06
#define USB_REQ_SET_DESCRIPTOR		0x07
#define USB_REQ_GET_CONFIGURATION	0x08
#define USB_REQ_SET_CONFIGURATION	0x09
#define USB_REQ_GET_INTERFACE		0x0A
#define USB_REQ_SET_INTERFACE		0x0B
#define USB_REQ_SYNCH_FRAME		0x0C

#define USB_REQ_SET_ENCRYPTION		0x0D	/* Wireless USB */
#define USB_REQ_GET_ENCRYPTION		0x0E
#define USB_REQ_RPIPE_ABORT		0x0E
#define USB_REQ_SET_HANDSHAKE		0x0F
#define USB_REQ_RPIPE_RESET		0x0F
#define USB_REQ_GET_HANDSHAKE		0x10
#define USB_REQ_SET_CONNECTION		0x11
#define USB_REQ_SET_SECURITY_DATA	0x12
#define USB_REQ_GET_SECURITY_DATA	0x13
#define USB_REQ_SET_WUSB_DATA		0x14
#define USB_REQ_LOOPBACK_DATA_WRITE	0x15
#define USB_REQ_LOOPBACK_DATA_READ	0x16
#define USB_REQ_SET_INTERFACE_DS	0x17

/* The Link Power Management (LPM) ECN defines USB_REQ_TEST_AND_SET command,
 * used by hubs to put ports into a new L1 suspend state, except that it
 * forgot to define its number ...
 */

/*
 * USB feature flags are written using USB_REQ_{CLEAR,SET}_FEATURE, and
 * are read as a bit array returned by USB_REQ_GET_STATUS.  (So there
 * are at most sixteen features of each type.)  Hubs may also support a
 * new USB_REQ_TEST_AND_SET_FEATURE to put ports into L1 suspend.
 */
#define USB_DEVICE_SELF_POWERED		0	/* (read only) */
#define USB_DEVICE_REMOTE_WAKEUP	1	/* dev may initiate wakeup */
#define USB_DEVICE_TEST_MODE		2	/* (wired high speed only) */
#define USB_DEVICE_BATTERY		2	/* (wireless) */
#define USB_DEVICE_B_HNP_ENABLE		3	/* (otg) dev may initiate HNP */
#define USB_DEVICE_WUSB_DEVICE		3	/* (wireless)*/
#define USB_DEVICE_A_HNP_SUPPORT	4	/* (otg) RH port supports HNP */
#define USB_DEVICE_A_ALT_HNP_SUPPORT	5	/* (otg) other RH port does */
#define USB_DEVICE_DEBUG_MODE		6	/* (special devices only) */

/*
 * Test Mode Selectors
 * See USB 2.0 spec Table 9-7
 */
#define	TEST_J		1
#define	TEST_K		2
#define	TEST_SE0_NAK	3
#define	TEST_PACKET	4
#define	TEST_FORCE_EN	5

/*
 * New Feature Selectors as added by USB 3.0
 * See USB 3.0 spec Table 9-6
 */
#define USB_DEVICE_U1_ENABLE	48	/* dev may initiate U1 transition */
#define USB_DEVICE_U2_ENABLE	49	/* dev may initiate U2 transition */
#define USB_DEVICE_LTM_ENABLE	50	/* dev may send LTM */
#define USB_INTRF_FUNC_SUSPEND	0	/* function suspend */

#define USB_INTR_FUNC_SUSPEND_OPT_MASK	0xFF00

#define USB_ENDPOINT_HALT		0	/* IN/OUT will STALL */

/* Bit array elements as returned by the USB_REQ_GET_STATUS request. */
#define USB_DEV_STAT_U1_ENABLED		2	/* transition into U1 state */
#define USB_DEV_STAT_U2_ENABLED		3	/* transition into U2 state */
#define USB_DEV_STAT_LTM_ENABLED	4	/* Latency tolerance messages */

/*-------------------------------------------------------------------------*/

/*
 * STANDARD DESCRIPTORS ... as returned by GET_DESCRIPTOR, or
 * (rarely) accepted by SET_DESCRIPTOR.
 *
 * Note that all multi-byte values here are encoded in little endian
 * byte order "on the wire".  Within the kernel and when exposed
 * through the Linux-USB APIs, they are not converted to cpu byte
 * order; it is the responsibility of the client code to do this.
 * The single exception is when device and configuration descriptors (but
 * not other descriptors) are read from usbfs (i.e. /proc/bus/usb/BBB/DDD);
 * in this case the fields are converted to host endianness by the kernel.
 */

/*
 * Descriptor types ... USB 2.0 spec table 9.5
 */
#define USB_DT_DEVICE			0x01
#define USB_DT_CONFIG			0x02
#define USB_DT_STRING			0x03
#define USB_DT_INTERFACE		0x04
#define USB_DT_ENDPOINT			0x05
#define USB_DT_DEVICE_QUALIFIER		0x06
#define USB_DT_OTHER_SPEED_CONFIG	0x07
#define USB_DT_INTERFACE_POWER		0x08
/* these are from a minor usb 2.0 revision (ECN) */
#define USB_DT_OTG			0x09
#define USB_DT_DEBUG			0x0a
#define USB_DT_INTERFACE_ASSOCIATION	0x0b
/* these are from the Wireless USB spec */
#define USB_DT_SECURITY			0x0c
#define USB_DT_KEY			0x0d
#define USB_DT_ENCRYPTION_TYPE		0x0e
#define USB_DT_BOS			0x0f
#define USB_DT_DEVICE_CAPABILITY	0x10
#define USB_DT_WIRELESS_ENDPOINT_COMP	0x11
#define USB_DT_WIRE_ADAPTER		0x21
#define USB_DT_RPIPE			0x22
#define USB_DT_CS_RADIO_CONTROL		0x23
/* From the T10 UAS specification */
#define USB_DT_PIPE_USAGE		0x24
/* From the USB 3.0 spec */
#define	USB_DT_SS_ENDPOINT_COMP		0x30

/* Conventional codes for class-specific descriptors.  The convention is
 * defined in the USB "Common Class" Spec (3.11).  Individual class specs
 * are authoritative for their usage, not the "common class" writeup.
 */
#define USB_DT_CS_DEVICE		(USB_TYPE_CLASS | USB_DT_DEVICE)
#define USB_DT_CS_CONFIG		(USB_TYPE_CLASS | USB_DT_CONFIG)
#define USB_DT_CS_STRING		(USB_TYPE_CLASS | USB_DT_STRING)
#define USB_DT_CS_INTERFACE		(USB_TYPE_CLASS | USB_DT_INTERFACE)
#define USB_DT_CS_ENDPOINT		(USB_TYPE_CLASS | USB_DT_ENDPOINT)

/* All standard descriptors have these 2 fields at the beginning */
struct usb_descriptor_header {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
} __packed;


/*-------------------------------------------------------------------------*/

/* USB_DT_DEVICE: Device descriptor */
struct usb_device_descriptor {
	uint8_t  bLength;
	uint8_t  bDescriptorType;

	uint16_t bcdUSB;
	uint8_t  bDeviceClass;
	uint8_t  bDeviceSubClass;
	uint8_t  bDeviceProtocol;
	uint8_t  bMaxPacketSize0;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice;
	uint8_t  iManufacturer;
	uint8_t  iProduct;
	uint8_t  iSerialNumber;
	uint8_t  bNumConfigurations;
} __packed;

#define USB_DT_DEVICE_SIZE		18


/*
 * Device and/or Interface Class codes
 * as found in bDeviceClass or bInterfaceClass
 * and defined by www.usb.org documents
 */
#define USB_CLASS_PER_INTERFACE		0	/* for DeviceClass */
#define USB_CLASS_AUDIO			1
#define USB_CLASS_COMM			2
#define USB_CLASS_HID			3
#define USB_CLASS_PHYSICAL		5
#define USB_CLASS_STILL_IMAGE		6
#define USB_CLASS_PRINTER		7
#define USB_CLASS_MASS_STORAGE		8
#define USB_CLASS_HUB			9
#define USB_CLASS_CDC_DATA		0x0a
#define USB_CLASS_CSCID			0x0b	/* chip+ smart card */
#define USB_CLASS_CONTENT_SEC		0x0d	/* content security */
#define USB_CLASS_VIDEO			0x0e
#define USB_CLASS_WIRELESS_CONTROLLER	0xe0
#define USB_CLASS_MISC			0xef
#define USB_CLASS_APP_SPEC		0xfe
#define USB_CLASS_VENDOR_SPEC		0xff

#define USB_SUBCLASS_VENDOR_SPEC	0xff

#define USBH_CONFIGURATION_DESCRIPTOR_SIZE (USB_CONFIGURATION_DESC_SIZE \
+ USB_INTERFACE_DESC_SIZE\
+ (USBH_MAX_NUM_ENDPOINTS * USB_ENDPOINT_DESC_SIZE))


#define CONFIG_DESC_wTOTAL_LENGTH	\
(ConfigurationDescriptorData.ConfigDescfield.\
ConfigurationDescriptor.wTotalLength)

/*  This Union is copied from usb_core.h  */
typedef union
{
	uint16_t w;
	struct BW
	{
		uint8_t msb;
		uint8_t lsb;
	} bw;
}

uint16_t_uint8_t;


typedef union _USB_Setup
{
	uint8_t d8[8];

	struct _SetupPkt_Struc
	{
		uint8_t bmRequestType;
		uint8_t bRequest;
		uint16_t_uint8_t wValue;
		uint16_t_uint8_t wIndex;
		uint16_t_uint8_t wLength;
	} b;
}

USB_Setup_TypeDef;

typedef struct _DescHeader
{
	uint8_t bLength;
	uint8_t bDescriptorType;
} USBH_DescHeader_t;

struct usb_interface_assoc_descriptor
{
	u8 bLength;
	u8 bDescriptorType;

	u8 bFirstInterface;
	u8 bInterfaceCount;
	u8 bFunctionClass;
	u8 bFunctionSubClass;
	u8 bFunctionProtocol;
	u8 iFunction;
} __packed;

struct usb_security_descriptor
{
	u8 bLength;
	u8 bDescriptorType;

	uint16_t wTotalLength;
	u8 bNumEncryptionTypes;
} __packed;


struct usb_ss_ep_comp_descriptor
{
	u8 bLength;
	u8 bDescriptorType;

	u8 bMaxBurst;
	u8 bmAttributes;
	uint16_t wBytesPerInterval;
} __packed;


typedef struct _DeviceDescriptor
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdUSB; /* USB Specification Number which device complies too
	 */
	uint8_t bDeviceClass;
	uint8_t bDeviceSubClass;
	uint8_t bDeviceProtocol;
	/* If equal to Zero, each interface specifies its own class
	code if equal to 0xFF, the class code is vendor specified.
	Otherwise field is valid Class Code.*/
	uint8_t bMaxPacketSize;
	uint16_t idVendor; /* Vendor ID (Assigned by USB Org) */
	uint16_t idProduct; /* Product ID (Assigned by Manufacturer) */
	uint16_t bcdDevice; /* Device Release Number */
	uint8_t iManufacturer; /* Index of Manufacturer String Descriptor */
	uint8_t iProduct; /* Index of Product String Descriptor */
	uint8_t iSerialNumber; /* Index of Serial Number String Descriptor */
	uint8_t bNumConfigurations; /* Number of Possible Configurations */
} USBH_DevDesc_TypeDef;


typedef struct _ConfigurationDescriptor
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wTotalLength; /* Total Length of Data Returned */
	uint8_t bNumInterfaces; /* Number of Interfaces */
	uint8_t bConfigurationValue; /* Value to use as an argument to select
	this configuration*/
	uint8_t iConfiguration; /*Index of String Descriptor Describing this
	configuration */
	uint8_t bmAttributes; /* D7 Bus Powered , D6 Self Powered, D5 Remote
	Wakeup , D4..0 Reserved (0)*/
	uint8_t bMaxPower; /*Maximum Power Consumption */
} USBH_CfgDesc_TypeDef;


typedef struct _HIDDescriptor
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdHID; /* indicates what endpoint this descriptor is
	describing */
	uint8_t bCountryCode; /* specifies the transfer type. */
	uint8_t bNumDescriptors; /* specifies the transfer type. */
	uint8_t bReportDescriptorType; /* Maximum Packet Size this endpoint is
	capable of sending or receiving */
	uint16_t wItemLength; /* is used to specify the polling interval of
	certain transfers. */
} USBH_HIDDesc_TypeDef;


typedef struct _InterfaceDescriptor
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bInterfaceNumber;
	uint8_t bAlternateSetting; /* Value used to select alternative setting
	 */
	uint8_t bNumEndpoints; /* Number of Endpoints used for this interface */
	uint8_t bInterfaceClass; /* Class Code (Assigned by USB Org) */
	uint8_t bInterfaceSubClass; /* Subclass Code (Assigned by USB Org) */
	uint8_t bInterfaceProtocol; /* Protocol Code */
	uint8_t iInterface; /* Index of String Descriptor Describing this
	interface */

} USBH_InterfaceDesc_TypeDef;

#define USBH_StatusTypeDef	USBH_Status
#define USBH_HandleTypeDef	USBH_HOST

typedef struct _EndpointDescriptor
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bEndpointAddress; /* indicates what endpoint this descriptor is
	describing */
	uint8_t bmAttributes; /* specifies the transfer type. */
	uint16_t wMaxPacketSize; /* Maximum Packet Size this endpoint is
	capable of sending or receiving */
	uint8_t bInterval; /* is used to specify the polling interval of
	certain transfers. */
} USBH_EpDesc_TypeDef;

struct usb_endpoint_descriptor
{
	uint8_t bLength;
	uint8_t bDescriptorType;

	uint8_t bEndpointAddress;
	uint8_t bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t bInterval;

	/* NOTE:  these two are _only_ in audio endpoints. */
	/* use USB_DT_ENDPOINT*_SIZE in bLength, not sizeof. */
	uint8_t bRefresh;
	uint8_t bSynchAddress;
} __packed;

struct usb_interface_descriptor
{
	uint8_t bLength;
	uint8_t bDescriptorType;

	uint8_t bInterfaceNumber;
	uint8_t bAlternateSetting;
	uint8_t bNumEndpoints;
	uint8_t bInterfaceClass;
	uint8_t bInterfaceSubClass;
	uint8_t bInterfaceProtocol;
	uint8_t iInterface;
} __packed;


struct usb_iso_packet_descriptor
{
	unsigned int offset;
	unsigned int length; /* expected length */
	unsigned int actual_length;
	int status;
};

struct urb
{
	struct usb_device *dev; /* (in) pointer to associated device */
	struct usb_host_endpoint *ep; /* (internal) pointer to endpoint */
	unsigned int pipe; /* (in) pipe information */
	unsigned int stream_id; /* (in) stream ID */
	int status; /* (return) non-ISO status */
	unsigned int transfer_flags; /* (in) URB_SHORT_NOT_OK | ...*/
	void *transfer_buffer; /* (in) associated data buffer */
	dma_addr_t transfer_dma; /* (in) dma addr for transfer_buffer */
	u32 transfer_buffer_length; /* (in) data buffer length */
	u32 actual_length; /* (return) actual transfer length */
	int start_frame; /* (modify) start frame (ISO) */
	int number_of_packets; /* (in) number of ISO packets */
	int interval; /* (modify) transfer interval	 * (INT/ISO) */
	int error_count; /* (return) number of ISO errors */
	void *context; /* (in) context for completion */
	void *complete; /* (in) completion routine */
	bool complete_flag;
	bool in_request;
	struct usb_iso_packet_descriptor iso_frame_desc[1];
	wait_event_t wait_complete;
};

typedef void(*usb_complete_t)(struct urb*);

#define usb_free_urb(X) 		mem_free(X)
#define usb_alloc_urb(X, Y) 	mem_calloc(1, sizeof(struct urb))

#define USB_DT_ENDPOINT_SIZE		7
#define USB_DT_ENDPOINT_AUDIO_SIZE	9	/* Audio extension */


/*
 * Endpoints
 */
#define USB_ENDPOINT_NUMBER_MASK	0x0f	/* in bEndpointAddress */
#define USB_ENDPOINT_DIR_MASK		0x80

#define USB_ENDPOINT_SYNCTYPE		0x0c
#define USB_ENDPOINT_SYNC_NONE		(0 << 2)
#define USB_ENDPOINT_SYNC_ASYNC		(1 << 2)
#define USB_ENDPOINT_SYNC_ADAPTIVE	(2 << 2)
#define USB_ENDPOINT_SYNC_SYNC		(3 << 2)

#define USB_ENDPOINT_XFERTYPE_MASK	0x03	/* in bmAttributes */
#define USB_ENDPOINT_XFER_CONTROL	0
#define USB_ENDPOINT_XFER_ISOC		1
#define USB_ENDPOINT_XFER_BULK		2
#define USB_ENDPOINT_XFER_INT		3
#define USB_ENDPOINT_MAX_ADJUSTABLE	0x80

/*-------------------------------------------------------------------------*/

/**
 * usb_endpoint_num - get the endpoint's number
 * @epd: endpoint to be checked
 *
 * Returns @epd's number: 0 to 15.
 */
static __inline int usb_endpoint_num(const struct usb_endpoint_descriptor *epd)
{
	return epd->bEndpointAddress &USB_ENDPOINT_NUMBER_MASK;
} 

/**
 * usb_endpoint_type - get the endpoint's transfer type
 * @epd: endpoint to be checked
 *
 * Returns one of USB_ENDPOINT_XFER_{CONTROL, ISOC, BULK, INT} according
 * to @epd's transfer type.
 */
static __inline int usb_endpoint_type(const struct usb_endpoint_descriptor *epd)
{
	return epd->bmAttributes &USB_ENDPOINT_XFERTYPE_MASK;
} 

/**
 * usb_endpoint_dir_in - check if the endpoint has IN direction
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint is of type IN, otherwise it returns false.
 */
static __inline int usb_endpoint_dir_in(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bEndpointAddress &USB_ENDPOINT_DIR_MASK) == USB_DIR_IN);
} 

/**
 * usb_endpoint_dir_out - check if the endpoint has OUT direction
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint is of type OUT, otherwise it returns false.
 */
static __inline int usb_endpoint_dir_out(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bEndpointAddress &USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT);
} 

/**
 * usb_endpoint_xfer_bulk - check if the endpoint has bulk transfer type
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint is of type bulk, otherwise it returns false.
 */
static __inline int usb_endpoint_xfer_bulk(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes &USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK);
} 

/**
 * usb_endpoint_xfer_control - check if the endpoint has control transfer type
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint is of type control, otherwise it returns false.
 */
static __inline int usb_endpoint_xfer_control(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes &USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_CONTROL);
} 

/**
 * usb_endpoint_xfer_int - check if the endpoint has interrupt transfer type
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint is of type interrupt, otherwise it returns
 * false.
 */
static __inline int usb_endpoint_xfer_int(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes &USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT);
} 

/**
 * usb_endpoint_xfer_isoc - check if the endpoint has isochronous transfer type
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint is of type isochronous, otherwise it returns
 * false.
 */
static __inline int usb_endpoint_xfer_isoc(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes &USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_ISOC);
} 

/**
 * usb_endpoint_is_bulk_in - check if the endpoint is bulk IN
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint has bulk transfer type and IN direction,
 * otherwise it returns false.
 */
static __inline int usb_endpoint_is_bulk_in(const struct usb_endpoint_descriptor *epd)
{
	return usb_endpoint_xfer_bulk(epd) && usb_endpoint_dir_in(epd);
} 

/**
 * usb_endpoint_is_bulk_out - check if the endpoint is bulk OUT
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint has bulk transfer type and OUT direction,
 * otherwise it returns false.
 */
static __inline int usb_endpoint_is_bulk_out(const struct usb_endpoint_descriptor *epd)
{
	return usb_endpoint_xfer_bulk(epd) && usb_endpoint_dir_out(epd);
} 

/**
 * usb_endpoint_is_int_in - check if the endpoint is interrupt IN
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint has interrupt transfer type and IN direction,
 * otherwise it returns false.
 */
static __inline int usb_endpoint_is_int_in(const struct usb_endpoint_descriptor *epd)
{
	return usb_endpoint_xfer_int(epd) && usb_endpoint_dir_in(epd);
} 

/**
 * usb_endpoint_is_int_out - check if the endpoint is interrupt OUT
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint has interrupt transfer type and OUT direction,
 * otherwise it returns false.
 */
static __inline int usb_endpoint_is_int_out(const struct usb_endpoint_descriptor *epd)
{
	return usb_endpoint_xfer_int(epd) && usb_endpoint_dir_out(epd);
} 

/**
 * usb_endpoint_is_isoc_in - check if the endpoint is isochronous IN
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint has isochronous transfer type and IN direction,
 * otherwise it returns false.
 */
static __inline int usb_endpoint_is_isoc_in(const struct usb_endpoint_descriptor *epd)
{
	return usb_endpoint_xfer_isoc(epd) && usb_endpoint_dir_in(epd);
} 

/**
 * usb_endpoint_is_isoc_out - check if the endpoint is isochronous OUT
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint has isochronous transfer type and OUT direction,
 * otherwise it returns false.
 */
static __inline int usb_endpoint_is_isoc_out(const struct usb_endpoint_descriptor *epd)
{
	return usb_endpoint_xfer_isoc(epd) && usb_endpoint_dir_out(epd);
} 

#endif 
 /******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
