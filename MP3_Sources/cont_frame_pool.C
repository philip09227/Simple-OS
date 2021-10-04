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
    for (int i =0; i*4 < n_frames; i++){
	    bitmap[i] = 0x0;
    }
    
    // set first frame in fram pool to be head  
    if(info_frame_no==0){
	    bitmap[0] = 0x40;  //which is 01 for the first two bit 
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


unsigned char ContFramePool::get_state( unsigned int frame_no)
{
	// in which array total will be 128 
	unsigned int index = (frame_no-base_frame_no)/4;
	// which 2 bits we need to checke in this bitmap[i]
	// it also was the how many times the mask  should shift
	unsigned int shift = 6-(((frame_no-base_frame_no)%4)*2);
	unsigned int offset = (((frame_no-base_frame_no)%4)*2);
	unsigned char mask = 0xc0 >> offset;
        return (bitmap[index] & mask) >> shift;
        
}

void ContFramePool::set_state( unsigned int frame_no , unsigned char state)
{
	unsigned int index = ( frame_no - base_frame_no)/4;
	unsigned int shift = 6-(((frame_no -base_frame_no)%4)*2);
	unsigned int offset = (((frame_no-base_frame_no)%4)*2);
	unsigned char mask = 0xc0 >> offset;
	state = state << shift;
	bitmap[index] = (bitmap[index] & (~mask))| state;
}



unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{
    
    // Make sure we still have frames to allocated 
    assert(nFreeFrames >0);
    
    // first number of frame pool
    unsigned long first_frame_no = 0;
    unsigned long last_frame_no = 0;
    unsigned int i = base_frame_no;
    unsigned int j = 0;
    unsigned int frames_needed = _n_frames;
    unsigned int count =0;
    // use frame number to traverse  from first frame to last frames 
    while ((i < (base_frame_no+n_frames)) && (count < frames_needed))    
    {
	    
	    if (this->get_state(i) == 0x0)
	    {
		    count++;
	    }
	    //reset count if count < frame we want and there's a frame is not free
	    else
	    {
		    count = 0;
	    }
	    i++;
    
    }

    last_frame_no = i -1;
    first_frame_no = i - frames_needed;
    // set the head of sequence to head 01 
    this->set_state(first_frame_no,0x1);
    // set the rest of sequence to allocated 11
    for( j =first_frame_no+1 ; j < i; j++)
    {
	    this->set_state(j,0x3);
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
    for( int i =0; i < _n_frames; i++)
    {
	    this->set_state((_base_frame_no +i),0x2);
    }
    nFreeFrames -= _n_frames; 
}

void ContFramePool::release_frames(unsigned long _first_frame_no)
{
    unsigned int frame_no = _first_frame_no;
    unsigned int release_frame_amount=0;
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

     // if input is not head
    if ( current->get_state(frame_no) != 0x1)
    {
	    assert(false);
    }
    //set the head of sequence to free
    current->set_state(frame_no,0x0);
    release_frame_amount++;
   // set the rest of sequence to free
    frame_no++;
    while( current->get_state(frame_no) != 0x1)
    {
	   // if there's a free frame the sequence finish
	    if (current->get_state(frame_no) == 0x0)
	   {
		   break;
	   }
	   current->set_state(frame_no,0x0);
	   release_frame_amount++;
	   frame_no++;
    } 
  
    Console::puts( " This time the number of frames we relase  is : "); Console::puti(release_frame_amount);
    Console::puts("\n");
    current->nFreeFrames += release_frame_amount;
    Console::puts( " Now the total free frames are "); Console::puti(current->nFreeFrames);
    Console::puts("\n");

}
    



unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{
	// 4K = 4096
	return (_n_frames/ 4*4096) + ( (_n_frames) % (4*4096) > 0 ? 1 : 0);
}
