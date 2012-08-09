#include "a2.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/*
 * Anamnesis 2
 *
 * Author:	      Extreme Tomato
 * Project started:   Nov. 2008
 *
 * This is the main functional file of anamnesis.
 */


/*
 * Reference only, from a2.h
 *
 *typedef struct a2_block{
 *    void	      * ptr;
 *    uint32_t		length;
 *
 *    struct a2_block   * next;
 *    struct a2_block   * prev;
 *} a2_blk;
 *
 *typedef struct a2_repo{
 *    
 *    a2_blk	      * cache;
 *    void	      * base_ptr;
 *    
 *    uint32_t		cache_limit;
 *    long int		id_count;   //goes round and round cache limit
 *    uint32_t		cache_used;
 *
 *    uint32_t		mem_limit;
 *    uint32_t		mem_used;
 *    
 *    uint32_t		options;
 *    pthread_mutex_t	a2_mutex;
 *
 *    a2_blk	      * front;
 *    a2_blk	      * back;
 *    
 *} a2_repo;
 */

/* Gets the next id number for a repo, internal helper only */
long int get_next_id(a2_repo * r){
    while(r->cache[r->id_count].ptr)
	r->id_count = (r->id_count + 1) % r->cache_limit;
    return r->id_count; 
}
/* Gets a link to the block itself from within a repo, internal helper only */
a2_blk a2_getblk(a2_repo * r, a2var var){
    return r->cache[var];
}
/* Returns a pointer o the block given, internal helper only */
a2_blk * a2_getblkptr(a2_repo * r, a2var var){
    return &r->cache[var];
}
/* Checks whether the next block is free, internal helper only */
uint8_t has_next_block(a2_repo * r, a2var var){
    return (r->cache[r->cache[var].next].ptr) ? 1 : 0;
}
/* Calculates the difference between blocks, and hence free space (fragmentation gap), internal only */
uint32_t get_block_separation(a2_repo * r, a2var var){
    int difference = r->cache[r->cache[var].next].ptr - (r->cache[var].length + r->cache[var].ptr);
    return (difference < 0) ? 0 : (uint32_t)difference;
}

/* Sets the properties of a block in order to initialise it.  Internal helper only */
void setblk(a2_repo * r, a2var var, uint32_t p_length, void * p_ptr, a2var p_next, a2var p_prev){
    a2_blk *temp = a2_getblkptr(r, var);
    temp->length = p_length;
    temp->ptr = p_ptr;

    //pointing at front means unallocated.
    if(!p_next)
	temp->next = r->front;
    else
	temp->next = p_next;
    temp->prev = p_prev;
}



/* Initialises a repo
 *
 * p_mem_limit	      The size of memory to manage
 * p_base_ptr	      The memory area to manage
 * p_cache_limit      The number of items
 * p_cache	      TODO: I've forgotten!
 * p_options	      Some options, see a2.h for a set
 */ 
void a2_init(a2_repo *	repo, 
	    uint32_t	p_mem_limit, 
	    void *	p_base_ptr, 
	    uint32_t	p_cache_limit, 
	    a2_blk *	p_cache, 
	    int		p_options){

    int i;

    //initialise the repo
    repo->base_ptr		= p_base_ptr;
    repo->cache			= p_cache;
    repo->id_count		= 0;
    repo->cache_limit		= p_cache_limit;
    repo->cache_used		= 0;

    repo->mem_limit		= p_mem_limit;
    repo->mem_used		= 0;
    
    repo->options		= p_options;
    
    repo->front			= 0;

    for(i=0; i<repo->cache_limit; i++)
	repo->cache[i].ptr = 0;

    //populate 0 entry
    setblk(repo, repo->front, 0, repo->base_ptr, NULL, 0);
    repo->cache_used ++;

}

/* Prints the repo's layout in RAM in a human-readable fashion */
void a2_print_debug_repo_layout(a2_repo *r){
    int i;
    a2_blk * temp;

    printf("Repository @ %p\n", r);
    printf("--------------------------\n");
    printf("memory: %u / %u bytes.\n", r->mem_used, r->mem_limit);
    printf("cache:  %u / %u slots.\n", r->cache_used, r->cache_limit);
    
    for(i=0;i < r->cache_limit;i++){
	temp = a2_getblkptr(r, i);
	printf("%4u <  %u  > %2u \t ptr = %p, length = %i -> %x\n", temp->prev, i , temp->next, temp->ptr, temp->length, temp->ptr + temp->length);
    }
}

