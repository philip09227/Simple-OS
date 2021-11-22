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
#include "simple_idisk.H"
#include "thread.H"

extern Scheduler* SYSTEM_SCHEDULER
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
	queue* new_queue = new queue;
	new_queue->thread = _thread;
	new_QUEUE->next = NULL;
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
void BlockDisk::dequeue()
{
	if (head!=NULL|| tail!=NULL)
	{
		// only one item in device queue
		if (head ==tail)
		{
			queue* temp = head;
			Thread* return_thread = temp->thread;
			head = NULL;
			tail = NULL;
			delete temp;
			return return_thread;
		}
		else
		{
			queue* temp = head;
			head = head->next;
			Thread* return_thread = temp->thread;
			delete temp;
			return return_thread;
		}
	}
}

			

void BlockingDisk::wait_until_ready()
{
	if(!is_readu())
	{
		// devices are not ready therefore, add current thread which is calling I/O operation into device queue
		Thread * current_thread = Thread::CurrentThread();
		head->enqueue(current_thread);
		queue_size+=1;
		SYSTEM_SCHEDULER->yield();
	}
}



bool BlockingDisk::is_ready()
{
	return SimpleDisk::is_ready();
}

void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {
	
	wait_until_ready();

  	SimpleDisk::read(_block_no, _buf);

}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {
  // -- REPLACE THIS!!!
  SimpleDisk::write(_block_no, _buf);
}
