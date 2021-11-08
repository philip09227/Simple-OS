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

Scheduler::Scheduler() {
  current_running_thread = NULL;
     
  head = NULL;
  tail = NULL; 
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
	if(Machine::interrupts_enabled())
	{
		Machine::disable_interrupts();
	}
	if(head!=NULL) // first item in ready queue is not NULL
	{
		queue* first_queue = head; //first item
		Thread* next_thread = first_queue->thread; 
		head = head->next;
		current_running_thread = next_thread;		
		//Console::puts("dispatch to next thread\n");
		Thread::dispatch_to(next_thread);

	}
	//Console::puts("dispatch done \n");

	if(!Machine::interrupts_enabled())
	{
		Machine::enable_interrupts();
	}
	Console::puts("Yield finished.\n");
		
}

void Scheduler::resume(Thread * _thread) {
	if(Machine::interrupts_enabled())
        {
                Machine::disable_interrupts();
        }
	enqueue(_thread);

	if(!Machine::interrupts_enabled())
        {
                Machine::enable_interrupts();
        }
	Console::puts("resume done\n");
}

void Scheduler::add(Thread * _thread) {
	if(Machine::interrupts_enabled())
        {
                Machine::disable_interrupts();
        }
        
        enqueue(_thread);

        if(!Machine::interrupts_enabled())
        {
                Machine::enable_interrupts();
        }

	Console::puts("add done\n");

}

void Scheduler::terminate(Thread * _thread) {
	//if thread sucide
	if(current_running_thread ==_thread)
	{
		yield();		
		
	}
	else
	{	if( head->thread ==_thread)
		{
			head = head->next;
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
					break;
				}
				current = current->next;
				prev = prev->next;
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

	

RRScheduler::RRScheduler(unsigned EOQ)
{
	  current_running_thread = NULL;

  head = NULL;
  tail = NULL;
  SimpleTimer* timer = new SimpleTimer(1000/EOQ);
  InterruptHandler::register_handler(0,timer);
  Console::puts("Constructed RR Scheduler.\n");
}

void RRScheduler::yield() {
        if(Machine::interrupts_enabled())
        {
                Machine::disable_interrupts();
        }
        if(head!=NULL) // first item in ready queue is not NULL
        {
                queue* first_queue = head; //first item
                Thread* next_thread = first_queue->thread;
                head = head->next;
                current_running_thread = next_thread;
               
                Thread::dispatch_to(next_thread);

        }
        

        if(!Machine::interrupts_enabled())
        {
                Machine::enable_interrupts();
        }
        Console::puts("RR Yield finished.\n");

}

void RRScheduler::resume(Thread * _thread) {
        if(Machine::interrupts_enabled())
        {
                Machine::disable_interrupts();
        }
        enqueue(_thread);

        if(!Machine::interrupts_enabled())
        {
                Machine::enable_interrupts();
        }
        Console::puts("RR resume done\n");
}

void RRScheduler::add(Thread * _thread) {
        if(Machine::interrupts_enabled())
        {
                Machine::disable_interrupts();
        }

        enqueue(_thread);

        if(!Machine::interrupts_enabled())
        {
                Machine::enable_interrupts();
        }

        Console::puts("RR add done\n");

}
void RRScheduler::terminate(Thread * _thread) {
        //if thread sucide
        if(current_running_thread ==_thread)
        {
                yield();

        }
        else
        {       if( head->thread ==_thread)
                {
                        head = head->next;
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
                                        break;
                                }
                                current = current->next;
                                prev = prev->next;
                        }
                }
        }

	Console::puts("RR Terminate Scheduler\n");

}
void RRScheduler::enqueue(Thread* _thread)
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
        Console::puts("RR enqueue done\n");
}

