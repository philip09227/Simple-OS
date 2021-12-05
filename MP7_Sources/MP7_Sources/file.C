/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
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
#include "file.H"
#include "file_system.H"
extern FileSystem *FILE_SYSTEM;
/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File(FileSystem *_fs, int _id) {

	Console::puts(" start open file\n");
    	fs = _fs;
  	file_size = 0;
  	file_id = _id;
 	current_position = 0;
	int i =0; 
	
	while(i < fs->number_of_inodes)
	{

		if(fs->inodes[i].id == _id)
		{
			Console::puti(fs->inodes[i].id);
			inode = &(fs->inodes[i]);
			current_block = inode->current_block;
			inode->file_address = this;
			break;
		}
		i++;
	}


    Console::puts("Open file. done ");Console::puti(_id);
    Console::puts("\n");

    
}

File::~File() {
    Console::puts("Closing file.\n");

    inode -> id = file_id;
    inode -> current_position = current_position;
    inode -> fs = fs;
    inode -> file_size = file_size;
    inode->file_address = this;
    unsigned char *buffer = new unsigned char[512];
    memset(buffer , 0 , 512);

    //update inode list into disk
    for( int i =0 ; i< (512/sizeof(Inode)); i++)
    {
          memcpy( buffer+(i*sizeof(Inode)), &fs->inodes[i],sizeof(Inode));
    }
    fs->disk->write(0,buffer);
    



    


    /* Make sure that you write any cached data to disk. */
    /* Also make sure that the inode in the inode list is updated. */

}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char *_buf) {

    Console::puts("reading from file\n");
    int read_n_char = _n;
    int file_size = inode->file_size;
    int already_read = 0; 
    memset(block_cache,0,512);
    fs->disk->read(inode->current_block, block_cache);
    Console::puts("current_block is "); Console::puti(inode->current_block); Console::puts("\n");
    Console::puts("current postion  "); Console::puti(current_position); Console::puts("\n");
    Console::puts("current postion  "); Console::puti(file_size); Console::puts("\n");


    while ( already_read < read_n_char)// && inode->current_position <= file_size)
    {
	    Console::puti(already_read);

	    memcpy(_buf+already_read, block_cache+current_position,1);
	    already_read++;
	    if(current_position ==511)
	    {
		    //current_position = 0;
		    break;
	    }
	    else
	    {
		    current_position++;
	    }
    }
    Console::puti(already_read);
    return already_read;
    Console::puts(" file read done\n");
}



int File::Write(unsigned int _n, const char *_buf) {
    Console::puts("writing to file\n");
    int write_n_char = _n;
    int file_size = inode->file_size;
    int already_write=0;
    int current_block = (inode->current_block);
    Console::puts("XXXXXXXXXXXx\n");
    memset(block_cache,0,512);
    //read entire block into cache 
    Console::puts("YYYYYYYYYYYYY\n");
    Console::puti(current_block);
    fs->disk->read(inode->current_block,block_cache);
    Console::puts("ZZZZZZZZZZZZZ\n");

    //copy data into cache 
    if((512-file_size) <write_n_char)
    {
	    //space is not enough 
	    //copy data into cache
	    Console::puts("CCCCCCCCCCCCCc\n"); 
	    for (int i=current_position; i<512; i++)
	    {
		    Console::puts("DDDDDDDDDDDD\n");
		    memcpy(block_cache+current_position,_buf+already_write,1);
		    current_position++;
		    file_size++;
		    already_write++;
	    }
    }
    else
    {       
	    Console::puts("EEEEEEEEEEEEEEEEEEE\n");
	    //space in file is enough
	    for ( int j=already_write; j<write_n_char; j++)
	    {
		    Console::puts("FFFFFFFFFFFFFFFf\n");
		    memcpy(block_cache+current_position,_buf+already_write,1);
		    current_position++;
		    file_size++;
		    already_write++;
	    }
    }
    fs->disk->write(current_block,block_cache);
    Console::puts("file write done\n");
    return already_write;
}




void File::Reset() {
    Console::puts("resetting file\n");
    current_position = 0;
}

bool File::EoF() {
    Console::puts("checking for EoF\n");
    return current_position == 511;
}
