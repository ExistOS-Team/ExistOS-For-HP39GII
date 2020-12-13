
#include "irq.h"
#include "hw_irq.h"
#include "utils.h"
#include "usb_base_drv.h"
#include "usb_base_drv_config.h"
#include "usb_base_drv_queue.h"
#include "usb_class_driver.h"
#include "usb_storage.h"





unsigned char usb_buffer[4096];
unsigned int usb_core_num_interfaces = 0;

#define MAX_PKT_SIZE        1024
#define MAX_PKT_SIZE_EP0    64

/* USB device mode registers (Little Endian) */
#define REG_ID               (*(volatile unsigned int *)(USB_BASE+0x000))
#define REG_HWGENERAL        (*(volatile unsigned int *)(USB_BASE+0x004))
#define REG_HWHOST           (*(volatile unsigned int *)(USB_BASE+0x008))
#define REG_HWDEVICE         (*(volatile unsigned int *)(USB_BASE+0x00c))
#define REG_TXBUF            (*(volatile unsigned int *)(USB_BASE+0x010))
#define REG_RXBUF            (*(volatile unsigned int *)(USB_BASE+0x014))
#define REG_CAPLENGTH        (*(volatile unsigned char*)(USB_BASE+0x100))
#define REG_DCIVERSION       (*(volatile unsigned int *)(USB_BASE+0x120))
#define REG_DCCPARAMS        (*(volatile unsigned int *)(USB_BASE+0x124))
#define REG_USBCMD           (*(volatile unsigned int *)(USB_BASE+0x140))
#define REG_USBSTS           (*(volatile unsigned int *)(USB_BASE+0x144))
#define REG_USBINTR          (*(volatile unsigned int *)(USB_BASE+0x148))
#define REG_FRINDEX          (*(volatile unsigned int *)(USB_BASE+0x14c))
#define REG_DEVICEADDR       (*(volatile unsigned int *)(USB_BASE+0x154))
#define REG_ENDPOINTLISTADDR (*(volatile unsigned int *)(USB_BASE+0x158))
#define REG_BURSTSIZE        (*(volatile unsigned int *)(USB_BASE+0x160))
#define REG_ULPI             (*(volatile unsigned int *)(USB_BASE+0x170))
#define REG_CONFIGFLAG       (*(volatile unsigned int *)(USB_BASE+0x180))
#define REG_PORTSC1          (*(volatile unsigned int *)(USB_BASE+0x184))
#define REG_OTGSC            (*(volatile unsigned int *)(USB_BASE+0x1a4))
#define REG_USBMODE          (*(volatile unsigned int *)(USB_BASE+0x1a8))
#define REG_ENDPTSETUPSTAT   (*(volatile unsigned int *)(USB_BASE+0x1ac))
#define REG_ENDPTPRIME       (*(volatile unsigned int *)(USB_BASE+0x1b0))
#define REG_ENDPTFLUSH       (*(volatile unsigned int *)(USB_BASE+0x1b4))
#define REG_ENDPTSTATUS      (*(volatile unsigned int *)(USB_BASE+0x1b8))
#define REG_ENDPTCOMPLETE    (*(volatile unsigned int *)(USB_BASE+0x1bc))
#define REG_ENDPTCTRL0       (*(volatile unsigned int *)(USB_BASE+0x1c0))
#define REG_ENDPTCTRL1       (*(volatile unsigned int *)(USB_BASE+0x1c4))
#define REG_ENDPTCTRL2       (*(volatile unsigned int *)(USB_BASE+0x1c8))
#define REG_ENDPTCTRL(_x_)   (*(volatile unsigned int *)(USB_BASE+0x1c0+4*(_x_)))

/* USB CMD  Register Bit Masks */
#define USBCMD_RUN                            (0x00000001)
#define USBCMD_CTRL_RESET                     (0x00000002)
#define USBCMD_PERIODIC_SCHEDULE_EN           (0x00000010)
#define USBCMD_ASYNC_SCHEDULE_EN              (0x00000020)
#define USBCMD_INT_AA_DOORBELL                (0x00000040)
#define USBCMD_ASP                            (0x00000300)
#define USBCMD_ASYNC_SCH_PARK_EN              (0x00000800)
#define USBCMD_SUTW                           (0x00002000)
#define USBCMD_ATDTW                          (0x00004000)
#define USBCMD_ITC                            (0x00FF0000)

/* bit 15,3,2 are frame list size */
#define USBCMD_FRAME_SIZE_1024                (0x00000000)
#define USBCMD_FRAME_SIZE_512                 (0x00000004)
#define USBCMD_FRAME_SIZE_256                 (0x00000008)
#define USBCMD_FRAME_SIZE_128                 (0x0000000C)
#define USBCMD_FRAME_SIZE_64                  (0x00008000)
#define USBCMD_FRAME_SIZE_32                  (0x00008004)
#define USBCMD_FRAME_SIZE_16                  (0x00008008)
#define USBCMD_FRAME_SIZE_8                   (0x0000800C)

/* bit 9-8 are async schedule park mode count */
#define USBCMD_ASP_00                         (0x00000000)
#define USBCMD_ASP_01                         (0x00000100)
#define USBCMD_ASP_10                         (0x00000200)
#define USBCMD_ASP_11                         (0x00000300)
#define USBCMD_ASP_BIT_POS                    (8)

