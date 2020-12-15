#include "tusb.h"
#include "ServiceRawFlash.h"
#include "raw_flash.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#if CFG_TUD_MSC

// Invoked when received SCSI_CMD_INQUIRY
// Application fill vendor id, product id and revision with string up to 8, 16, 4 characters respectively
void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
{
  (void) lun;

  const char vid[] = "HP39GII";
  const char pid[] = " Mass Storage";
  const char rev[] = "1.0";

  memcpy(vendor_id  , vid, strlen(vid));
  memcpy(product_id , pid, strlen(pid));
  memcpy(product_rev, rev, strlen(rev));
}

// Invoked when received Test Unit Ready command.
// return true allowing host to read/write this LUN e.g SD card inserted
bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
  (void) lun;
	if(xGetFlashStatus() == NO_ERROR){
		return true; 
	}
  return false; 
}

// Invoked when received SCSI_CMD_READ_CAPACITY_10 and SCSI_CMD_READ_FORMAT_CAPACITY to determine the disk size
// Application update block count and block size
void tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count, uint16_t* block_size)
{
  (void) lun;

  *block_count = 65536;
  *block_size  = 2048;
}

// Invoked when received Start Stop Unit command
// - Start = 0 : stopped power mode, if load_eject = 1 : unload disk storage
// - Start = 1 : active mode, if load_eject = 1 : load disk storage
bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
{
  (void) lun;
  (void) power_condition;

  if ( load_eject )
  {
    if (start)
    {
      // load disk storage
    }else
    {
      // unload disk storage
    }
  }

  return true;
}

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and return number of copied bytes.
int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
{
  (void) lun;
  
  unsigned char *rb_buffer;
	if(bufsize == 2048){
		xReadFlashPages(lba,1,buffer,5000);
	}else{
		rb_buffer = pvPortMalloc(bufsize);
		xReadFlashPages(lba,1,(unsigned int *)rb_buffer,5000);
		memcpy(buffer, rb_buffer + offset, bufsize);
		vPortFree(rb_buffer);
	}
  //uint8_t const* addr = msc_disk[lba] + offset;
  //memcpy(buffer, addr, bufsize);
  //xReadFlashPages(lba,1,buffer,5000);
  
	//memset(buffer,0,bufsize);
	//cdc_printf("USB Read %d\n",bufsize);
  return bufsize;
}
extern unsigned char NMETA[19];
// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and return number of written bytes
int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize)
{
  (void) lun;

  //uint8_t* addr = msc_disk[lba] + offset;
  //memcpy(addr, buffer, bufsize);
	//printf("lba %d,offset %d,size:%d\n",lba,offset,bufsize);
	/*OperationQueue O;
	O.OperationType = WRTIE;
	O.whichPage = lba;
	O.flashDataInBuffer = buffer;
	O.flashMetaInBuffer = NULL;
	*/
	delay_us(1000);
	GPMI_write_block_with_ecc8(NAND_CMD_SEQIN,NAND_CMD_PAGEPROG,NAND_CMD_STATUS,
								lba, buffer, NMETA);
	
	//xQueueSend(flashOperationQueue, &O , ( TickType_t ) 0 );

	//xWriteFlashPages(lba,1,buffer,NULL,1000);
	

  return bufsize;
}

// Callback invoked when received an SCSI command not in built-in list below
// - READ_CAPACITY10, READ_FORMAT_CAPACITY, INQUIRY, MODE_SENSE6, REQUEST_SENSE
// - READ10 and WRITE10 has their own callbacks
int32_t tud_msc_scsi_cb (uint8_t lun, uint8_t const scsi_cmd[16], void* buffer, uint16_t bufsize)
{
  // read10 & write10 has their own callback and MUST not be handled here

  void const* response = NULL;
  uint16_t resplen = 0;

  // most scsi handled is input
  bool in_xfer = true;

  switch (scsi_cmd[0])
  {
    case SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL:
      // Host is about to read/write etc ... better not to disconnect disk
      resplen = 0;
    break;

    default:
      // Set Sense = Invalid Command Operation
      tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);

      // negative means error -> tinyusb could stall and/or response with failed status
      resplen = -1;
    break;
  }

  // return resplen must not larger than bufsize
  if ( resplen > bufsize ) resplen = bufsize;

  if ( response && (resplen > 0) )
  {
    if(in_xfer)
    {
      memcpy(buffer, response, resplen);
    }else
    {
      // SCSI output
    }
  }

  return resplen;
}

#endif