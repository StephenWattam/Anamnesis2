#include <stdio.h>
#include <stdlib.h>
#include "a2.h"

/* These macros are handy for avoiding pointer usage.  They ensure that no unsafe A2 variables are pointed at during over operations that may re-order RAM. */
#define A_G(var) a2_get(var)
#define A(var, type) *((type*)(a2_get(var)->ptr))


/* Simple test program to illustrate the use of anamnesis. 
 *
 * This code allocates 2k of RAM and then assigns some variables to it.  Whilst the code manipulates variables anamnesis maintains a track of memory usage, displaying it using the debug functions.
 *
 */
int main(int argc, char ** argv){
    a2_repo repository; //new repo
    a2var one, two, three, four, five, six; //6 variables

    //Initialise the repo
    a2_init(&repository, 2000, malloc(2000), 10, (a2_blk *)malloc(10 * sizeof(a2_blk)), NULL);


    /* Look, a brand new repo! */
    a2_print_debug_memory_layout(&repository);

    /* Allocate the variables in the anamnesis repo */
    one	    = a2_malloc(&repository, 100);
    a2_print_debug_memory_layout(&repository);
    
    two	    = a2_malloc(&repository, 200);
    a2_print_debug_memory_layout(&repository);
    
    three   = a2_malloc(&repository, 300);
    a2_print_debug_memory_layout(&repository);
    
    four	    = a2_malloc(&repository, 100);
    a2_print_debug_memory_layout(&repository);
    
    five	    = a2_malloc(&repository, 200);
    a2_print_debug_memory_layout(&repository);
    
    six   = a2_malloc(&repository, 300);
    a2_print_debug_memory_layout(&repository);

    



    /* Free up some variables to test defragmentation */
    a2_free(&repository, two);
    a2_print_debug_memory_layout(&repository);
    a2_free(&repository, three);
    a2_print_debug_memory_layout(&repository);

    
    
    two	    = a2_malloc(&repository, 200);
    a2_print_debug_memory_layout(&repository);
    three   = a2_malloc(&repository, 300);
    a2_print_debug_memory_layout(&repository);
    
    one	    = a2_malloc(&repository, 100);
    a2_print_debug_memory_layout(&repository);
    two	    = a2_malloc(&repository, 200);
    a2_print_debug_memory_layout(&repository);
    three   = a2_malloc(&repository, 300);
    a2_print_debug_memory_layout(&repository);
    four    = a2_malloc(&repository, 100);
    a2_print_debug_memory_layout(&repository);
    five    = a2_malloc(&repository, 200);
    a2_print_debug_memory_layout(&repository);
    six   = a2_malloc(&repository, 300);
    a2_print_debug_memory_layout(&repository);
    one	    = a2_malloc(&repository, 100);
    a2_print_debug_memory_layout(&repository);
    two	    = a2_malloc(&repository, 200);
    a2_print_debug_memory_layout(&repository);
    three   = a2_malloc(&repository, 300);
    a2_print_debug_memory_layout(&repository);
    four    = a2_malloc(&repository, 100);
    a2_print_debug_memory_layout(&repository);
    five    = a2_malloc(&repository, 200);
    a2_print_debug_memory_layout(&repository);
    six   = a2_malloc(&repository, 300);


    //Free RAM, make sure the code doesn't leak
    a2_free_all(&repository);

    a2_print_debug_memory_layout(&repository);
    four    = a2_malloc(&repository, 100);
    a2_print_debug_memory_layout(&repository);
    five    = a2_malloc(&repository, 200);
    a2_print_debug_memory_layout(&repository);
    six   = a2_malloc(&repository, 300);
    
    return 0;
}