/* bit 23-16 are interrupt threshold control */
#define USBCMD_ITC_NO_THRESHOLD               (0x00000000)
#define USBCMD_ITC_1_MICRO_FRM                (0x00010000)
#define USBCMD_ITC_2_MICRO_FRM                (0x00020000)
#define USBCMD_ITC_4_MICRO_FRM                (0x00040000)
#define USBCMD_ITC_8_MICRO_FRM                (0x00080000)
#define USBCMD_ITC_16_MICRO_FRM               (0x00100000)
#define USBCMD_ITC_32_MICRO_FRM               (0x00200000)
#define USBCMD_ITC_64_MICRO_FRM               (0x00400000)
#define USBCMD_ITC_BIT_POS                    (16)

/* USB STS Register Bit Masks */
#define USBSTS_INT                            (0x00000001)
#define USBSTS_ERR                            (0x00000002)
#define USBSTS_PORT_CHANGE                    (0x00000004)
#define USBSTS_FRM_LST_ROLL                   (0x00000008)
#define USBSTS_SYS_ERR                        (0x00000010) /* not used */
#define USBSTS_IAA                            (0x00000020)
#define USBSTS_RESET                          (0x00000040)
#define USBSTS_SOF                            (0x00000080)
#define USBSTS_SUSPEND                        (0x00000100)
#define USBSTS_HC_HALTED                      (0x00001000)
#define USBSTS_RCL                            (0x00002000)
#define USBSTS_PERIODIC_SCHEDULE              (0x00004000)
#define USBSTS_ASYNC_SCHEDULE                 (0x00008000)

/* USB INTR Register Bit Masks */
#define USBINTR_INT_EN                        (0x00000001)
#define USBINTR_ERR_INT_EN                    (0x00000002)
#define USBINTR_PTC_DETECT_EN                 (0x00000004)
#define USBINTR_FRM_LST_ROLL_EN               (0x00000008)
#define USBINTR_SYS_ERR_EN                    (0x00000010)
#define USBINTR_ASYN_ADV_EN                   (0x00000020)
#define USBINTR_RESET_EN                      (0x00000040)
#define USBINTR_SOF_EN                        (0x00000080)
#define USBINTR_DEVICE_SUSPEND                (0x00000100)

/* ULPI Register Bit Masks */
#define ULPI_ULPIWU                           (0x80000000)
#define ULPI_ULPIRUN                          (0x40000000)
#define ULPI_ULPIRW                           (0x20000000)
#define ULPI_ULPISS                           (0x08000000)
#define ULPI_ULPIPORT                         (0x07000000)
#define ULPI_ULPIADDR                         (0x00FF0000)
#define ULPI_ULPIDATRD                        (0x0000FF00)
#define ULPI_ULPIDATWR                        (0x000000FF)
 

/* Device Address bit masks */
#define USBDEVICEADDRESS_MASK                 (0xFE000000)
#define USBDEVICEADDRESS_BIT_POS              (25)

/* Endpoint Setup Status bit masks */
#define EPSETUP_STATUS_EP0                    (0x00000001)

/* PORTSCX  Register Bit Masks */
#define PORTSCX_CURRENT_CONNECT_STATUS         (0x00000001)
#define PORTSCX_CONNECT_STATUS_CHANGE          (0x00000002)
#define PORTSCX_PORT_ENABLE                    (0x00000004)
#define PORTSCX_PORT_EN_DIS_CHANGE             (0x00000008)
#define PORTSCX_OVER_CURRENT_ACT               (0x00000010)
#define PORTSCX_OVER_CURRENT_CHG               (0x00000020)
#define PORTSCX_PORT_FORCE_RESUME              (0x00000040)
#define PORTSCX_PORT_SUSPEND                   (0x00000080)
#define PORTSCX_PORT_RESET                     (0x00000100)
#define PORTSCX_LINE_STATUS_BITS               (0x00000C00)
#define PORTSCX_PORT_POWER                     (0x00001000)
#define PORTSCX_PORT_INDICTOR_CTRL             (0x0000C000)
#define PORTSCX_PORT_TEST_CTRL                 (0x000F0000)
#define PORTSCX_WAKE_ON_CONNECT_EN             (0x00100000)
#define PORTSCX_WAKE_ON_CONNECT_DIS            (0x00200000)
#define PORTSCX_WAKE_ON_OVER_CURRENT           (0x00400000)
#define PORTSCX_PHY_LOW_POWER_SPD              (0x00800000)
#define PORTSCX_PORT_FORCE_FULL_SPEED          (0x01000000)
#define PORTSCX_PORT_SPEED_MASK                (0x0C000000)
#define PORTSCX_PORT_WIDTH                     (0x10000000)
#define PORTSCX_PHY_TYPE_SEL                   (0xC0000000)

/* bit 11-10 are line status */
#define PORTSCX_LINE_STATUS_SE0                (0x00000000)
#define PORTSCX_LINE_STATUS_JSTATE             (0x00000400)
#define PORTSCX_LINE_STATUS_KSTATE             (0x00000800)
#define PORTSCX_LINE_STATUS_UNDEF              (0x00000C00)
#define PORTSCX_LINE_STATUS_BIT_POS            (10)

/* bit 15-14 are port indicator control */
#define PORTSCX_PIC_OFF                        (0x00000000)
#define PORTSCX_PIC_AMBER                      (0x00004000)
#define PORTSCX_PIC_GREEN                      (0x00008000)
#define PORTSCX_PIC_UNDEF                      (0x0000C000)
#define PORTSCX_PIC_BIT_POS                    (14)

