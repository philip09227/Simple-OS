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
extern Scheduler* SYSTEM_SCHEDULER;
/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size) 
  : SimpleDisk(_disk_id, _size) {
	  disk_head = NULL;
	  disk_tail = NULL;

}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/
bool BlockingDisk::disk_is_ready()
{
	return SimpleDisk::is_ready();
}

void BlockingDisk::disk_wait_until_ready()
{
	if(!SimpleDisk::is_ready())
	{
		Thread* current_thread = Thread::CurrentThread();
		disk_enqueue(current_thread);
		SYSTEM_SCHEDULER->yield();
	}
}

void BlockingDisk::disk_enqueue(Thread* _thread)
{
	disk_queue* new_queue = new disk_queue();
	new_queue->thread = _thread;
	new_queue ->next = NULL;
	// if queue is empty 
	if( disk_head ==NULL || disk_tail==NULL)
	{
		disk_head =disk_tail = new_queue;
	}
	else
	{
		disk_tail->next =  new_queue;
		disk_tail = disk_tail->next;
	}
}

Thread* BlockingDisk::disk_dequeue()
{
	if(disk_head!=NULL)
	{
		disk_queue* temp_queue = disk_head;
		Thread* temp_thread = disk_head->thread;
		if( disk_head == disk_tail)
		{
			disk_head = NULL;
			disk_tail = NULL;
		}
		else
		{
			disk_head = disk_head->next;
		}
		delete temp_queue;
		return temp_thread;
	}
}


	
void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {
  SimpleDisk::issue_operation(DISK_OPERATION::READ, _block_no);
  SYSTEM_SCHEDULER->flag = true;
  disk_wait_until_ready();
  int i;
        unsigned short tmpw;
        for ( i = 0; i< 256; i++)
        {
                tmpw = Machine::inportw(0x1F0);
                _buf[i*2] = (unsigned char) tmpw;
                _buf[i*2+1] = (unsigned char) (tmpw >> 8);

        }
        
        Console::puts("Block Disk read done\n");
	SYSTEM_SCHEDULER->flag = false;

}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {
	SimpleDisk::issue_operation(DISK_OPERATION::WRITE , _block_no);
	SYSTEM_SCHEDULER->flag = true;
	disk_wait_until_ready();
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
	SYSTEM_SCHEDULER->flag = false;
}

void BlockingDisk::interrupt_handler(REGS *_r)
{
	Thread* next_thread = disk_dequeue();
	SYSTEM_SCHEDULER->resume(next_thread->CurrentThread());
}


