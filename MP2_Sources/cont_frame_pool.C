/*
 File: ContFramePool.C
 
 Author:
 Date  : 
 
 */

/*--------------------------------------------------------------------------*/
/* 
 POSSIBLE IMPLEMENTATION
 -----------------------

 The class SimpleFramePool in file "simple_frame_pool.H/C" describes an
 incomplete vanilla implementation of a frame pool that allocates 
 *single* frames at a time. Because it does allocate one frame at a time, 
 it does not guarantee that a sequence of frames is allocated contiguously.
 This can cause problems.
 
 The class ContFramePool has the ability to allocate either single frames,
 or sequences of contiguous frames. This affects how we manage the
 free frames. In SimpleFramePool it is sufficient to maintain the free 
 frames.
 In ContFramePool we need to maintain free *sequences* of frames.
 
 This can be done in many ways, ranging from extensions to bitmaps to 
 free-lists of frames etc.
 
 IMPLEMENTATION:
 
 One simple way to manage sequences of free frames is to add a minor
 extension to the bitmap idea of SimpleFramePool: Instead of maintaining
 whether a frame is FREE or ALLOCATED, which requires one bit per frame, 
 we maintain whether the frame is FREE, or ALLOCATED, or HEAD-OF-SEQUENCE.
 The meaning of FREE is the same as in SimpleFramePool. 
 If a frame is marked as HEAD-OF-SEQUENCE, this means that it is allocated
 and that it is the first such frame in a sequence of frames. Allocated
 frames that are not first in a sequence are marked as ALLOCATED.
 
 NOTE: If we use this scheme to allocate only single frames, then all 
 frames are marked as either FREE or HEAD-OF-SEQUENCE.
 
 NOTE: In SimpleFramePool we needed only one bit to store the state of 
 each frame. Now we need two bits. In a first implementation you can choose
 to use one char per frame. This will allow you to check for a given status
 without having to do bit manipulations. Once you get this to work, 
 revisit the implementation and change it to using two bits. You will get 
 an efficiency penalty if you use one char (i.e., 8 bits) per frame when
 two bits do the trick.
 
 DETAILED IMPLEMENTATION:
 
 How can we use the HEAD-OF-SEQUENCE state to implement a contiguous
 allocator? Let's look a the individual functions:
 
 Constructor: Initialize all frames to FREE, except for any frames that you 
 need for the management of the frame pool, if any.
 
 get_frames(_n_frames): Traverse the "bitmap" of states and look for a 
 sequence of at least _n_frames entries that are FREE. If you find one, 
 mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 ALLOCATED.

 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or 
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.
 
 mark_inaccessible(_base_frame_no, _n_frames): This is no different than
 get_frames, without having to search for the free sequence. You tell the
 allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
 frames after that to mark as ALLOCATED.
 
 needed_info_frames(_n_frames): This depends on how many bits you need 
 to store the state of each frame. If you use a char to represent the state
 of a frame, then you need one info frame for each FRAME_SIZE frames.
 
 A WORD ABOUT RELEASE_FRAMES():
 
 When we releae a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e., 
 not associated with a particular frame pool.
 
 This problem is related to the lack of a so-called "placement delete" in
 C++. For a discussion of this see Stroustrup's FAQ:
 http://www.stroustrup.com/bs_faq2.html#placement-delete
 
 */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

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
/* METHODS FOR CLASS   C o n t F r a m e P o o l */
/*--------------------------------------------------------------------------*/


ContFramePool* ContFramePool::node_head;
ContFramePool* ContFramePool::list;


