/*
 File: scheduler.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
#include "simple_timer.H"
#include "blocking_disk.H"
#include "mirrored_disk.H"
/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

//extern BlockingDisk* SYSTEM_DISK; 
extern MirroredDisk* SYSTEM_DISK;
Scheduler::Scheduler() {
  current_running_thread = NULL;
  flag = false;  
  head = NULL;
  tail = NULL; 
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
/*	if(Machine::interrupts_enabled())
	{
		Machine::disable_interrupts();
	}*/
	// check whether IO device is done or not if it's done, dequeu item form device queue and add it to the front of ready queue
	Console::puts("==================================================================================================\n");


	bool flag1 = SYSTEM_DISK->MASTER_DISK->disk_is_ready();
	Console::puts("is ready????  "); Console::puti(flag1);Console::puts("\n");
	
	if (SYSTEM_DISK->MASTER_DISK->disk_head!=NULL && SYSTEM_DISK->DEPENDENT_DISK->disk_head!=NULL && SYSTEM_DISK->MASTER_DISK->disk_is_ready()&& SYSTEM_DISK->DEPENDENT_DISK->disk_is_ready() )

	{

		
		Thread* device_thread = SYSTEM_DISK->MASTER_DISK->disk_dequeue();
		Thread* device_thread2 = SYSTEM_DISK->DEPENDENT_DISK->disk_dequeue();

		//Thread* temp = SYSTEM_DISK->DEPENDENT_DISK->dequeue();
		Console::puts("****************************************************************************************\n");

		Thread::dispatch_to(device_thread);
		
	}
	else
	{
		if(head!=NULL) // first item in ready queue is not NULL
		{
			Console::puts("jha\n");
			queue* first_queue = head; //first item
			Thread* next_thread = first_queue->thread; 
			head = head->next;
			current_running_thread = next_thread;	
	        	delete first_queue;	
			Console::puts("dispatch to next thread\n");
			Thread::dispatch_to(next_thread);

		}	
	}
	//Console::puts("dispatch done \n");

/*	if(!Machine::interrupts_enabled())
	{
		Machine::enable_interrupts();
	}*/
	Console::puts("Yield finished.\n");
		
}

void Scheduler::resume(Thread * _thread) {
/*	if(Machine::interrupts_enabled())
        {
                Machine::disable_interrupts();
        }*/
	enqueue(_thread);

/*	if(!Machine::interrupts_enabled()
        {
                Machine::enable_interrupts();
        }*/
	Console::puts("resume done\n");
}

void Scheduler::add(Thread * _thread) {
/*	if(Machine::interrupts_enabled())
        {
                Machine::disable_interrupts();
        }*/
        
        enqueue(_thread);

/*        if(!Machine::interrupts_enabled())
        {
               Machine::enable_interrupts();
        }*/

	Console::puts("add done\n");

}

void Scheduler::terminate(Thread * _thread) {
	//if thread sucide
	Console::puts("Started terminate \n");
	if(current_running_thread ==_thread)
	{
		delete _thread;
		yield();		
	}
	else
	{	if( head->thread ==_thread)
		{
			queue * temp = head;
			head = head->next;
			delete temp;
			delete _thread;
		}
		else
		{
			queue* prev = head;
			queue* current = head->next;
			while(current!=NULL)
			{
				if(current->thread == _thread)
				{
					prev->next = current->next;	
					delete current;
					delete _thread;
					break;
				}
				else
				{
					current = current->next;
					prev = prev->next;
				}
			}
		}
	}
	Console::puts("Terminate Scheduler\n");

}

void Scheduler::enqueue(Thread* _thread)
{
	
	queue* new_queue = new queue;
	new_queue->thread = _thread;
	new_queue->next = NULL;
	
	if(head ==NULL||tail ==NULL)
	{
		head = tail = new_queue;
	}
	else
	{
		tail->next = new_queue;
    		tail = new_queue;
	}
        Console::puts("enqueue done\n");
} 

	

