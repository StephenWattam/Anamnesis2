
#include <stdint.h>
#include <pthread.h>

#ifndef __A2
#define __A2

#define false 0
#define true 1

#define A2_DEFRAG_ON_ALLOCATE	0x01
#define	A2_DEFRAG_ON_FREE	0x02

typedef int a2var;


//typedef int a2var, something;


typedef struct a2_block{
    void	      * ptr;
    uint32_t		length;

    a2var		next;
    a2var		prev;
} a2_blk;

typedef struct a2_repo{
    
    a2_blk	      * cache;
    void	      * base_ptr;
    
    uint32_t		cache_limit;
    long int		id_count;   //goes round and round cache limit
    uint32_t		cache_used;

    uint32_t		mem_limit;
    uint32_t		mem_used;
    
    uint32_t		options;
    pthread_mutex_t	a2_mutex;

    a2var		front;
    
} a2_repo;




/** Writes 0s across the available memory space, and creates partitions. */
void a2_init(a2_repo *	repo, 
	    uint32_t	p_mem_limit, 
	    void *	p_base_ptr, 
	    uint32_t	p_cache_limit, 
	    a2_blk *	p_cache, 
	    int		p_options);





void a2_print_debug_repo_layout(a2_repo * repo);
void a2_print_debug_memory_layout(a2_repo * repo);
a2var a2_malloc(a2_repo * repo, uint32_t want);
a2_blk a2_get_blk(a2_repo * r, a2var var);
a2_blk * a2_get_blkptr(a2_repo * r, a2var var);
int a2_free(a2_repo * r, a2var item);
void a2_free_all(a2_repo * r);
int a2_count();
int a2_compact();

#endif
