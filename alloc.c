/** @file alloc.c */
//|========================================
//|Program:				alloc_test
//|File Name:			alloc_test.c
//|Author:				Rohan Jyoti
//|Purpose:				CS241 Malloc
//|________________________________________

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#ifndef alloc_h
#define alloc_h
void *calloc(size_t nmemb, size_t size);
void *malloc(size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);

/* Use
 | Explicit free and allocated lists 
 | adjacency matrix
 */

typedef struct _fListNode
{
	struct _fListNode* nextNode;
} fListNode;

typedef struct _mBlock_t
{
	int *mList; //the allocated blocks list
	fListNode **fList; //the free list
	int *num_entries; //number of allocated blocks
	int fListCount; //number of free blocks
} mBlock_t;

mBlock_t* mBlock;
#define EMPTY 0

void init_Lists(mBlock_t *mB);
int isFree(int index);
#define NOT_FREE_AT_INDEX 0
#define FREE_AT_INDEX 1

void *fListRemove(int index);
void fListAdd(int* pointer, int index, int size);
#define soMD 4
#define LIST_MAX 24
#define LIST_THRESHOLD LIST_MAX/2

#define ERR_COMP 1.21					
#define FO_COMP 19

size_t getSize(void *in_ptr);
void *setSize(void *in_ptr, size_t size, int opt);
#define AT_HEAD 0
#define AT_OFFSET 1

int F_COMP;	
float M_POW(int a, int b);
#endif

/**
 * Allocate space for array in memory
 * 
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 * 
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size)
{
	/* Note: This function is complete. You do not need to modify it. */
	void *ptr = malloc(num * size);
	
	if (ptr)
		memset(ptr, 0x00, num * size);

	return ptr;
}


/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */

/*******************************************************************************
 * @author	: Rohan Jyoti
 * @name	: malloc
 * @param	: size_t
 * @return	: void * to allocated block
 * @purpose	: allocate/return space on the heap
 ******************************************************************************/
void *malloc(size_t size)
{
	if(size==0) return NULL;
	if(mBlock == NULL)
	{
		fprintf(stderr, "Custom malloc engine initiated rev 98\n");
		//sbrk a new mBlock buffer where buffSize = mBlock_t + space for fList + space for mList
		int listMaxBound = LIST_MAX + 1;
		int buffSize = sizeof(mBlock_t)+(sizeof(fListNode*)*listMaxBound)+sizeof(int)*listMaxBound;
		mBlock=sbrk(buffSize);
		init_Lists(mBlock);
	}
	
	void *ptr1 = NULL;
	int index1, index2, listIndex; //reusable
	F_COMP = (int)(2*M_POW(2,9));
	if(mBlock->fListCount > 0)
	{
		for(index1 = 0; index1 < LIST_THRESHOLD; index1++)
		{
			listIndex = soMD * (int)(M_POW(2,index1));
			if(listIndex >= (int)size)
			{
				if(isFree(index1))
					return fListRemove(index1);
				else break;
			}
		}
		
		if(index1 >= LIST_THRESHOLD)
		{
			size_t ptr_size;
			for(; index1 <= LIST_MAX; index1++)
			{
				fListNode *prev = NULL;
				fListNode *fLN_at_index =(mBlock->fList)[index1];
				while(fLN_at_index)
				{
					ptr1 = fLN_at_index;
					ptr_size = getSize(fLN_at_index);
					if(size <= ptr_size)
					{
						if(size == ptr_size)
						{
							if(!prev)
								mBlock->fList[index1]=fLN_at_index->nextNode;
							else
								prev->nextNode=fLN_at_index->nextNode;
							mBlock->fListCount--;
							return ptr1;
						}
						else
						{
							//meaning size < ptr_size
							float t1 = ERR_COMP + (100.0/size);
							int t2 = (int)(size * t1);
							if((int)ptr_size < t2)
							{
								if(!prev)
									mBlock->fList[index1]=fLN_at_index->nextNode;
								else
									prev->nextNode=fLN_at_index->nextNode;
								mBlock->fListCount--;
								return ptr1;
							}
						}
					}
					prev=fLN_at_index;
					fLN_at_index=fLN_at_index->nextNode;
				}
			}
		}
	}
	
	//we reach here indicating that there is no free memory block that can accomodate size or there is no free memory
	for(index1 = 0; index1 < LIST_THRESHOLD; index1++)
	{
		listIndex = soMD * (int)(M_POW(2,index1));
		if(listIndex >= (int)size)
		{
			int soMD_index1 = soMD * (int)(M_POW(2,index1));
			int soMD_index1_minus_1 = soMD * (int)(M_POW(2,index1-1));
			int buffSize = (sizeof(int)+soMD_index1) * (1<<(mBlock->mList)[index1]) + (sizeof(int)+soMD_index1_minus_1);
			ptr1=sbrk(buffSize);
			if(ptr1 == (int*)-1) return NULL; //meaning we have run out of memory
			else
			{
				for(index2 = 0; index2 < (1<<(mBlock->mList)[index1])-1; index2++)
				{
					ptr1 = setSize(ptr1, listIndex, AT_HEAD);
					fListAdd(ptr1, index1, listIndex);
					char *tempy = (char*)ptr1;
					tempy+=listIndex;
					ptr1=tempy;
				}
				if(index1!=0)
				{
					ptr1 = setSize(ptr1, soMD_index1_minus_1, AT_HEAD);
					fListAdd(ptr1, index1-1, soMD_index1_minus_1);
					char *tempy = (char*)ptr1;
					tempy+=soMD_index1_minus_1;
					ptr1=tempy;
				}
				ptr1 = setSize(ptr1, listIndex, AT_HEAD);
				(mBlock->mList)[index1]++;
				int shiftMax = 8;
				int reduce_val = 4;			
				if((mBlock->mList)[index1]>shiftMax)
					(mBlock->mList)[index1]=reduce_val;			
				return ptr1;
			}
		}
	}
	
	for(; index1 < LIST_MAX; index1++)
	{
		int soMD_LT = soMD * (int)(M_POW(2,LIST_THRESHOLD));
		listIndex = soMD_LT + (index1 * F_COMP);
		if(listIndex >= (int)size)
		{
			int size_to_alloc = sizeof(int) + size;
			ptr1=sbrk(size_to_alloc);
			ptr1 = setSize(ptr1, size, AT_HEAD);
			return ptr1;
		}
	}
	
	if(index1 == LIST_MAX)
	{
		int size_to_alloc = sizeof(int) + size;
		ptr1=sbrk(size_to_alloc);
		if(ptr1 == (int*)-1) return NULL; //meaning we have run out of memory
		ptr1 = setSize(ptr1, size, AT_HEAD);
		return ptr1;
	}
	return ptr1;
}