ContFramePool::ContFramePool(unsigned long _base_frame_no,
                             unsigned long _n_frames,
                             unsigned long _info_frame_no,
                             unsigned long _n_info_frames)
{
   
    // bitmap can fit in one single frame
    assert(_n_frames <= FRAME_SIZE *4);

    base_frame_no = _base_frame_no; // the id of the first frame, in kernel first frame is 512nd frame
    n_frames = _n_frames; // how many frames in this pool,  2^9 frames for kernel pool
    nFreeFrames = _n_frames; 
    info_frame_no = _info_frame_no;
    n_info_frames = _n_info_frames;
    

    // bitmap is a array and it used as a indicator of address
    // if info_fram =0 means management information store in this frame
    // we use first frame in this poll to store information
    // and set the bitmap(address) to the address of first frame
    // aka. " Frame Number * Frame Size" frame number== base_frame
    // this time bitmap should be at 2MB  200000(hex) 
    if (info_frame_no == 0){
	    bitmap = (unsigned char *) ( base_frame_no * FRAME_SIZE);

    }else{
	    bitmap = (unsigned char *) (info_frame_no * FRAME_SIZE); 
	    // if not in the first frame set address to the current frame
	    // which contain the information 
	                                                          
    }
    
    //make sure each cell has four bit
    assert((n_frames %4) == 0);
    
    

    // status of frames
    // 00 means free
    // 01 means head
    // 10 meand in accessible
    // 11 means allocated 
    // inititalize all frame to free 
    // In Kernel frame pool there's 512 frames
    // Thus the loop will so 128 times to make each one is free 
    for (int i =0; i*4< n_frames; i++){
	    bitmap[i] = 0x0;
    }
    
    // set first frame in fram pool to allocated 
    if(info_frame_no==0){
	    bitmap[0] = 0xc0;  //which is 11 for the first two bit 
	    nFreeFrames--;    // decrease the number of totoal number of free frame 
    }

    // if list has not been declared
    if ( ContFramePool::node_head == NULL)
    {
	    ContFramePool::node_head = this;
	    ContFramePool::list = this;
    }
    else
    {
	    ContFramePool::list->next = this;
	    ContFramePool::list = this;
    }
    next = NULL;



    Console::puts("Frame Pool initialized\n");
}

unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{
    
    // Make sure we still have frames to allocated 
    assert(nFreeFrames >= _n_frames );
    
    // first number of frame pool
    unsigned long first_frame_no = 0;
    unsigned long last_frame_no = 0;
    unsigned int i =0;
    unsigned int j = 0;
    unsigned int frames_needed = _n_frames;
    unsigned int count =0;
    unsigned int bitmap_index;
    unsigned int rest;
    unsigned char checker = 0xc0;
    unsigned char checker_reset = 0xc0;
    unsigned char checker_head = 0x40 ;
    unsigned char checker_allocated = 0xc0;

    // find the first frames which has consecutive number of frames we can use
    // n_frame is the size of frame pool for kernel frame is 512 frames
    // since we need 8 bits for one frame we only can iterate 128 times 
    Console::puts("Get frame started: number of frames we need is : ");Console::puti(frames_needed); Console::puts("\n");
    while ((i < (n_frames/4)) && (count < frames_needed))    
    { 
            
	    Console::puts("Inside the while loop i : means the ith frame we are checking ");Console::puti(i); Console::puts("\n");
            Console::puts("Current count frame we have is : "); Console::puti(count); Console::puts("\n");
	    Console::puts("But we need frames_needed = "); Console::puti(frames_needed); Console::puts("\n");
	    for ( j=0 ; j < 4 && (count < frames_needed); j++)
	    {
                    Console::puts("Inside the j loop j : means checkeing the status for item in this bitmap[i] "); Console::puti(j); Console::puts("\n");
		    Console::puts("bitmap i in j is : "); Console::puti(bitmap[i]); Console::puts("\n");
		    Console::puts("checker for check this bitmap is head or not "); Console::puti(checker); Console::puts("\n");
                    unsigned char temp = (bitmap[i] & checker);// just for debugging
		    Console::puts("bitmap i & checker: temp outputof checkis head or not "); Console::puti(temp); Console::puts("\n");
	
		    // bit manipulation if i and checker = 0 means it's free 
		    if ((bitmap[i] & checker) == 0x0)
		    {
			    count++;
			    last_frame_no = base_frame_no + i*4 + j;
			    Console::puts("this bitmap[ i] frame is  free");Console::puts("\n");
                            Console::puts("current cout is"); Console::puti(count); Console::puts("\n");
		            Console::puts(" the last frame number is ="); Console::puti(last_frame_no); Console::puts("\n");
		    }
		    checker_reset = 0xc0 >> ((last_frame_no%4)*2);
		    checker_head = 0x40 >> ((last_frame_no%4)*2);
		    Console::puts("we need to check next two bit in bitamp[i] so right shift checker_reset = "); Console::puti(checker_reset); Console::puts("\n");
                    Console::puts("we need to checl next two bit in bitmap[i] so right shift checker_head = "); Console::puti(checker_head); Console::puts("\n");

		    //check it is head or not if it is head it will be 0 else wil be 40 = 01
		    if (((bitmap[i] & checker_reset)^checker_head)==0)
		    {
			    count= 0;
			    Console::puts("In this j for loop we found there is one is head so have to reset"); Console::puts("\n");
			    Console::puts("reset count to 0 "); Console::puts("\n");

		    }
		    checker = checker >> 2;
	    } // end j loop
	    // reset the checker for next i 
	    checker = 0xc0;
	    i++;
    }

    // in the end we will find the frame which has consecutive frames we need in front of it 	
    // to get the number of this sequential frames we need to use the last one substract the number frames requested      
    first_frame_no = last_frame_no - frames_needed+1;
    Console::puts("the first frame of this sequential is"); Console::puti(first_frame_no); Console::puts("\n");
    Console::puts("the last frame of this sequential is"); Console::puti(last_frame_no); Console::puts("\n");

    // get the index of head frame in bitmap
    bitmap_index = ((first_frame_no - base_frame_no)/4);
    Console::puts("head frame of sequence it's index is "); Console::puti(bitmap_index); Console::puts("\n");
    Console::puts("head frame of sequence it's value is (should be 0 )"); Console::puti(bitmap[bitmap_index]); Console::puts("\n");
    // find enough frames we need set the status set 01 to head  and rest to 11
    // set the head to 01
    checker_reset = 0xc0 >> ((first_frame_no%4)*2);
    checker_head = 0x40 >> ((first_frame_no%4)*2);
    bitmap[bitmap_index] = (( bitmap[bitmap_index] & (~checker_reset)) | checker_head); 
    Console::puts("After setting the value of  head frame in sequence is ( should be 01) "); Console::puti(bitmap[bitmap_index]); Console::puts("\n");

    //set the rest of frame to allocated 11
    rest = first_frame_no + 1;
    for ( rest; rest <= last_frame_no; rest++)
    {
	    
	    Console::puts("In rest loop current frame is  "); Console::puti(rest); Console::puts("\n");
	    Console::puts("the end is last_frame_no and it is :"); Console::puti(last_frame_no); Console::puts("\n");
	    //  get the index of next frame in bitmap
	    bitmap_index = ((rest - base_frame_no)/4);
	    checker_reset = 0xc0 >> ((rest%4)*2);
	    checker_allocated = 0xc0 >> ((rest)%4*2);
            Console::puts("the index for the rest frame in bitmap is  "); Console::puti(bitmap_index); Console::puts("\n"); 

            // set the status to 11
	    bitmap[bitmap_index] = ((bitmap[bitmap_index] & (~checker_reset)) | checker_allocated);
            Console::puts("After set the status of this rest frame index in bitmap is ( should be 11) "); Console::puti(bitmap[bitmap_index]); Console::puts("\n"); 
	    
    }
    
    nFreeFrames -= _n_frames;
    Console::puts(" total free frame we have right now is : " ); Console::puti(nFreeFrames);Console::puts("\n");
    Console::puts("return value of first frame is ") ; Console::puti(first_frame_no); Console::puts("\n");
    Console::puts("get frames done" ); Console::puts("\n");
    return (first_frame_no);
 
}

