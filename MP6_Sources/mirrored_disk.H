#ifndef _MIRRORED_DISK_H
#define _MIRRORED_DISK_H




#include "simple_disk.H"
#include "blocking_disk.H"
struct mirrored_queue
{
        Thread * thread;
        struct mirrored_queue* next;
};
class MirroredDisk :public BlockingDisk{

public:
	BlockingDisk* MASTER_DISK;
	BlockingDisk* DEPENDENT_DISK;

        bool mirrored_is_ready();
  	Thread* mirrored_dequeue();
 	void mirrored_enqueue(Thread* _thread);
  	int mirrored__queue_size();
   	MirroredDisk(DISK_ID _disk_id, unsigned int _size);
        virtual void issue_operation(DISK_OPERATION _op, unsigned long _block_no, DISK_ID disk_id);
      

   	virtual void mirrored_read(unsigned long _block_no, unsigned char * _buf);
   /* Reads 512 Bytes from the given block of the disk and copies them 
      to the given buffer. No error check! */

   	virtual void mirrored_write(unsigned long _block_no, unsigned char * _buf);
   /* Writes 512 Bytes from the buffer to the given block on the disk. */
protected:
        virtual void  mirrored_wait_until_ready();
};
#endif 
