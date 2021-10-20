/*
 File: vm_pool.C
 
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

#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
#include "page_table.H"

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
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table) 
{
    
    // Initialize all the data structures
    base_address = _base_address;
    size = _size;
    frame_pool = _frame_pool;
    page_table = _page_table;
    page_table -> register_pool(this);
    //initialize first item of vm pool to store base information
    region * region_array = (region *)(base_address);
    region_array[0].first_address = _base_address;
    region_array[0].size = Machine::PAGE_SIZE;
    // initialize the rest of vm pool
    for(int i = 1; i < 512; i++)
    {
        region_array[i].first_address = 0;
        region_array[i].size = 0;
    }

    Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) 
{
    Console::puts("entry allocate  vm pool "); Console::puts("\n"); 	
    unsigned int i;
    unsigned int needed_pages= _size/Machine::PAGE_SIZE;
    unsigned int remainder= _size%Machine::PAGE_SIZE;
    region * region_array = (region *)(base_address);
    if(remainder >0)
    {
  	needed_pages++;
    }
    for (i = 1; i < 512; i++)
    {
        // Find the free region and allocate the memory
        if (region_array[i].size == 0)
        {
		//find first free vm pool it's first address should be previous addr+ previous size
                region_array[i].first_address = region_array[i-1].first_address + region_array[i-1].size;
                region_array[i].size = needed_pages * Machine::PAGE_SIZE; // this vm pool size should be needed * page size
                break;
        }
    }

    Console::puts("Allocated region of memory.\n");
    return region_array[i].first_address;
}

void VMPool::release(unsigned long _start_address)
{
    Console::puts("entry release vm pool "); Console::puts("\n"); 
    region * region_array = (region *)(base_address);
    unsigned int i;
    // Find the region vm pool whom address equal _start_address
    for (i = 1; i < 512; i++)
    {
        if (region_array[i].first_address == _start_address)
        {
            Console::puts("Found the address"); Console::puts("\n"); 
            break;
        }

    }

    unsigned int j = Machine::PAGE_SIZE;
    // Free the frames allocated
    
    while( j <= region_array[i].size)
    {
	    page_table -> free_page(region_array[i].first_address + j);
	    j +=Machine::PAGE_SIZE;
    }
    // since we remove the  vm pool we need to move the rest of vm pool foward 
    for( i; i< 511; i++)
    {
	    if ( region_array[i+1].size !=0)
	    {
		    region_array[i].first_address = region_array[i + 1].first_address;
            	    region_array[i].size = region_array[i + 1].size;
	    }
	    else
	    {
		    region_array[i].first_address = 0;
            	    region_array[i].size = 0;
	    }
    }
    Console::puts("Released region of memory.\n");
}

bool VMPool::is_legitimate(unsigned long _address) 
{
    if (_address == base_address)
    {
        return true;
    }
    unsigned long address = _address;
    region * region_array = (region *)(base_address);	
    // Check whether the address falls into a region
    for (int i = 0; i < 512; i++)
    {
        if ((address >= region_array[i].first_address) && (address <= region_array[i].first_address + region_array[i].size))
        {
            return true;
            
        }
    }
    // if not found means it's in legitimate 
    return false;
    Console::puts("Checked whether address is part of an allocated region.\n");
}