/* bit 19-16 are port test control */
#define PORTSCX_PTC_DISABLE                    (0x00000000)
#define PORTSCX_PTC_JSTATE                     (0x00010000)
#define PORTSCX_PTC_KSTATE                     (0x00020000)
#define PORTSCX_PTC_SE0NAK                     (0x00030000)
#define PORTSCX_PTC_PACKET                     (0x00040000)
#define PORTSCX_PTC_FORCE_EN                   (0x00050000)
#define PORTSCX_PTC_BIT_POS                    (16)

/* bit 27-26 are port speed */
#define PORTSCX_PORT_SPEED_FULL                (0x00000000)
#define PORTSCX_PORT_SPEED_LOW                 (0x04000000)
#define PORTSCX_PORT_SPEED_HIGH                (0x08000000)
#define PORTSCX_PORT_SPEED_UNDEF               (0x0C000000)
#define PORTSCX_SPEED_BIT_POS                  (26)

/* bit 28 is parallel transceiver width for UTMI interface */
#define PORTSCX_PTW                            (0x10000000)
#define PORTSCX_PTW_8BIT                       (0x00000000)
#define PORTSCX_PTW_16BIT                      (0x10000000)

/* bit 31-30 are port transceiver select */
#define PORTSCX_PTS_UTMI                       (0x00000000)
#define PORTSCX_PTS_CLASSIC                    (0x40000000)
#define PORTSCX_PTS_ULPI                       (0x80000000)
#define PORTSCX_PTS_FSLS                       (0xC0000000)
#define PORTSCX_PTS_BIT_POS                    (30)

/* USB MODE Register Bit Masks */
#define USBMODE_CTRL_MODE_IDLE                (0x00000000)
#define USBMODE_CTRL_MODE_DEVICE              (0x00000002)
#define USBMODE_CTRL_MODE_HOST                (0x00000003)
#define USBMODE_CTRL_MODE_RSV                 (0x00000001)
#define USBMODE_SETUP_LOCK_OFF                (0x00000008)
#define USBMODE_STREAM_DISABLE                (0x00000010)

/* ENDPOINTCTRLx  Register Bit Masks */
#define EPCTRL_TX_ENABLE                       (0x00800000)
#define EPCTRL_TX_DATA_TOGGLE_RST              (0x00400000)    /* Not EP0 */
#define EPCTRL_TX_DATA_TOGGLE_INH              (0x00200000)    /* Not EP0 */
#define EPCTRL_TX_TYPE                         (0x000C0000)
#define EPCTRL_TX_DATA_SOURCE                  (0x00020000)    /* Not EP0 */
#define EPCTRL_TX_EP_STALL                     (0x00010000)
#define EPCTRL_RX_ENABLE                       (0x00000080)
#define EPCTRL_RX_DATA_TOGGLE_RST              (0x00000040)    /* Not EP0 */
#define EPCTRL_RX_DATA_TOGGLE_INH              (0x00000020)    /* Not EP0 */
#define EPCTRL_RX_TYPE                         (0x0000000C)
#define EPCTRL_RX_DATA_SINK                    (0x00000002)    /* Not EP0 */
#define EPCTRL_RX_EP_STALL                     (0x00000001)

/* bit 19-18 and 3-2 are endpoint type */
#define EPCTRL_TX_EP_TYPE_SHIFT                (18)
#define EPCTRL_RX_EP_TYPE_SHIFT                (2)

#define QH_MULT_POS                            (30)
#define QH_ZLT_SEL                             (0x20000000)
#define QH_MAX_PKT_LEN_POS                     (16)
#define QH_IOS                                 (0x00008000)
#define QH_NEXT_TERMINATE                      (0x00000001)
#define QH_IOC                                 (0x00008000)
#define QH_MULTO                               (0x00000C00)
#define QH_STATUS_HALT                         (0x00000040)
#define QH_STATUS_ACTIVE                       (0x00000080)
#define EP_QUEUE_CURRENT_OFFSET_MASK         (0x00000FFF)
#define EP_QUEUE_HEAD_NEXT_POINTER_MASK      (0xFFFFFFE0)
#define EP_QUEUE_FRINDEX_MASK                (0x000007FF)
#define EP_MAX_LENGTH_TRANSFER               (0x4000)

#define DTD_NEXT_TERMINATE                   (0x00000001)
#define DTD_IOC                              (0x00008000)
#define DTD_STATUS_ACTIVE                    (0x00000080)
#define DTD_STATUS_HALTED                    (0x00000040)
#define DTD_STATUS_DATA_BUFF_ERR             (0x00000020)
#define DTD_STATUS_TRANSACTION_ERR           (0x00000008)
#define DTD_RESERVED_FIELDS                  (0x80007300)
#define DTD_ADDR_MASK                        (0xFFFFFFE0)
#define DTD_PACKET_SIZE                      (0x7FFF0000)
#define DTD_LENGTH_BIT_POS                   (16)
#define DTD_ERROR_MASK                       (DTD_STATUS_HALTED | \
                                               DTD_STATUS_DATA_BUFF_ERR | \
                                               DTD_STATUS_TRANSACTION_ERR)

#define DTD_RESERVED_LENGTH_MASK             0x0001ffff
#define DTD_RESERVED_IN_USE                  0x80000000
#define DTD_RESERVED_PIPE_MASK               0x0ff00000
#define DTD_RESERVED_PIPE_OFFSET             20

#define MAX(x,y) (x > y ? x : y) 
#define MIN(x,y) (x < y ? x : y)
/*-------------------------------------------------------------------------*/
/* 4 transfer descriptors per endpoint allow 64k transfers, which is the usual MSC
   transfer size, so it seems like a good size */