/**
 * Deallocate space in memory
 * 
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */

/*******************************************************************************
 * @author	: Rohan Jyoti
 * @name	: free
 * @param	: void *
 * @return	: void
 * @purpose	: mark ptr as free, manage free list, return memory to OS accordingly
 ******************************************************************************/
void free(void *ptr)
{
	// "If a null pointer is passed as argument, no action occurs."
	if(!ptr) return;
	
	int index1, listIndex;
	void *ptr1 = ptr;
	int ptr_size = getSize(ptr);
	F_COMP = (int)(2*M_POW(2,9));
	int soMD_FO_C = soMD * (int)(M_POW(2,FO_COMP));
	
	if(ptr + ptr_size == sbrk(0) && ptr_size >= soMD_FO_C)
	{
		size_t buffSize = ptr_size - 4;
		sbrk((int)-buffSize);
		return ;
	}
	
	for(index1 = 0; index1 < LIST_THRESHOLD; index1++)
	{
		listIndex = soMD * (int)(M_POW(2, index1));
		if(listIndex >= ptr_size)
		{
			fListAdd(ptr1, index1, ptr_size);
			return;
		}
	}
	for(; index1 < LIST_MAX; index1++)
	{
		listIndex = (soMD * (int)(M_POW(2,LIST_THRESHOLD))) + (index1 * F_COMP);
		if(listIndex >= ptr_size)
		{
			fListAdd(ptr1, index1, ptr_size);
			return;
		}
	}
	fListAdd(ptr1, index1, ptr_size);
	return;
}


/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *    
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */

/*******************************************************************************
 * @author	: Rohan Jyoti
 * @name	: realloc
 * @param	: void *, size_t
 * @return	: void * to reallocated block
 * @purpose	: shrinks/expands allocated block
 ******************************************************************************/
