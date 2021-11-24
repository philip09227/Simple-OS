/*
     File        : blocking_disk.c

     Author      : 
     Modified    : 

     Description : 

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "console.H"
#include "blocking_disk.H"
#include "scheduler.H"
#include "simple_disk.H"
#include "thread.H"

extern Scheduler* SYSTEM_SCHEDULER;
/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size) 
  : SimpleDisk(_disk_id, _size) {
	  blockingdisk_queue_size = 0 ;
	  blockingdisk_head = NULL;
	  blockingdisk_tail = NULL;
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/
/*int BlockingDisk::blockingdisk_device_queue_size()
{
	return blockingdisk_queue_size;
}*/
void BlockingDisk::blockingdisk_enqueue(Thread* _thread)
{
	blockingdisk_queue* new_queue = new blockingdisk_queue;
	new_queue->thread = _thread;
	new_queue->next = NULL;
	if((blockingdisk_head ==NULL ||  blockingdisk_tail ==NULL))
	{
		blockingdisk_head = blockingdisk_tail = new_queue;
	}
	else
	{
		blockingdisk_tail->next = new_queue;
		blockingdisk_tail = new_queue;
	}
	blockingdisk_queue_size+=1;
	Console::puts(" Blcoking disk  queue enqueue done\n");

}
Thread* BlockingDisk::blockingdisk_dequeue()
{
	if (blockingdisk_head!=NULL|| blockingdisk_tail!=NULL)
	{
		blockingdisk_queue* temp = blockingdisk_head;
                Thread* return_thread = temp->thread;

		// only one item in device queue
		if (blockingdisk_head ==blockingdisk_tail)
		{
			blockingdisk_head = NULL;
			blockingdisk_tail = NULL;
			delete temp;
			blockingdisk_queue_size--;
			
		}
		else
		{
			
			blockingdisk_head = blockingdisk_head->next;
			delete temp;
			blockingdisk_queue_size--;
		
		}
		Console::puts(" Device queue dequeue done\n");
        	return return_thread;
	}


}

			

void BlockingDisk::blockingdisk_wait_until_ready()
{
	if(!blockingdisk_is_ready())
	{
		// devices are not ready therefore, add current thread which is calling I/O operation into device queue
		Thread * current_thread = Thread::CurrentThread();
		blockingdisk_enqueue(current_thread);
		blockingdisk_queue_size+=1;
		//SYSTEM_SCHEDULER->resume(current_thread);
		Console::puts(" device is not ready  thread calling  yield \n");	
		SYSTEM_SCHEDULER->resume(current_thread);
		SYSTEM_SCHEDULER->yield();
	
	}
}



bool BlockingDisk::blockingdisk_is_ready()
{
	return SimpleDisk::is_ready();
}

void BlockingDisk::blockingdisk_read(unsigned long _block_no, unsigned char * _buf) {
	// check the status of device whether it is ready for I/O opearation 
	//wait_until_ready();
 	SimpleDisk::issue_operation(DISK_OPERATION::READ,_block_no);
//	SYSTEM_SCHEDULER->flag = true;
	bool flag2 = blockingdisk_is_ready();
	Console::puts("Before wait until ready bdisk is ready??????"); Console::puti(flag2);Console::puts("\n");
	blockingdisk_wait_until_ready();
	bool flag3= blockingdisk_is_ready();
	Console::puts("After wait until ready bdisk is ready??????"); Console::puti(flag3);Console::puts("\n");

	int i;
	unsigned short tmpw;
	for ( i = 0; i< 256; i++)
	{
		tmpw = Machine::inportw(0x1F0);
		_buf[i*2] = (unsigned char) tmpw;
		_buf[i*2+1] = (unsigned char) (tmpw >> 8);

	}
  	//SYSTEM_SCHEDULER->flag = false;
  	Console::puts("Block Dish read done\n");


}


void BlockingDisk::blockingdisk_write(unsigned long _block_no, unsigned char * _buf) {
  	//wait_until_ready();
	SimpleDisk::issue_operation(DISK_OPERATION::WRITE , _block_no);
	//SYSTEM_SCHEDULER->flag = true;
        bool flag2 = blockingdisk_is_ready();
        Console::puts("Before wait until ready bdisk is ready??????"); Console::puti(flag2);Console::puts("\n");
        blockingdisk_wait_until_ready();
        bool flag3= blockingdisk_is_ready();
        Console::puts("After wait until ready bdisk is ready??????"); Console::puti(flag3);Console::puts("\n");

        int i;
        unsigned short tmpw;
        for ( i = 0; i< 256; i++)
        {
		for( i=0; i<256; i++)
		{
			tmpw = _buf[2*i] | ( _buf[2*i+1] << 8);
			Machine::outportw(0x1F0,tmpw);
                }
        }
	Console::puts("Block Disk write done\n");
	//SYSTEM_SCHEDULER->flag = false;
}