#define NUM_TDS_PER_EP 4


static struct usb_class_driver drivers[USB_NUM_DRIVERS] =
{
	#ifdef USB_ENABLE_STORAGE
    [USB_DRIVER_MASS_STORAGE] = {
        .enabled = false,
        .needs_exclusive_storage = true,
        .first_interface = 0,
        .last_interface = 0,
        .request_endpoints = usb_storage_request_endpoints,
        .set_first_interface = usb_storage_set_first_interface,
        .get_config_descriptor = usb_storage_get_config_descriptor,
        .init_connection = usb_storage_init_connection,
        .init = usb_storage_init,
        .disconnect = usb_storage_disconnect,
        .transfer_complete = usb_storage_transfer_complete,
        .control_request = usb_storage_control_request,
#ifdef HAVE_HOTSWAP
        .notify_hotswap = usb_storage_notify_hotswap,
#endif
    },
#endif
};


typedef struct usb_endpoint
{
    bool allocated[2];
    short type[2];
    short max_pkt_size[2];
} usb_endpoint_t;
static usb_endpoint_t endpoints[USB_NUM_ENDPOINTS];


static struct usb_device_descriptor device_descriptor=
{
    .bLength            = sizeof(struct usb_device_descriptor),
    .bDescriptorType    = USB_DT_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = USB_CLASS_PER_INTERFACE,
    .bDeviceSubClass    = 0,
    .bDeviceProtocol    = 0,
    .bMaxPacketSize0    = 64,
    .idVendor           = 0x2333,
    .idProduct          = 0x6666,
    .bcdDevice          = 2 << 8 | 3,
    .iManufacturer      = 1,
    .iProduct           = 2,
    .iSerialNumber      = 0,
    .bNumConfigurations = 1
};

static struct usb_config_descriptor __attribute__((aligned(2)))
                                    config_descriptor =
{
    .bLength             = sizeof(struct usb_config_descriptor),
    .bDescriptorType     = USB_DT_CONFIG,
    .wTotalLength        = 0, /* will be filled in later */
    .bNumInterfaces      = 1,
    .bConfigurationValue = 1,
    .iConfiguration      = 0,
    .bmAttributes        = USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
    .bMaxPower           = (USB_MAX_CURRENT + 1) / 2, /* In 2mA units */
};

static const struct usb_string_descriptor __attribute__((aligned(2)))
                                    usb_string_iManufacturer =
{
    0x14,
    USB_DT_STRING,
    {0x77,0x00,0x77,0x00,0x77,0x00,0x2E,0x00,0x68,0x00,0x70,0x00,0x2E,0x00,
	0x63,0x00,0x6E,0x00} 
};

static const struct usb_string_descriptor __attribute__((aligned(2)))
                                    usb_string_iProduct =
{
    0x1C,
	USB_DT_STRING,
	{0xBB,0x50,0x3C,0x90,0x68,0x00,0x70,0x00,0x20,0x00,0x33,0x00,0x39,0x00,
	0x67,0x00,0x69,0x00,0x69,0x00,0xA1,0x8B,0x97,0x7B,0x68,0x56}

};

static struct usb_string_descriptor __attribute__((aligned(2)))
                                    usb_string_iSerial =
{
    84,
    USB_DT_STRING,
    {'0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
     '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
     '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
     '0', '0', '0', '0', '0', '0', '0', '0'}
};

/* Generic for all targets */

/* this is stringid #0: languages supported */
static const struct usb_string_descriptor __attribute__((aligned(2)))
                                    lang_descriptor =
{
    4,
    USB_DT_STRING,
    {0x04,0x08}
    //{0x09,0x04} /* LANGID US English */
};

static const struct usb_string_descriptor* const usb_strings[] =
{
   &lang_descriptor,
   &usb_string_iManufacturer,
   &usb_string_iProduct,
   &usb_string_iSerial
};


static const struct usb_qualifier_descriptor __attribute__((aligned(2)))
                                             qualifier_descriptor =
{
    .bLength            = sizeof(struct usb_qualifier_descriptor),
    .bDescriptorType    = USB_DT_DEVICE_QUALIFIER,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0,
    .bDeviceSubClass    = 0,
    .bDeviceProtocol    = 0,
    .bMaxPacketSize0    = 64,
    .bNumConfigurations = 1
};


struct transfer_descriptor {
    unsigned int next_td_ptr;           /* Next TD pointer(31-5), T(0) set
                                           indicate invalid */
    unsigned int size_ioc_sts;          /* Total bytes (30-16), IOC (15),
                                           MultO(11-10), STS (7-0)  */
    unsigned int buff_ptr0;             /* Buffer pointer Page 0 */
    unsigned int buff_ptr1;             /* Buffer pointer Page 1 */
    unsigned int buff_ptr2;             /* Buffer pointer Page 2 */
    unsigned int buff_ptr3;             /* Buffer pointer Page 3 */
    unsigned int buff_ptr4;             /* Buffer pointer Page 4 */
    unsigned int reserved;
} __attribute__ ((packed));

static struct transfer_descriptor td_array[USB_NUM_ENDPOINTS*2*NUM_TDS_PER_EP]
    __attribute__((aligned(32)));
	
