#include "assert.H"
#include "utils.H"
#include "console.H"
#include "blocking_disk.H"
#include "scheduler.H"
#include "simple_disk.H"
#include "thread.H"
#include "mirrored_disk.H"

extern Scheduler* SYSTEM_SCHEDULER;

MirroredDisk::MirroredDisk(DISK_ID _disk_id, unsigned int _size)
  : BlockingDisk(_disk_id, _size) {
          MASTER_DISK = new BlockingDisk(DISK_ID::MASTER, _size);
	  DEPENDENT_DISK = new BlockingDisk(DISK_ID::DEPENDENT, _size);

}
void MirroredDisk::issue_operation(DISK_OPERATION _op, unsigned long _block_no,DISK_ID disk_id) {

  Machine::outportb(0x1F1, 0x00); /* send NULL to port 0x1F1         */
  Machine::outportb(0x1F2, 0x01); /* send sector count to port 0X1F2 */
  Machine::outportb(0x1F3, (unsigned char)_block_no);
                         /* send low 8 bits of block number */
  Machine::outportb(0x1F4, (unsigned char)(_block_no >> 8));
                         /* send next 8 bits of block number */
  Machine::outportb(0x1F5, (unsigned char)(_block_no >> 16));
                         /* send next 8 bits of block number */
  unsigned int disk_no = disk_id == DISK_ID::MASTER ? 0 : 1;
  Machine::outportb(0x1F6, ((unsigned char)(_block_no >> 24)&0x0F) | 0xE0 | (disk_no << 4));
                         /* send drive indicator, some bits,
                            highest 4 bits of block no */

  Machine::outportb(0x1F7, (_op == DISK_OPERATION::READ) ? 0x20 : 0x30);

}



void MirroredDisk::mirrored_wait_until_ready()
{
        if(!MASTER_DISK->is_ready() || !DEPENDENT_DISK->is_ready())
        {
                // devices are not ready therefore, add current thread which is calling I/O operation into device queue
                Thread * current_thread = Thread::CurrentThread();
                MASTER_DISK->enqueue(current_thread);
                queue_size+=1;
                //SYSTEM_SCHEDULER->resume(current_thread);
                Console::puts(" device is not ready  then yield \n");
                SYSTEM_SCHEDULER->yield();

        }
}

void MirroredDisk::mirrored_read(unsigned long _block_no, unsigned char * _buf) {
        // check the status of device whether it is ready for I/O opearation 
        //wait_until_ready();        
	issue_operation(DISK_OPERATION::READ,_block_no,DISK_ID::MASTER);
	issue_operation(DISK_OPERATION::READ,_block_no,DISK_ID::DEPENDENT);
        //SYSTEM_SCHEDULER->flag = true;
        mirrored_wait_until_ready();
        //MASTER_DISK->read(_block_no,_buf);
	//DEPENDENT_DISK->read(_block_no,_buf);
	int i;
        unsigned short tmpw;
        for ( i = 0; i< 256; i++)
        {
                tmpw = Machine::inportw(0x1F0);
                _buf[i*2] = (unsigned char) tmpw;
                _buf[i*2+1] = (unsigned char) (tmpw >> 8);

        }
        //SYSTEM_SCHEDULER->flag = false;
        Console::puts("MIRRORED Disk read done\n");


}


void MirroredDisk::mirrored_write(unsigned long _block_no, unsigned char * _buf) {
        //wait_until_ready();
        //issue_operation(DISK_OPERATION::WRITE , _block_no, DISK_ID::MASTER);
	//issue_operation(DISK_OPERATION::WRITE , _block_no,DISK_ID::DEPENDENT);
        //SYSTEM_SCHEDULER->flag = true;
       // mirrored_wait_until_ready();
	MASTER_DISK->write(_block_no,_buf);
	DEPENDENT_DISK->write(_block_no,_buf);
        /*
	int i;
	unsigned short tmpw;
        for ( i = 0; i< 256; i++)
        {
                for( i=0; i<256; i++)
                {
                        tmpw = _buf[2*i] | ( _buf[2*i+1] << 8);
                        Machine::outportw(0x1F0,tmpw);
                }
        }*/
        Console::puts("MIRRORED Disk write done\n");
       // SYSTEM_SCHEDULER->flag = false;
}

