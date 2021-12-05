/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file_system.H"
#include "file.H"
/*--------------------------------------------------------------------------*/
/* CLASS Inode */
/*--------------------------------------------------------------------------*/

/* You may need to add a few functions, for example to help read and store 
   inodes from and to disk. */

/*--------------------------------------------------------------------------*/
/* CLASS FileSystem */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/


void Inode::inode_read( unsigned char * _buf)
{
	fs->disk->read(0, _buf);
}

void Inode::inode_write(unsigned char * _buf)
{
	fs->disk->write(0, _buf);
}




FileSystem::FileSystem() {

	inodes = new Inode[MAX_INODES];
	count = 0 ;
	number_of_free_blocks = 0; //after the fromat will == disk_size/block_size
	number_of_inodes = 0; // once file created number of inode plus 1 
        Console::puts("In file system constructor.\n");
    
}

FileSystem::~FileSystem() {
    Console::puts("unmounting file system\n");
    /* Make sure that the inode list and the free list are saved. */
    unsigned char *buffer = new unsigned char[512];
    memset(buffer,0,512);
    //write inode list into disk 
    for(int i =0; i< MAX_INODES; i++)
    {
	    memcpy( buffer+(i*sizeof(Inode)), &inodes[i],sizeof(Inode));
    }
    disk->write(0,buffer);
    //reset buffer 
    memset(buffer,0,512);
    for(int i=0;i< size/512; i++)
    {
	    memcpy(&buffer[i],&free_blocks[i],1);
    }
    disk->write(1,buffer);




    //assert(false);
}


/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/


void FileSystem::free_block_read( SimpleDisk * _disk, unsigned char * _buf)
{
	_disk->read(1, _buf);
}

void FileSystem::free_block_write(SimpleDisk * _disk,unsigned char * _buf)
{
	_disk->write(1,_buf);
}


bool FileSystem::Mount(SimpleDisk * _disk) {
    Console::puts("mounting file system from disk\n");

    /* Here you read the inode list and the free list into memory */
    if ( count !=0)
    {
	    return false;
    }
    else
    {
	    count = 1; //one file system per disk 
	    disk = _disk; // connect to disk 
	    unsigned char *buffer = new unsigned char[512];
	    memset(buffer,0,512); // initialize ram for inode list 
	    disk->read(0,buffer); // read first block for inode list 
	    // initialize inode list 
	    for (int i=0; i<MAX_INODES; i++)
	    {
		    // let files in this file system point to this file system 
		    inodes[i].fs = this;
		    //copy data from inode list to buffer
		    memcpy( buffer+(i*sizeof(Inode)), &inodes[i],sizeof(Inode));
	    }
	    
	    //write buffer back to disk => aka write inodes list into disk 
	    disk->write(0,buffer); // write back 
	    
	    // reset buffer for free block list 
	    memset(buffer,0,512);
	   //read data from first block for free block list  
	    disk->read(1,buffer); 

	    // copy data from free block list to buffer 
	    for(int j =0; j < number_of_free_blocks; j++)
	    {
		    memcpy(&buffer[j],&free_blocks[j],1);
	    }
	    //write free block list into  disk 
	    disk->write(1,buffer);
	    Console::puts("Mount file system done!!!\n");
	    return true;
	    // at this point i have connected the disk to file system 
	    // the inode list is empty; the free block list = (total free blocks - 2)
	    // i have writed both lists into disk 

    }
}


bool FileSystem::Format(FileSystem *fs, SimpleDisk * _disk, unsigned int _size) { // static!
       Console::puts("formatting disk\n");
    /* Here you populate the disk with an initialized (probably empty) inode list
       and a free list. Make sure that blocks used for the inodes and for the free list
       are marked as used, otherwise they may get overwritten. */
       unsigned char *buffer = new unsigned char[512]; 
        fs->disk = _disk;
        fs->size = _size;
	fs->number_of_free_blocks = _size/512; 
	fs->free_blocks =  (unsigned char *) (fs->number_of_free_blocks);
	
	// initialize buffer 
	memset(buffer,0,512);
	// clean up all data in this file system 
	for ( int i=0; i<fs->number_of_free_blocks; i++)
	{
		fs->disk->write(i,buffer);
	}

	// initialize fist two blocks b0 = inode b1 = free_block _list 
	// mark first two block as used aka=1
	fs->free_blocks[0] = 0x1;
	fs->free_blocks[1] = 0x1;
	//mark the rest of block to 0 
	for(int j = 2; j<fs->number_of_free_blocks; j++)
	{
		fs->free_blocks[j] = 0;
	}
	for( int i =0; i < fs->number_of_free_blocks;i++)
	{
	Console::puti(fs->free_blocks[i]);
	}
	fs->number_of_free_blocks-=2;
	return true;
	Console::puts("Format file system done\n");

	// at this point i have initialized the inode list (empty) 
	// and i have initialized the free block list ( 2 used rest free)
	// but i have not write these into disk will do so in mount
	
}


	
int FileSystem::GetFreeInode()
{
	if( number_of_inodes != MAX_INODES)
	{
		//return next free inode number 
		return number_of_inodes;
	
	}
 	else
	{
		return MAX_INODES;
	}
	
}