struct queue_head {
    unsigned int max_pkt_length;    /* Mult(31-30) , Zlt(29) , Max Pkt len
                                       and IOS(15) */
    unsigned int curr_dtd_ptr;      /* Current dTD Pointer(31-5) */
    struct transfer_descriptor dtd; /* dTD overlay */
    unsigned int setup_buffer[2];   /* Setup data 8 bytes */
    unsigned int reserved;          /* for software use, pointer to the first TD */
    unsigned int status;            /* for software use, status of chain in progress */
    unsigned int length;            /* for software use, transfered bytes of chain in progress */
    unsigned int wait;              /* for softwate use, indicates if the transfer is blocking */
} __attribute__((packed));

static struct queue_head qh_array[USB_NUM_ENDPOINTS*2] __attribute__((aligned(2048)));
	
static const unsigned int pipe2mask[] = {
    0x01, 0x010000,
    0x02, 0x020000,
    0x04, 0x040000,
    0x08, 0x080000,
    0x10, 0x100000,
};


int usb_drv_port_speed(void)
{
    return (REG_PORTSC1 & 0x08000000) ? 1 : 0;
}

bool usb_drv_stalled(int endpoint,bool in)
{
    if(in) {
        return ((REG_ENDPTCTRL(EP_NUM(endpoint)) & EPCTRL_TX_EP_STALL)!=0);
    }
    else {
        return ((REG_ENDPTCTRL(EP_NUM(endpoint)) & EPCTRL_RX_EP_STALL)!=0);
    }

}
void usb_drv_stall(int endpoint, bool stall, bool in)
{
    int ep_num = EP_NUM(endpoint);

    printf("%sstall %d", stall ? "" : "un", ep_num);

    if(in) {
        if (stall) {
            REG_ENDPTCTRL(ep_num) |= EPCTRL_TX_EP_STALL;
        }
        else {
            REG_ENDPTCTRL(ep_num) &= ~EPCTRL_TX_EP_STALL;
        }
    }
    else {
        if (stall) {
            REG_ENDPTCTRL(ep_num) |= EPCTRL_RX_EP_STALL;
        }
        else {
            REG_ENDPTCTRL(ep_num) &= ~EPCTRL_RX_EP_STALL;
        }
    }
}

static void prepare_td(struct transfer_descriptor* td,
                       struct transfer_descriptor* previous_td,
                       void *ptr, int len,int pipe)
{
    //logf("adding a td : %d",len);
    /* FIXME td allow iso packets per frame override but we don't use it here */
    memset(td, 0, sizeof(struct transfer_descriptor));
    td->next_td_ptr = DTD_NEXT_TERMINATE;
    td->size_ioc_sts = (len<< DTD_LENGTH_BIT_POS) |
        DTD_STATUS_ACTIVE | DTD_IOC;
    td->buff_ptr0 = (unsigned int)ptr;
    td->buff_ptr1 = ((unsigned int)ptr & 0xfffff000) + 0x1000;
    td->buff_ptr2 = ((unsigned int)ptr & 0xfffff000) + 0x2000;
    td->buff_ptr3 = ((unsigned int)ptr & 0xfffff000) + 0x3000;
    td->buff_ptr4 = ((unsigned int)ptr & 0xfffff000) + 0x4000;
    td->reserved |= DTD_RESERVED_LENGTH_MASK & len;
    td->reserved |= DTD_RESERVED_IN_USE;
    td->reserved |= (pipe << DTD_RESERVED_PIPE_OFFSET);

    if (previous_td != 0) {
        previous_td->next_td_ptr=(unsigned int)td;
        previous_td->size_ioc_sts&=~DTD_IOC;
    }
}


static int prime_transfer(int ep_num, void* ptr, int len, bool send, bool wait)
{
    int rc = 0;
    int pipe = ep_num * 2 + (send ? 1 : 0);
    unsigned int mask = pipe2mask[pipe];
    struct queue_head* qh = &qh_array[pipe];
    static long last_tick;
    struct transfer_descriptor *new_td, *cur_td, *prev_td;

	//irq_set_enable(HW_IRQ_USB_CTRL,0);
    //int oldlevel = disable_irq_save();
/*
    if (send && ep_num > EP_CONTROL) {
        logf("usb: sent %d bytes", len);
    }
*/
    qh->status = 0;
    qh->wait = wait;

    new_td=&td_array[pipe*NUM_TDS_PER_EP];
    cur_td=new_td;
    prev_td=0;
    int tdlen;

    do
    {
        tdlen=MIN(len,16384);
        prepare_td(cur_td, prev_td, ptr, tdlen,pipe);
        ptr+=tdlen;
        prev_td=cur_td;
        cur_td++;
        len-=tdlen;
    }
    while(len>0);
    //logf("starting ep %d %s",ep_num,send?"send":"receive");

    qh->dtd.next_td_ptr = (unsigned int)new_td;
    qh->dtd.size_ioc_sts &= ~(QH_STATUS_HALT | QH_STATUS_ACTIVE);

    REG_ENDPTPRIME |= mask;

    if(ep_num == EP_CONTROL && (REG_ENDPTSETUPSTAT & EPSETUP_STATUS_EP0)) {
        /* 32.14.3.2.2 */
        printf("new setup arrived\n");
        rc = -4;
        goto pt_error;
    }

    //last_tick = current_tick;
	unsigned int timeout_cnt = 2000*10;
    while ((REG_ENDPTPRIME & mask)) {
        if (REG_USBSTS & USBSTS_RESET) {
            rc = -1;
            goto pt_error;
        }
		
		if(timeout_cnt != 0){
			timeout_cnt--;
			delay_us(100);
		}else{
			printf("prime timeout\n");
            rc = -2;
            goto pt_error;
		}
		/*
        if (TIME_AFTER(current_tick, last_tick + HZ/4)) {
            printf("prime timeout");
            rc = -2;
            goto pt_error;
        }*/
		
		
    }

    if (!(REG_ENDPTSTATUS & mask)) {
        if(REG_ENDPTCOMPLETE & mask)
        {
            printf("endpoint completed fast! %d %d %x\n", ep_num, pipe, qh->dtd.size_ioc_sts & 0xff);
        }
        else
        {
            printf("no prime! %d %d %x\n", ep_num, pipe, qh->dtd.size_ioc_sts & 0xff);
            rc = -3;
            goto pt_error;
        }
    }
    if(ep_num == EP_CONTROL && (REG_ENDPTSETUPSTAT & EPSETUP_STATUS_EP0)) {
        /* 32.14.3.2.2 */
        printf("new setup arrived\n");
        rc = -4;
        goto pt_error;
    }

	//irq_set_enable(HW_IRQ_USB_CTRL,1);
    //restore_irq(oldlevel);
	
    if (wait) {
        /* wait for transfer to finish */
        //semaphore_wait(&transfer_completion_signal[pipe], TIMEOUT_BLOCK);
        
		if(qh->status!=0) {
            /* No need to cancel wait here since it was done and the signal
             * came. */
           // return -5;
        }
        //logf("all tds done");
    }

pt_error:
    //if(rc<0)
     //   restore_irq(oldlevel);

    /* Error status must make sure an abandoned wakeup signal isn't left */
    if (rc < 0 && wait) {
        /* Cancel wait */
        qh->wait = 0;
        /* Make sure to remove any signal if interrupt fired before we zeroed
         * qh->wait. Could happen during a bus reset for example. */
        //semaphore_wait(&transfer_completion_signal[pipe], TIMEOUT_NOBLOCK);
    }

    return rc;
}

