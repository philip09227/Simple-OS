#ifndef OPERAND_H
#define OPERAND_H
#include "frame_pool.H"      /* MEMORY MANAGEMENT */
#include "mem_pool.H"


typedef long unsigned int size_t;

//replace the operator "new"
void * operator new (size_t size);

//replace the operator "new[]"
void * operator new[] (size_t size);

//replace the operator "delete"
void operator delete (void * p);

//replace the operator "delete[]"
void operator delete[] (void * p);

#endif 