void *realloc(void *ptr, size_t size)
{
	 // "In case that ptr is NULL, the function behaves exactly as malloc()"
	if (!ptr)
		return malloc(size);

	 // "In case that the size is 0, the memory previously allocated in ptr
	 //  is deallocated as if a call to free() was made, and a NULL pointer
	 //  is returned."
	if (!size)
	{
		free(ptr);
		return NULL;
	}

	void *ptr1 = ptr;
	size_t orig_size = getSize(ptr);
	if(orig_size == size) return ptr;
	
	int index1;
	if(ptr + orig_size == sbrk(0))
	{
		int table_shift = 29;
		int r_limit = 4938 * table_shift;
		if((int)orig_size > r_limit)
		{
			if(orig_size >= size && (int)size >= (soMD * (int)(M_POW(2,LIST_THRESHOLD))))
			{
				for(index1 = 0; index1 < LIST_MAX; index1++)
				{
					if((int)orig_size <= (soMD * (int)(M_POW(2, index1)))) break;
					else
					{
						size_t remain = (soMD * (int)(M_POW(2,index1))) - orig_size;
						sbrk((int)remain);		
						ptr1 = setSize(ptr1, soMD * (int)(M_POW(2,index1)), AT_OFFSET);
						return ptr;
					}
				}
			}
		}
	}
	
	if(orig_size > size && (soMD * (int)(M_POW(2,LIST_MAX))) >= (int)size) return ptr;
	void* ret_ptr = malloc(size);
	size_t size_to_copy;
	if(orig_size < size)
		size_to_copy = orig_size;
	else 
		size_to_copy = size;
	
	memcpy(ret_ptr, ptr, size_to_copy);	
	free(ptr);
	return ret_ptr;
}

/*******************************************************************************
 * @author	: Rohan Jyoti
 * @name	: init_Lists
 * @param	: mBlock_t *
 * @return	: void
 * @purpose	: initialize both the mList and fList (described above in declration)
 ******************************************************************************/
void init_Lists(mBlock_t *mB)
{
	int listMaxBound = LIST_MAX + 1;
	mB->fListCount=0;
	char* fLN=(char*)mB;
	fLN+=sizeof(mBlock_t);
	mB->fList=(fListNode**)fLN;
	fLN+=sizeof(fListNode*)*listMaxBound;
	mB->mList=(int*)fLN;
	
	int i;
	for(i = 0; i < listMaxBound; i++)
	{
		(mB->fList)[i]=NULL;
		(mB->mList)[i]=EMPTY;
	}
}

/*******************************************************************************
 * @author	: Rohan Jyoti
 * @name	: isFree
 * @param	: int
 * @return	: int (indicating whether or not it is free)
 * @purpose	: Find out if block on list is free
 ******************************************************************************/
int isFree(int index)
{
	if((mBlock->fList)[index]==0) return NOT_FREE_AT_INDEX;
	else return FREE_AT_INDEX;
}

/*******************************************************************************
 * @author	: Rohan Jyoti
 * @name	: fListAdd
 * @param	: ptr, index, size
 * @return	: void
 * @purpose	: add fListNode entry to fList
 ******************************************************************************/
void fListAdd(int *ptr, int index, int size)
{
	if(size == 4) ptr--;
	fListNode *fLN = (fListNode *)ptr;
	fLN->nextNode = (mBlock->fList)[index];
	(mBlock->fList)[index] = fLN;
	mBlock->fListCount++;
}

/*******************************************************************************
 * @author	: Rohan Jyoti
 * @name	: fListRemove
 * @param	: int
 * @return	: void *
 * @purpose	: remove a fListNode entry from fList
 ******************************************************************************/
void *fListRemove(int index)
{
	fListNode *fLN = (mBlock->fList)[index];
	(mBlock->fList)[index] = fLN->nextNode;
	mBlock->fListCount--;
	if(index == 0)
	{
		int *fLN_Head = (int *)fLN;
		fLN_Head[0] = soMD;
		fLN_Head++;
		fLN=(fListNode *)fLN_Head;
	}
	return (int *)fLN;
}

/*******************************************************************************
 * @author	: Rohan Jyoti
 * @name	: getSize
 * @param	: void *
 * @return	: size_t
 * @purpose	: get the size associated with the ptr
 ******************************************************************************/
size_t getSize(void *in_ptr)
{
	int *ptr1=(int*)in_ptr;
	ptr1--;
	size_t ptr_size=ptr1[0];
	ptr1++;
	return ptr_size;
}

/*******************************************************************************
 * @author	: Rohan Jyoti
 * @name	: setSize
 * @param	: void *, size_t, int
 * @return	: void *
 * @purpose	: updates/sets size in given pointer
 ******************************************************************************/
void *setSize(void *in_ptr, size_t size, int opt)
{
	int *ptr1 = (int *)in_ptr;
	if(opt == AT_HEAD)
		ptr1[0]=size;
	if(opt == AT_OFFSET)
	{
		ptr1--;
		ptr1[0] = size;
	}
	ptr1++;
	return ptr1;
}

/*******************************************************************************
 * @author	: Rohan Jyoti
 * @name	: M_POW
 * @param	: int, int
 * @return	: doublr
 * @purpose	: since we cannot edit the makefile to add -lm to link math lib
 ******************************************************************************/
float M_POW(int a, int b)
{
	if(b==0) return 1;
	else if(b==1) return a;
	int i, temp = a;
	for(i=0; i<=b-2; i++)
		temp = a * temp;
	return temp;
}