int usb_drv_send(int endpoint, void* ptr, int length)
{
    return prime_transfer(EP_NUM(endpoint), ptr, length, true, true);
}

int usb_drv_recv(int endpoint, void* ptr, int length)
{
    return prime_transfer(EP_NUM(endpoint), ptr, length, false, true);
}

/* manual: 32.14.4.1 Queue Head Initialization */
static void init_queue_heads(void)
{
    /* FIXME the packetsize for isochronous transfers is 1023 : 1024 but
     * the current code only support one type of packet size so we restrict
     * isochronous packet size for now also */
    int packetsize = (usb_drv_port_speed() ? 512 : 64);
    int i;

    /* TODO: this should take ep_allocation into account */
    for (i=1;i<USB_NUM_ENDPOINTS;i++) {

        /* OUT */
        if(endpoints[i].type[DIR_OUT] == USB_ENDPOINT_XFER_ISOC)
            /* FIXME: we can adjust the number of packets per frame, currently use one */
            qh_array[i*2].max_pkt_length = packetsize << QH_MAX_PKT_LEN_POS | QH_ZLT_SEL | 1 << QH_MULT_POS;
        else
            qh_array[i*2].max_pkt_length = packetsize << QH_MAX_PKT_LEN_POS | QH_ZLT_SEL;

        qh_array[i*2].dtd.next_td_ptr = QH_NEXT_TERMINATE;

        /* IN */
        if(endpoints[i].type[DIR_IN] == USB_ENDPOINT_XFER_ISOC)
            /* FIXME: we can adjust the number of packets per frame, currently use one */
            qh_array[i*2+1].max_pkt_length = packetsize << QH_MAX_PKT_LEN_POS | QH_ZLT_SEL | 1 << QH_MULT_POS;
        else
            qh_array[i*2+1].max_pkt_length = packetsize << QH_MAX_PKT_LEN_POS | QH_ZLT_SEL;

        qh_array[i*2+1].dtd.next_td_ptr = QH_NEXT_TERMINATE;
    }
}

static void init_endpoints(void)
{
    int ep_num;

    printf("init_endpoints\n");
    /* RX/TX from the device POV: OUT/IN, respectively */
    for(ep_num=1;ep_num<USB_NUM_ENDPOINTS;ep_num++) {
        usb_endpoint_t *endpoint = &endpoints[ep_num];

        /* manual: 32.9.5.18 (Caution): Leaving an unconfigured endpoint control
         * will cause undefined behavior for the data pid tracking on the active
         * endpoint/direction. */
        if (!endpoint->allocated[DIR_OUT])
            endpoint->type[DIR_OUT] = USB_ENDPOINT_XFER_BULK;
        if (!endpoint->allocated[DIR_IN])
            endpoint->type[DIR_IN] = USB_ENDPOINT_XFER_BULK;

        REG_ENDPTCTRL(ep_num) =
            EPCTRL_RX_DATA_TOGGLE_RST | EPCTRL_RX_ENABLE |
            EPCTRL_TX_DATA_TOGGLE_RST | EPCTRL_TX_ENABLE |
            (endpoint->type[DIR_OUT] << EPCTRL_RX_EP_TYPE_SHIFT) |
            (endpoint->type[DIR_IN] << EPCTRL_TX_EP_TYPE_SHIFT);
    }
}


void usb_drv_set_address(int address)
{
    REG_DEVICEADDR = address << USBDEVICEADDRESS_BIT_POS;
    init_queue_heads();
    init_endpoints();
}

