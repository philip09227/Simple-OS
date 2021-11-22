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
	  queue_size = 0 ;
	  head = NULL;
	  tail = NULL;
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/
void BlockingDisk::enqueue(Thread* _thread)
{
	device_queue* new_queue = new device_queue;
	new_queue->thread = _thread;
	new_queue->next = NULL;
	if((head ==NULL ||  tail ==NULL))
	{
		head = tail = new_queue;
	}
	else
	{
		tail->next = new_queue;
		tail = new_queue;
	}
	queue_size+=1;
	Console::puts(" Device queue enqueue done\n");

}
Thread* BlockingDisk::dequeue()
{
	if (head!=NULL|| tail!=NULL)
	{
		// only one item in device queue
		if (head ==tail)
		{
			device_queue* temp = head;
			Thread* return_thread = temp->thread;
			head = NULL;
			tail = NULL;
			delete temp;
			queue_size--;
			return return_thread;
		}
		else
		{
			device_queue* temp = head;
			head = head->next;
			Thread* return_thread = temp->thread;
			delete temp;
			queue_size--;
			return return_thread;
		}
	}
}

			

void BlockingDisk::wait_until_ready()
{
	if(!is_ready())
	{
		// devices are not ready therefore, add current thread which is calling I/O operation into device queue
		Thread * current_thread = Thread::CurrentThread();
		enqueue(current_thread);
		queue_size+=1;
		SYSTEM_SCHEDULER->resume(current_thread);
		SYSTEM_SCHEDULER->yield();
	}
}



bool BlockingDisk::is_ready()
{
	return SimpleDisk::is_ready();
}

void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {
	// check the status of device whether it is ready for I/O opearation 
	wait_until_ready();
 	SimpleDisk::issue_operation(DISK_OPERATION::READ,_block_no);
	wait_until_ready();
	int i;
	unsigned short tmpw;
	for ( i = 0; i< 256; i++)
	{
		tmpw = Machine::inportw(0x1F0);
		_buf[i*2] = (unsigned char) tmpw;
		_buf[i*2+1] = (unsigned char) (tmpw >> 8);
		Console::puts("Block Dish read done\n");

	}

  	

}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {
  	wait_until_ready();
	SimpleDisk::issue_operation(DISK_OPERATION::WRITE , _block_no);
        wait_until_ready();
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
	Console::puts("Block Dish write done\n");

}
