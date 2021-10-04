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



void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
   kernel_mem_pool = _kernel_mem_pool;
   process_mem_pool = _process_mem_pool;
   shared_size = _shared_size;
   Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
   // in kernel frame pool we have total 1024 entries which are directy mapping from frames to pages
   // get a frame for page directory
   page_directory = (unsigned long *) ((kernel_mem_pool->get_frames(1))*PAGE_SIZE);
   // get a frame for page table 
   unsigned long * page_table_page = (unsigned long *) ((kernel_mem_pool->get_frames(1))*PAGE_SIZE);
   unsigned long address = 0;
   // we shared 4MB so need 1024 frames and first 4MB are directed mapping  
   unsigned long shared_frames = (PageTable::shared_size/PAGE_SIZE); 

   //set the write & valid-> means this entry point to a valid frame
   //if frame this page entry point to is not use theres no need to associate a page to it
   //setup the frames for directed mapping   and inititaize the last two bits to 11 
   for(int i =0; i <shared_frames; i++)
   {
	   page_table_page[i] = address|3;
	   address+=4096;
   }
   Console::puts("page_table_page  is : "); Console::puti(page_table_page[1]);Console::puts("\n");
   //point the first entry in page directory to page table page  
   page_directory[0] = (unsigned long) page_table_page;
   Console::puts("page_directory is : "); Console::puti(page_directory[0]);Console::puts("\n");
   page_directory[0] = page_directory[0] | 3;
   
   //initialize the rest of page directory
   for(int i = 1; i< shared_frames; i++)
   {
	   page_directory[i] = 0;

   }
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

   write_cr0(read_cr0()|0x80000000);
   paging_enabled = 1;
   Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
  // if cpu reference a page page number is 0 then page fault occur
  // called when access a page is not a frame associated it's in valid 
  unsigned long* page_directory = (unsigned long *) read_cr3(); //what address cause page fault
  // page direntory entry
  unsigned long page_directory_index = read_cr2() >>22;
  unsigned long page_table_page_index;
  unsigned long *page_table_page;
  unsigned long *page_table_page_address;
  unsigned long page_entry;
  unsigned long *page;
  unsigned long error_code = _r->err_code;

  if ( error_code & 0xFFFFFFFE)
  {
	  if ((page_directory[page_directory_index] &0x1)==0) // page fault occurs because page directory
	  {
                  Console::puts("page fault occur at directory"); 
		  page_table_page = (unsigned long *) ((kernel_mem_pool->get_frames(1))*PAGE_SIZE); // create a new page table page  

	          page_directory[page_directory_index]=((unsigned long) page_table_page)|0x3; // point the entry of page directory to page table page
 	  }
	  //get the first 20 bits
	  page_table_page_address = ((unsigned long *) (page_directory[page_directory_index] &0xffffff000));
	  //get the last 10 bits 
	  page_table_page_index = ((read_cr2()>>12) & 0x3ff);
	  if ((page_table_page_address[page_table_page_index]&0x1)==0)
	  {
	          Console::puts("page fault occur at page table ");
		  page = (unsigned long *) ((process_mem_pool ->get_frames(1))* PAGE_SIZE);
                  page_table_page_address[page_table_page_index]=((unsigned long) page)|0x3;
	  }

  }



  Console::puts("handled page fault\n");
}

