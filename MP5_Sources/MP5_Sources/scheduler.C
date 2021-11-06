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
  if(head!=NULL)
  {
	  queue* temp = head;
	  Thread* next_thread = head->thread;
	  head = head->next;
	  if(head==NULL)
	  {
		  tail=NULL;
	  }
	  current_running_thread = next_thread;
	  Thread::dispatch_to(next_thread);
  }
}

void Scheduler::resume(Thread * _thread) {
  enqueue(_thread);
}

void Scheduler::add(Thread * _thread) {
  enqueue(_thread);

}

void Scheduler::terminate(Thread * _thread) {
 // running thread sucide
  if (current_running_thread == _thread)
  {
	 yield();
  }
  else
  {
  	if(head->thread == _thread) // if terminate head
  	{
	  	head = head->next;
	  	if(head==NULL)
	  	{
	  		tail = NULL;
	  	}
  	}
  	else
  	{
  		queue* current = head->next;
  		queue* prev = head;
  		while(current!=NULL)
  		{
	  		if (current->thread !=_thread)
	  		{
		  		current = current->next;
		 		prev = prev->next;
	  		}
	  		else
	  		{
				prev->next = current->next;
				current = current->next;
			}
		}
  	}
  }
}

void Scheduler::enqueue(Thread* _thread)
{
	if(head==NULL||tail==NULL)
	{
		queue new_thread;
		new_thread.thread = _thread;
		new_thread.next = NULL;
		head = &new_thread;
		tail = &new_thread;
	}
	else
	{
		queue new_thread;
		new_thread.thread = _thread;
		new_thread.next = NULL;
		tail->next = &new_thread;
		tail = tail->next;
	}
}

	