int FileSystem::GetFreeBlock()
{
	if(number_of_free_blocks>0)
	{
		return (size/512) - number_of_free_blocks;
	}
	else
	{
		return 0;
	}
}

		



Inode * FileSystem::LookupFile(int _file_id) {
    Console::puts("looking up file with id = "); Console::puti(_file_id); Console::puts("\n");
    /* Here you go through the inode list to find the file. */

    for( int i =0; i <MAX_INODES; i++)
    {
	    if(inodes[i].id == _file_id)
	    {
		    return &inodes[i];
	    }
    }
    return NULL;


}

bool FileSystem::CreateFile(int _file_id) {
    Console::puts("creating file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* Here you check if the file exists already. If so, throw an error.
       Then get yourself a free inode and initialize all the data needed for the
       new file. After this function there will be a new file on disk. */
    for( int i =0; i <MAX_INODES; i++)
    {
            if(inodes[i].id == _file_id)
            {
                    Console::puts("file with id: exists already"); Console::puti(_file_id); Console::puts("\n");
		    return false;
		    //assert(false);
            
    	    }
    }
     // we do not actually allocate a new file object 
    // since i will allocate a new file in kernel 
    //File * new_file = new File(this,_file_id);

    int next_free_block = GetFreeBlock(); // next free block 
    int next_free_inode = GetFreeInode(); // next free inode
    // inititalize data in inode  
    inodes[next_free_inode].id = _file_id;
    inodes[next_free_inode].current_block = next_free_block;
    inodes[next_free_inode].file_size =0;
    inodes[next_free_inode].current_position = 0;
    number_of_inodes +=1;
    number_of_free_blocks-=1;
    // initialize buffer for write in disk 
    unsigned char *buffer = new unsigned char[512];
    memset(buffer,0,512);
    // read inode list into buffer 
    disk->read(0,buffer);
    //upadte new inode we just initialize and store it into buffer 
    memcpy(buffer+(next_free_inode*sizeof(Inode)), &inodes[next_free_inode], sizeof(Inode));
    //write updated inode list back to disk
    disk->write(0,buffer);

    //Now we deal with free block list 
    // reset buffer 
    memset(buffer,0,512);
    // read old  free block list from disk into buffer 
    disk->read(1,buffer);
    //update data  in buffer and free block list 
    free_blocks[next_free_block] = 0x1;
    buffer[next_free_block] = 0x1;
    // write free block list back to disk
    disk->write(1,buffer);
    Console::puts("Create file done\n");
    return true;
    // at this point i have update inode list which have  1. current poistion=0 
    // 							  2. current block = next available block 
    // 							  3.size of file = 0 
    // 							  4. file name aka id = gived id 
    //Beside i have update the free block list and two number in file system (inode_size & fb_size)
    //in the end i have write both new data into disk  		    
}

bool FileSystem::DeleteFile(int _file_id) {
    Console::puts("deleting file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* First, check if the file exists. If not, throw an error. 
       Then free all blocks that belong to the file and delete/invalidate 
       (depending on your implementation of the inode list) the inode. */
    

    // check the file with given id exist or not 
    for( int i =0; i <MAX_INODES; i++)
    {
   	    // if found akak exist
            if(inodes[i].id == _file_id)
            {
		    int file_in_block = inodes[i].current_block;
		    delete inodes[i].file_address;
		    //delete this inode 
		    delete &inodes[i];
		    //move the rest of i node forward
		    for( i;i<=number_of_inodes; i++)
		    {
			    //Console::puts("id:");Console::puti(inodes[i].id);Console::puts("\n");
			    //Console::puts("i :");Console::puti(i);Console::puts("\n");

 			    inodes[i] = inodes[i+1];
		    }
		    unsigned char * buffer = new unsigned char[512];
		    // upadate i node list  
		    for( int i =0 ; i<MAX_INODES; i++)
            	    {
                    	memcpy( buffer+(i*sizeof(Inode)), &inodes[i],sizeof(Inode));
            	    }
                    //write indoe list back into disk
            	    disk->write(0,buffer);
                    
		    number_of_inodes--;
		    number_of_free_blocks++;
		    // update free block list 
		    free_blocks[file_in_block] = 0 ;
		    
		     
		    memset(buffer,0,512);
		    disk->read(1,buffer);
		    buffer[file_in_block] = 0;
		    disk->write(1,buffer);
		    
		    return true;

		    // at this point i have delete file object delete inode mark free block as 
		    // free and write both list into disk 
            }

    }
    Console::puts("file with id: exists already"); 
    Console::puti(_file_id); Console::puts("\n");
    return false;
    


}