/* Prints the repo's layout in terms of slots in a human-readable fashion */
void a2_print_debug_memory_layout(a2_repo *r){
    int i = 0;
    a2var   currentv = r->front;
    a2_blk * current = a2_getblkptr(r, currentv);


    printf("Repository @ %p\n", r);
    printf("--------------------------\n");
    printf("memory: %u / %u bytes.\n", r->mem_used, r->mem_limit);
    printf("cache:  %u / %u slots.\n", r->cache_used, r->cache_limit);

    while(current->next != 0 && a2_getblkptr(r, current->next)->ptr){
	printf("%4i  |  %4i  ptr = %p, len = %i --> %p + %i = %p\n", i, currentv, current->ptr, current->length, (current->ptr + current->length), get_block_separation(r, currentv),   (current->ptr + current->length) + get_block_separation(r, currentv));
	
	i++;	
	currentv = current->next;
	current = a2_getblkptr(r, currentv);

    }
}


/* Allocates RAM inside the repo, returns -1 if it fails
 *
 * Depending on options set during initialisation this MAY DEFRAGMENT THE REPO, invalidating pointers
 *
 * returns -1 on error 
 */
a2var a2_malloc(a2_repo * r, uint32_t want){
    int id	    = 0;
    
    //populate pointers
    a2var currentv = r->front;
    a2_blk * new;
    a2_blk * current = a2_getblkptr(r, currentv);



    //some checks
    if(r->mem_used >= r->mem_limit)
	return (a2var)-1;

    if(r->cache_used >= r->cache_limit)
	return (a2var)-1;

    if((r->mem_limit - r->mem_used) < want)
	return (a2var)-1;


    id = get_next_id(r);
    printf("R: %i\n", id);

    while(current->next != 0 && a2_getblkptr(r, current->next)->ptr){
	//move around, update both
	currentv = current->next;
	current = a2_getblkptr(r, currentv);

	/*sleep(1);	*/
	/*printf("currentv: %i current: %p, len= %i, next= %i, sep= %i\n", currentv, current->ptr, current->length, current->next, get_block_separation(r, currentv));*/

	//try to insert into a gap if one exists
	if(get_block_separation(r, currentv) > want){
	    //yes! insert ourself onto 'next'
	    new = a2_getblkptr(r, id);
	    
	    setblk(r, id, want, current->ptr + current->length, current->next, currentv);	    
	    current->next	= id;

	    r->mem_used		+= new->length;
	    r->cache_used	++;

	    return id;
	}

    }


    //tack self onto end
    new = a2_getblkptr(r, id);
    setblk(r, id, want, current->ptr + current->length, NULL, currentv);	    
    current->next	= id;
    r->mem_used		+= new->length;
    r->cache_used	++;

    return id;


    return (a2var)-1;    
}


/* Frees a variable from the repo's allocation system.
 *
 * Depending on options set during initialisation this MAY DEFRAGMENT THE REPO
 */
int a2_free(a2_repo * r, a2var var){
    a2_blk * temp = a2_getblkptr(r, var);

    //make sure input is sane
    if(!var || var > r->cache_limit)
	return 0;

    //adjust running counters.
    r->mem_used -= temp->length;
    r->cache_used --;

    temp->length = 0;
    temp->ptr = 0;
    a2_getblkptr(r, temp->next)->prev = temp->prev;
    a2_getblkptr(r, temp->prev)->next = temp->next;

    temp->next = r->front;//get_next_id();
    temp->prev = r->front;//get_next_id();
    return 1;
}

/* Wipes the repo.  this is faster than incrementally wiping stuff.
 *
 * This will not defragment the repo, so may be considered safe-ish.
 */
void a2_free_all(a2_repo * r){
    int i;
    for(i=0; i<r->cache_limit; i++)
	if(i != r->front)
	    a2_free(r, i);

    //reset to point to self.
    a2_getblkptr(r,r->front)->next = r->front;
}


