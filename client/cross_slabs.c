#include "./lib/krwx.h"

int exhaust_slabs(){
    int chunk_size = 16;
    int n_chunks = 7000000;

    kmem_cache_create_usercopy(0, "leet", chunk_size, 8, SLAB_ACCOUNT, 10, 20);
    for(int i=0; i < n_chunks; i++){
        kmem_cache_alloc(0, GFP_KERNEL_ACCOUNT);
    }
    
}

int cross_slabs(){
    int chunk_size = 256;
    int cache_size = 4096;
    void* ptr;

    printf("[*] Some spray for partial slabs ..");
    for(int i = 0; i < 100; i++){ kmalloc( chunk_size, GFP_KERNEL); }
    
    kmem_cache_create_usercopy(0, "leet", chunk_size, 8, SLAB_HWCACHE_ALIGN | SLAB_ACCOUNT, 0, 10);

    int n_spray = 8 * 80;
    void* array_obj_A[n_spray];
    void* array_obj_B[n_spray];
    unsigned long diff;

    printf("[*] Allocating GFP_KERNEL chunks \n");
    for(int i = 0; i < n_spray; i++){
        ptr = kmalloc( chunk_size, GFP_KERNEL_ACCOUNT);
        array_obj_A[i] = ptr;
    }

    printf("Freeing all GFP_KERNEL allocations\n");

    for(int i=0; i < n_spray; i++){
        kfree(array_obj_A[i]);
    }

    printf("Allocating same size special purpose cache with SLAB_ACCOUNT\n");
    for(int i = 0; i < n_spray; i++){
        ptr = kmem_cache_alloc(0, GFP_KERNEL);
        array_obj_B[i] = ptr;
        if(diff == chunk_size)
            printf("diff : %ld\n", diff );
    }

    printf("Analysing results ..\n");
    int starting_index = 0;
    int crossed_slabs = 0;
    for(int i=0; i < n_spray; i++){
        for(int j = 0; j < n_spray; j++){
            diff = (unsigned long) array_obj_B[i] - (unsigned long) array_obj_A[j];
            if(diff <= chunk_size){
                if(diff == 0 && starting_index == 0) starting_index = i;
                crossed_slabs++;
                //printf("%d => %d = %ld (%p => %p)\n", i, j, diff, array_obj_A[i], array_obj_B[j]);
            }
        }
    }

    printf("[+] After %d special allocations the freed GFP_KERNEL slabs have been used\n", starting_index);
    printf("[+] Number of crossed slabs: %d\n", crossed_slabs);
    printf("[+] Number of crossed slabs: %d/~%d\n", (crossed_slabs * chunk_size) / cache_size, (chunk_size * n_spray) / cache_size );
    printf("Done\n");
    return 0;
}



int main(){
    //init_krwx(); // Not necessary anymore, initialized directly from the .SO
    // YOUR CODE
    //exhaust_slabs();
    cross_slabs();
    /*
    kmem_cache_create_usercopy(0, "leet", 128, 8, SLAB_ACCOUNT, 10, 20);
    kmem_cache_create_usercopy(1, "leet1", 128, 8, SLAB_HWCACHE_ALIGN | SLAB_ACCOUNT, 0, 0);
    void* test = kmem_cache_alloc(0, GFP_KERNEL);
    printf("test: %p\n", test);
    kmem_cache_free(0, test);
    test = kmem_cache_alloc(1, GFP_KERNEL);
    printf("test: %p\n", test);
    */
}
