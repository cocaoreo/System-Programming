#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mm.h"
#include "memlib.h"

#define WSIZE 4 /* Word and header/footer size (bytes) */            // line:vm:mm:beginconst
#define DSIZE 8                                                      /* Double word size (bytes) */
#define CHUNKSIZE (1 << 12) /* Extend heap by this amount (bytes) */ // line:vm:mm:endconst
#define MINSIZE 16
#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc)) // line:vm:mm:pack

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))              // line:vm:mm:get
#define PUT(p, val) (*(unsigned int *)(p) = (val)) // line:vm:mm:put

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7) // line:vm:mm:getsize
#define GET_ALLOC(p) (GET(p) & 0x1) // line:vm:mm:getalloc

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)                      // line:vm:mm:hdrp
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE) // line:vm:mm:ftrp

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE))) // line:vm:mm:nextblkp
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE))) // line:vm:mm:prevblkp
/* $end mallocmacros */

#define PREV_P(bp) (*(void **)(bp + WSIZE))
#define NEXT_P(bp) (*(void **)(bp))
/* Global variables */
static char *heap_listp = 0; /* Pointer to first block */
static char *root = 0;       // freelist root

team_t team = {
};

/* Function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);

static void addfreelist(void *bp);
static void removefreelist(void *bp);
size_t checksize(size_t size);

int mm_init(void)
{
  if ((heap_listp = mem_sbrk(6 * WSIZE)) == (void *)-1) // line:vm:mm:begininit
    return -1;
  PUT(heap_listp, 0);                              /* Alignment padding */
  PUT(heap_listp + (1 * WSIZE), PACK(MINSIZE, 1)); /* Prologue header */
  PUT(heap_listp + (2 * WSIZE), 0);                /* next */
  PUT(heap_listp + (3 * WSIZE), 0);                /* prev */
  PUT(heap_listp + (4 * WSIZE), PACK(MINSIZE, 1)); /* Prologue footer */
  PUT(heap_listp + (5 * WSIZE), PACK(0, 1));       /* Epilogue header */
  heap_listp += (2 * WSIZE);                       // line:vm:mm:endinit
  root = heap_listp;
  if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
    return -1;
  return 0;
}

void *mm_malloc(size_t size)
{
  size_t asize;      /* Adjusted block size */
  size_t extendsize; /* Amount to extend heap if no fit */
  char *bp;

  if (heap_listp == 0)
  {
    mm_init();
  }
  if (size == 0)
    return NULL;

  /* Adjust block size to include overhead and alignment reqs. */
  asize = checksize(size);

  /* Search the free list for a fit */
  if ((bp = find_fit(asize)) != NULL)
  {                   // line:vm:mm:findfitcall
    place(bp, asize); // line:vm:mm:findfitplace
    return bp;
  }

  /* No fit found. Get more memory and place the block */
  extendsize = MAX(asize, CHUNKSIZE); // line:vm:mm:growheap1
  if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
    return NULL;    // line:vm:mm:growheap2
  place(bp, asize); // line:vm:mm:growheap3
  return bp;
}

void mm_free(void *bp)
{
  /* $end mmfree */
  if (bp == 0)
    return;

  // printf("free\n");
  /* $begin mmfree */
  size_t size = GET_SIZE(HDRP(bp));
  PUT(HDRP(bp), PACK(size, 0));
  PUT(FTRP(bp), PACK(size, 0));
  coalesce(bp);
}

static void *coalesce(void *bp)
{
  size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp))) || PREV_BLKP(bp) == bp;
  size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
  size_t size = GET_SIZE(HDRP(bp));

  if (prev_alloc && next_alloc)
  { /* Case 1 */
    // printf("c1\n");
    addfreelist(bp);
  }

  else if (prev_alloc && !next_alloc)
  { /* Case 2 */
    // printf("c2\n");
    removefreelist(NEXT_BLKP(bp));
    size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    addfreelist(bp);
  }

  else if (!prev_alloc && next_alloc)
  { /* Case 3 */
    // printf("c3\n");
    removefreelist(PREV_BLKP(bp));
    size += GET_SIZE(HDRP(PREV_BLKP(bp)));
    PUT(FTRP(bp), PACK(size, 0));
    bp = PREV_BLKP(bp);
    PUT(HDRP(bp), PACK(size, 0));
    addfreelist(bp);
  }

  else
  { /* Case 4 */
    // printf("c4\n");
    removefreelist(NEXT_BLKP(bp));
    removefreelist(PREV_BLKP(bp));
    size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
    PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
    bp = PREV_BLKP(bp);
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    addfreelist(bp);
  }

  return bp;
}