static void handle_std_dev_desc(struct usb_ctrlrequest *req)
{
    int size;
    void* ptr = NULL;
    int length = req->wLength;
    int index = req->wValue & 0xff;
	printf("dt type:%x\n",req->wValue >> 8);
	
    switch(req->wValue >> 8)
    {
        case USB_DT_DEVICE:
            ptr = &device_descriptor;
            size = sizeof(struct usb_device_descriptor);
            break;
        case USB_DT_OTHER_SPEED_CONFIG:
        case USB_DT_CONFIG: {
                int i, max_packet_size;

                if(req->wValue>>8==USB_DT_CONFIG) {
                    max_packet_size = (usb_drv_port_speed() ? 512 : 64);
                    config_descriptor.bDescriptorType = USB_DT_CONFIG;
                }
                else {
                    max_packet_size=(usb_drv_port_speed() ? 64 : 512);
                    config_descriptor.bDescriptorType =
                        USB_DT_OTHER_SPEED_CONFIG;
                }
//#ifdef HAVE_USB_CHARGING_ENABLE
//                if (usb_charging_mode == USB_CHARGING_DISABLE) {
                    //config_descriptor.bMaxPower = (100+1)/2;
                    //usb_charging_current_requested = 100;
//                }
//               else {
                    config_descriptor.bMaxPower = (500+1)/2;
//                    usb_charging_current_requested = 500;
//                }
//#endif
                size = sizeof(struct usb_config_descriptor);
/*
                for(i = 0; i < USB_NUM_DRIVERS; i++)
                    if(drivers[i].enabled && drivers[i].get_config_descriptor)
                        size += drivers[i].get_config_descriptor(
                                    &response_data[size], max_packet_size);
*/
                config_descriptor.bNumInterfaces = usb_core_num_interfaces;
                config_descriptor.wTotalLength = (uint16_t)size;
                memcpy(&usb_buffer[0], &config_descriptor,
                        sizeof(struct usb_config_descriptor));

                ptr = usb_buffer;
                break;
            }

        case USB_DT_STRING:
            printf("STRING %d\n", index);
            if((unsigned)index < (sizeof(usb_strings) /
                        sizeof(struct usb_string_descriptor*))) {
                size = usb_strings[index]->bLength;
                ptr = usb_strings[index];
            }
            else if(index == 0xee) {
                /* We don't have a real OS descriptor, and we don't handle
                 * STALL correctly on some devices, so we return any valid
                 * string (we arbitrarily pick the manufacturer name)
                 */
                size = usb_string_iManufacturer.bLength;
                ptr = &usb_string_iManufacturer;
            }
            else {
                printf("bad string id %d\n", index);
                usb_drv_stall(EP_CONTROL, true, true);
            }
            break;

        case USB_DT_DEVICE_QUALIFIER:
            ptr = &qualifier_descriptor;
            size = sizeof(struct usb_qualifier_descriptor);
            break;

        default:
            printf("ctrl desc.\n");
            //control_request_handler_drivers(req);
            break;			
			
			
			
			
    }

    if(ptr)
    {
        length = MIN(size, length);

        if(ptr != usb_buffer)
            memcpy(usb_buffer, ptr, length);
		
		usb_drv_recv(EP_CONTROL, NULL, 0);
        usb_drv_send(EP_CONTROL, usb_buffer, length);
        
    }
    else
        usb_drv_stall(EP_CONTROL, true, true);
}

static void handle_std_dev_req(struct usb_ctrlrequest *req)
{
	printf("std req:%x\n",req->bRequest);
	switch(req->bRequest)
    {
        case USB_REQ_GET_CONFIGURATION:
            usb_buffer[0] = 1;
            usb_drv_send(EP_CONTROL, usb_buffer, 1);
            usb_drv_recv(EP_CONTROL, NULL, 0);
            break;
        case USB_REQ_SET_CONFIGURATION:
            usb_drv_send(EP_CONTROL, NULL, 0);
            break;
        case USB_REQ_GET_DESCRIPTOR:
            handle_std_dev_desc(req);
            break;
        case USB_REQ_SET_ADDRESS:
            usb_drv_send(EP_CONTROL, NULL, 0);
            usb_drv_set_address(req->wValue);
            break;
        case USB_REQ_GET_STATUS:
            usb_buffer[0] = 0;
            usb_buffer[1] = 0;
            usb_drv_send(EP_CONTROL, usb_buffer, 2);
            usb_drv_recv(EP_CONTROL, NULL, 0);
            break;
        default:
            usb_drv_stall(EP_CONTROL, true, true);
    }
	
	
}

static void handle_std_intf_req(struct usb_ctrlrequest *req)
{
	

	/*
	switch(req->bRequest)
    {
		case USB_REQ_GET_CONFIGURATION:
		
		break;
		
	}*/
	
	
}

static void handle_std_req(struct usb_ctrlrequest *req)
{
    switch(req->bRequestType & USB_RECIP_MASK)
    {
        case USB_RECIP_DEVICE:
            return handle_std_dev_req(req);
        case USB_RECIP_INTERFACE:
            return handle_std_intf_req(req);
        default:
            usb_drv_stall(EP_CONTROL, true, true);
    }
}

static void control_received(void)
{
    int i;
    /* copy setup data from packet */
    static unsigned int tmp[2];
    tmp[0] = qh_array[0].setup_buffer[0];
    tmp[1] = qh_array[0].setup_buffer[1];

    /* acknowledge packet recieved */
    REG_ENDPTSETUPSTAT = EPSETUP_STATUS_EP0;
	
    /* Stop pending control transfers */
    for(i=0;i<2;i++) {
        if(qh_array[i].wait) {
            qh_array[i].wait=0;
            qh_array[i].status=DTD_STATUS_HALTED;
        }
    }
	
	//usb_base_drv_queue_ctrl_req_join( (struct usb_ctrlrequest*)tmp );

}

