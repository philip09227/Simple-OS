#ifndef _MIRRORED_DISK_H
#define _MIRRORED_DISK_H




#include "simple_disk.H"
#include "blocking_disk.H"

class MirroredDisk :public BlockingDisk{

public:
	BlockingDisk* MASTER_DISK;
	BlockingDisk* DEPENDENT_DISK;

   	MirroredDisk(DISK_ID _disk_id, unsigned int _size);
        virtual void issue_operation(DISK_OPERATION _op, unsigned long _block_no, DISK_ID disk_id);
      

   	virtual void read(unsigned long _block_no, unsigned char * _buf);
   /* Reads 512 Bytes from the given block of the disk and copies them 
      to the given buffer. No error check! */

   	virtual void write(unsigned long _block_no, unsigned char * _buf);
   /* Writes 512 Bytes from the buffer to the given block on the disk. */
protected:
        virtual void  wait_until_ready();
};
#endif 
