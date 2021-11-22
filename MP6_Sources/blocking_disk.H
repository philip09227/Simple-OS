/*
     File        : blocking_disk.H

     Author      : 

     Date        : 
     Description : 

*/

#ifndef _BLOCKING_DISK_H_
#define _BLOCKING_DISK_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "simple_disk.H"
#include "thread.H"
#include "interrupts.H"

struct device_queue
{
	Thread * thread;
	struct device_queue* next;
};

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* B l o c k i n g D i s k  */
/*--------------------------------------------------------------------------*/

class BlockingDisk : public SimpleDisk {
public:
  bool is_ready();
  Thread* dequeue();
  void enqueue(Thread* _thread);

   BlockingDisk(DISK_ID _disk_id, unsigned int _size); 
   /* Creates a BlockingDisk device with the given size connected to the 
      MASTER or SLAVE slot of the primary ATA controller.
      NOTE: We are passing the _size argument out of laziness. 
      In a real system, we would infer this information from the 
      disk controller. */

   /* DISK OPERATIONS */

   virtual void read(unsigned long _block_no, unsigned char * _buf);
   /* Reads 512 Bytes from the given block of the disk and copies them 
      to the given buffer. No error check! */

   virtual void write(unsigned long _block_no, unsigned char * _buf);
   /* Writes 512 Bytes from the buffer to the given block on the disk. */
protected:
        virtual void wait_until_ready();
        //device_queue* device_queue;
        device_queue* head;
        device_queue* tail;
        int queue_size;

};

#endif
