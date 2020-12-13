#ifndef USB_DCD_H
#define USB_DCD_H


#define USB_BASE            0x80080000
#define USB_NUM_ENDPOINTS   5
#define USB_MAX_CURRENT 	500

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






#endif