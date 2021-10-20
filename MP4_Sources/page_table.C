#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;
VMPool * PageTable::vm_pool[];



void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
    kernel_mem_pool = _kernel_mem_pool;
    process_mem_pool = _process_mem_pool;
    shared_size = shared_size;
    Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
   // in kernel frame pool we have total 1024 entries which are directy mapping from frames to pages
   // get a frame for page directory
   page_directory = (unsigned long *) ((process_mem_pool->get_frames(1))*PAGE_SIZE);
   // get a frame for page table
   unsigned long * page_table_page = (unsigned long *) ((process_mem_pool->get_frames(1))*PAGE_SIZE);
   unsigned long address = 0;
   // we shared 4MB so need 1024 frames and first 4MB are directed mapping
   unsigned long shared_frames = (PageTable::shared_size/PAGE_SIZE);

   //set the write & valid-> means this entry point to a valid frame
   //if frame this page entry point to is not use theres no need to associate a page to it
   //setup the frames for directed mapping   and inititaize the last two bits to 11
   for(int i =0; i <1024; i++)
   {
           page_table_page[i] = address|3;
           address+=4096;
   }
   //point the first entry in page directory to page table page
   page_directory[0] = (unsigned long) page_table_page;
   page_directory[0] = page_directory[0] | 3;

   //initialize the rest of page directory
   for(int i = 1; i< 1024; i++)
   {
           page_directory[i] = 0|2;

   }
   // initialize the last entry in table directory to the head of page directory recurrsive page table directory
   page_directory[1023] = (unsigned long) page_directory|3;
   Console::puts("Constructed Page Table object\n");

}


void PageTable::load()
{
   // load the address (page directory address)  into page table register
   current_page_table = this;
   //store page directory address into cr3 register
   unsigned long temp = (unsigned long)page_directory;
   Console::puts(" load into register"); Console::puti(temp); Console::puts("\n");
   write_cr3((unsigned long)page_directory);
   Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
    // set the paging bit in cr0 to 1
    write_cr0 (read_cr0() | 0x80000000);
    paging_enabled = 1;
    Console::puts("Enabled paging\n");
}

// 1023 + 1023 + 10 bits for directory + 00 to get page directory entry 
unsigned long * PageTable::PDE_address(unsigned long address)
{

        address = (address >> 22) << 2;
        address = address |0xfffff000;
        return (unsigned long*) address;
}

//1023 + 10 bits for directory + 10 bits for page table + 00 to get page table page entry 
unsigned long * PageTable::PTE_address(unsigned long address)
{
      
        address = (address >> 12) << 2;
        address = address |0xffc00000;
        return (unsigned long *) address;
}


void PageTable::handle_fault(REGS * _r)
{
  unsigned long * current_page_directory = (unsigned long *) read_cr3(); // the address of page directory
  unsigned long current_page_fault_address = read_cr2(); // the address which causes page fault
  unsigned long current_page_directory_index = current_page_fault_address  >> 22; // first 10 bits represent the index of page directory
  unsigned long current_page_table_page_index; // index of entry in page table page
  unsigned long * current_page_table_page_address; // address of current page table page
  unsigned long * new_page_table_page; // pointer of new  page table page for fault directory
  unsigned long * new_pte; // new page table entry for fault page table
  unsigned long exc_no = _r->int_no;
  unsigned long * page_directory_entry;
  unsigned long * page_table_entry; 
  unsigned int flag = 0;
  unsigned int count = 0;
  unsigned int i;
    // Get the vm pool count
    for (i = 0; i < 512; i++)
    {
        if (vm_pool[i] != 0)
            count++;
    }

    for (i = 0; i < count; i++) 
    {
        // Check the address the legitimate before procedding 
	if (vm_pool[i] -> is_legitimate(current_page_fault_address) == true) 
        {
		flag = 1;
		break;
	}
    }
    if (flag == 0)
    {
	    Console::puts("Address is not legitimate \n");
	    assert(false);
    }

    if (exc_no==14)
    {
          page_directory_entry = current_page_table-> PDE_address(current_page_fault_address); 
          if (( *page_directory_entry & 1)==0) // page fault occurs because page directory
          {
                  Console::puts("page fault occurs at directory need to creat a new page table page for this entry\n");
                  new_page_table_page = (unsigned long *) ((process_mem_pool->get_frames(1))*PAGE_SIZE); // create a new page table page  
                  *page_directory_entry = ((unsigned long) new_page_table_page)|3; // point the entry of page directory to page table page
          }
          page_table_entry = current_page_table-> PTE_address(current_page_fault_address); 
          // checking the present bit in page table
          if ((*page_table_entry &1)==0)
          {
                  Console::puts("page fault occur at page table\n ");
                  //reference a frame for this page table page entry
                  new_pte = (unsigned long *) ((process_mem_pool ->get_frames(1))* PAGE_SIZE);
                  *page_table_entry=((unsigned long) new_pte)|3;
          }
    }
    Console::puts("handled page fault\n");
}

void PageTable::register_pool(VMPool * _vm_pool)
{
	// find first available vm pool 
        for (int i = 0; i < 512; i++)
        {
		if (vm_pool[i] == 0)
		{
			vm_pool[i] = _vm_pool;
                	break;
		}
        }
        Console::puts("registered VM pool\n");
}

void PageTable::free_page(unsigned long _page_no) 
{
    unsigned long * page_table_entry;
    unsigned long   frame_number;
    page_table_entry = current_page_table-> PTE_address(_page_no); 
    if ((*page_table_entry &1))
    {
        frame_number = *page_table_entry >> 12;
        process_mem_pool->release_frames(frame_number);
        *page_table_entry = 0 | 2;
    }
    //  flush the TLB entries
    write_cr3((unsigned long) page_directory);
}