void usb_drv_cancel_all_transfers(void)
{
    int i;
    REG_ENDPTFLUSH = ~0;
    while (REG_ENDPTFLUSH);

    memset(td_array, 0, sizeof td_array);
    for(i=0;i<USB_NUM_ENDPOINTS*2;i++) {
        if(qh_array[i].wait) {
            qh_array[i].wait=0;
            qh_array[i].status=DTD_STATUS_HALTED;
            //semaphore_release(&transfer_completion_signal[i]);
        }
    }
}

static void bus_reset(void)
{
    int i;
    printf("usb bus_reset\n");

    REG_DEVICEADDR = 0;
    REG_ENDPTSETUPSTAT = REG_ENDPTSETUPSTAT;
    REG_ENDPTCOMPLETE  = REG_ENDPTCOMPLETE;
	

    for (i=0; i<100; i++) {
        if (!REG_ENDPTPRIME)
            break;

        if (REG_USBSTS & USBSTS_RESET) {
            printf("usb: double reset\n");
            return;
        }

        delay_us(100);
    }
    if (REG_ENDPTPRIME) {
        printf("usb: short reset timeout\n");
    }

    usb_drv_cancel_all_transfers();

    if (!(REG_PORTSC1 & PORTSCX_PORT_RESET)) {
        printf("usb: slow reset!\n");
    }
}

static void usb_drv_reset(void)
{
	REG_USBCMD &= ~USBCMD_RUN;
	//REG_PORTSC1 = (REG_PORTSC1 & ~PORTSCX_PHY_TYPE_SEL) | USB_PORTSCX_PHY_TYPE;
	delay_us(500*1000);						//等待一段时间使主机释放该USB设备
    REG_USBCMD |= USBCMD_CTRL_RESET;
    while (REG_USBCMD & USBCMD_CTRL_RESET);
	
}


void usb_irq_service(){
	//printf("USB IRQ\n");
	unsigned int usbintr = REG_USBINTR; /* Only watch enabled ints */
    unsigned int status = REG_USBSTS & usbintr;
	
	/* usb transaction interrupt */
	if (status & USBSTS_INT) {
        REG_USBSTS = USBSTS_INT;
		 /* a control packet? */
        if (REG_ENDPTSETUPSTAT & EPSETUP_STATUS_EP0) {
			//EP0为USB控制端点，传输控制命令
			control_received();
        }

        if (REG_ENDPTCOMPLETE){
			
			
		}
            //transfer_completed();
		
	}
	
	/* error interrupt */
	if (status & USBSTS_ERR) {
        REG_USBSTS = USBSTS_ERR;
        printf("usb error int \n");
    }
	
	/* reset interrupt */
    if (status & USBSTS_RESET) {
        REG_USBSTS = USBSTS_RESET;
        bus_reset();
        //usb_core_bus_reset(); /* tell mom */
    }
	
	/* port change */
    if (status & USBSTS_PORT_CHANGE) {
        REG_USBSTS = USBSTS_PORT_CHANGE;
    }

	irq_set_enable(HW_IRQ_USB_CTRL,1);
}

/* manual: 32.14.4.1 Queue Head Initialization */
static void init_control_queue_heads(void)
{
    memset(qh_array, 0, sizeof qh_array);

    /*** control ***/
    qh_array[EP_CONTROL].max_pkt_length = 64 << QH_MAX_PKT_LEN_POS | QH_IOS;
    qh_array[EP_CONTROL].dtd.next_td_ptr = QH_NEXT_TERMINATE;
    qh_array[EP_CONTROL+1].max_pkt_length = 64 << QH_MAX_PKT_LEN_POS;
    qh_array[EP_CONTROL+1].dtd.next_td_ptr = QH_NEXT_TERMINATE;
}

void usb_drv_init(){
	
	irq_install_service(HW_IRQ_USB_CTRL,(void *)usb_irq_service);
	irq_set_enable(HW_IRQ_USB_CTRL,1);
	
	
	usb_drv_reset();
	REG_USBMODE = USBMODE_CTRL_MODE_DEVICE;	//USB控制器切换为设备模式
	REG_DEVICEADDR = 0;
	
	init_control_queue_heads();
	
	REG_ENDPOINTLISTADDR = (unsigned int)qh_array;
	REG_ENDPTSETUPSTAT = EPSETUP_STATUS_EP0;
	
    REG_USBINTR =
        USBINTR_INT_EN |
        USBINTR_ERR_INT_EN |
        USBINTR_PTC_DETECT_EN |
        USBINTR_RESET_EN;
	
	REG_USBCMD |= USBCMD_RUN;
	
}

void usb_exec_ctrl_req(struct usb_ctrlrequest *ctrl_req){
	//printf("ReqType:%x \n",ctrl_req->bRequestType & USB_TYPE_MASK);


    switch(ctrl_req->bRequestType & USB_TYPE_MASK)
    {
        case USB_TYPE_STANDARD:
            handle_std_req(ctrl_req);
            break;
        case USB_TYPE_CLASS:
            //handle_class_req(ctrl_req);
            break;
        default:
            usb_drv_stall(EP_CONTROL, true, true);
    }

}