void *mm_realloc(void *ptr, size_t size)
{
  size_t oldsize;
  size_t newsize;
  void *newptr;

  /* If size == 0 then this is just free, and we return NULL. */
  if (size == 0)
  {
    mm_free(ptr);
    return 0;
  }

  /* If oldptr is NULL, then this is just malloc. */
  if (ptr == NULL)
  {
    return mm_malloc(size);
  }

  /* Copy the old data. */
  oldsize = GET_SIZE(HDRP(ptr));
  newsize = checksize(size);

  // printf("oldsize: %d, newsize: %d %d\n", oldsize, newsize, oldsize-newsize);
  if (oldsize > newsize)
  {
    // if((oldsize - newsize) >= (2*DSIZE))
    // {
    // 	PUT(HDRP(ptr),PACK(newsize, 1));
    // 	PUT(FTRP(ptr),PACK(newsize, 1));
    // 	PUT(HDRP(NEXT_BLKP(ptr)),PACK(oldsize-newsize, 0));
    // 	PUT(FTRP(NEXT_BLKP(ptr)),PACK(oldsize-newsize, 0));
    // 	addfreelist(NEXT_BLKP(ptr));
    // }
    return ptr;
  }

  else
  {
    if (!GET_ALLOC(HDRP(NEXT_BLKP(ptr))))
    {
      if (GET_SIZE(HDRP(NEXT_BLKP(ptr))) + oldsize >= newsize)
      {
        removefreelist(NEXT_BLKP(ptr));
        newsize = GET_SIZE(HDRP(NEXT_BLKP(ptr))) + oldsize;
        PUT(HDRP(ptr), PACK(newsize, 1));
        PUT(FTRP(ptr), PACK(newsize, 1));
        return ptr;
      }

      else
      {
        newptr = mm_malloc(newsize);
        memcpy(newptr, ptr, oldsize);
        mm_free(ptr);
        return newptr;
      }
    }

    else
    {
      newptr = mm_malloc(newsize);
      memcpy(newptr, ptr, oldsize);
      mm_free(ptr);
      return newptr;
    }
  }
}

static void *extend_heap(size_t words)
{
  char *bp;
  size_t size;

  /* Allocate an even number of words to maintain alignment */
  size = (((words) + 7) >> 3) << 5;

  if ((long)(bp = mem_sbrk(size)) == -1)
    return NULL; // line:vm:mm:endextend

  /* Initialize free block header/footer and the epilogue header */
  PUT(HDRP(bp), PACK(size, 0)); /* Free block header */           // line:vm:mm:freeblockhdr
  PUT(FTRP(bp), PACK(size, 0)); /* Free block footer */           // line:vm:mm:freeblockftr
  PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */ // line:vm:mm:newepihdr

  /* Coalesce if the previous block was free */
  return coalesce(bp); // line:vm:mm:returnblock
}

static void place(void *bp, size_t asize)
/* $end mmplace-proto */
{
  size_t csize = GET_SIZE(HDRP(bp));
  // printf("csize: %d, asize: %d\n", csize, asize);

  if ((csize - asize) >= MINSIZE)
  {
    removefreelist(bp);
    PUT(HDRP(bp), PACK(asize, 1));
    PUT(FTRP(bp), PACK(asize, 1));
    bp = NEXT_BLKP(bp);
    PUT(HDRP(bp), PACK(csize - asize, 0));
    PUT(FTRP(bp), PACK(csize - asize, 0));
    addfreelist(bp);
  }
  else
  {
    removefreelist(bp);
    PUT(HDRP(bp), PACK(csize, 1));
    PUT(FTRP(bp), PACK(csize, 1));
  }
}

static void *find_fit(size_t asize)
{
  void *bp;

  for (bp = root; !GET_ALLOC(HDRP(bp)); bp = NEXT_P(bp))
  {
    if ((asize <= GET_SIZE(HDRP(bp))))
    {
      return bp;
    }
  }

  return NULL; /* No fit */
}

static void addfreelist(void *bp) // LIFO
{
  NEXT_P(bp) = root;
  PREV_P(bp) = NULL;
  PREV_P(root) = bp;
  root = bp;
}

static void removefreelist(void *bp)
{
  if (PREV_P(bp) == NULL)
  { // 처음
    root = NEXT_P(bp);
    PREV_P(root) = NULL;
  }

  else if (NEXT_P(bp) == NULL)
  { // 마지막
    NEXT_P(PREV_P(bp)) = NULL;
  }

  else
  { // 중간
    PREV_P(NEXT_P(bp)) = PREV_P(bp);
    NEXT_P(PREV_P(bp)) = NEXT_P(bp);
  }
}

size_t checksize(size_t size)
{
  if (size <= DSIZE)
    return MINSIZE;
  else
    return DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);
}