void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                      unsigned long _n_frames)
{  
    unsigned long start = _base_frame_no;
    unsigned long end = _base_frame_no + _n_frames;
    unsigned int bitmap_index;
    unsigned char checker;
    unsigned char checker_reset;
    for ( start; start < end; start++)
    {
	    bitmap_index = ( (start - base_frame_no)/4);
	    checker_reset = 0xc0 << (((start-base_frame_no)%4)*2);
	    checker = 0x80 >> (((start - base_frame_no)%4)*2);
	    // whater the value is reset it 
	    bitmap[bitmap_index] = bitmap[bitmap_index] & ( ~checker_reset);
	    // set the first two bit to 10 which means inaccessable
	    bitmap[bitmap_index] = bitmap[bitmap_index] | checker;
    }
    nFreeFrames -= _n_frames; 
}

void ContFramePool::release_frames(unsigned long _first_frame_no)
{
    // need to check this frame pool possess the frame we want to release
    ContFramePool* current = ContFramePool::node_head;
    Console::puts("Entry release : the head frame want to release is ");Console::puti(_first_frame_no); Console::puts("\n");
    while (( current->base_frame_no > _first_frame_no) || ( _first_frame_no >= current->base_frame_no + current->n_frames))
    {
	    if(current->next ==NULL)
	    {
		    return;
	    }
	    else
	    {
	            current = current->next;
	    }
    }

    // check if frame is head of sequence 
    // the get the index of head frame  in bitmap
    unsigned int bitmap_index = (_first_frame_no - current->base_frame_no)/4;
    unsigned char checker_head = 0x40 >>(((_first_frame_no - current->base_frame_no)%4)*2);
    unsigned char checker_reset = 0xc0 >> (((_first_frame_no - current->base_frame_no)%4*2));
    unsigned char checker_allocated = 0xc0;
    unsigned int rest;
    unsigned long next_frame_no;
    

    // checke whether the input is head of sequence : 01 ( it should be )
    if (((current->bitmap[bitmap_index] & checker_reset)^checker_head)==0)
    {
         Console::puts("the index of input first fram in bitmap is "); Console::puti(bitmap_index); Console::puts("\n");
         Console::puts("the input of first frame is head it's value int bitmap is "); Console::puti(current->bitmap[bitmap_index]); Console::puts("\n");
	 //reset to free 
	 current->bitmap[bitmap_index] = current->bitmap[bitmap_index] &(~checker_reset) | 0x0;
	 Console::puts("After reset the value of head is ( should be 0) "); Console::puti(current->bitmap[bitmap_index]); Console::puts("\n");

    }
    // deal with the rest of frames which value should be 11 (allocated right now )
    
    next_frame_no = _first_frame_no+1;
    current->nFreeFrames++;
    rest = ( (_first_frame_no+1) - current->base_frame_no)/4;
    while (rest < (current->n_frames/4))	    
    {
	   // Console::puts("current rest is : "); Console::puti(rest); Console::puts("\n");
	    rest = ( next_frame_no - current->base_frame_no)/4;
	    checker_allocated = 0xc0 >> ((next_frame_no%4)*2);
	    checker_reset = 0xc0 >> ((next_frame_no%4)*2);
	    // the reset is allocated
	    if ( (current->bitmap[rest] & checker_allocated) == checker_allocated)
            {
		    //Console::puts("the reset is allocated"); Console::puti(current->bitmap[rest]); Console::puts("\n");
		    current->bitmap[bitmap_index] = current->bitmap[bitmap_index] &(~checker_reset);
                    //Console::puts("after the reset allocated should be 0"); Console::puti(current->bitmap[bitmap_index]); Console::puts("\n");
	    }
	    else
	    {
		    break;
	    }
	    next_frame_no++;

    }
    current->nFreeFrames += current->n_frames -1;
}
    



unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{
	// 4K = 4096
	return (_n_frames*2/ 8*4096) + ( (_n_frames*2) % (8*4096) > 0 ? 1 : 0);
}
