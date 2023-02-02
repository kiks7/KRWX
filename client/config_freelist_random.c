#include "./lib/krwx.h"
#define MAGIC_DEFAULT   0x4141414141414141
#define MAGIC_OVF       0x4545454545454545
#define MAGIC_LEAK      0x4747474747474747

#define true 1
#define false 0

void allocate(int chunk_size, int cache){
    int to_spray = 2000;

    void* ptr;
    void* prev = 0x0;
    unsigned long diff = 0;
    int count_consecutive = 0;
    int count_alternate = 0;

    printf("Spraying a little bit to fill partial slabs\n");
    for(int i = 0; i < 100; i++){ ptr = kmalloc( chunk_size, GFP_KERNEL); }
    printf("DO IT\n");

    void* allocated_chunks[to_spray];
    for(int i = 0; i < to_spray; i++){
        ptr = kmalloc( chunk_size, GFP_KERNEL);
        diff = (unsigned long) ptr - (unsigned long) prev;
        if(diff == cache){
            //printf("[%d] Consecutive chunks !!\n", i);
            count_consecutive++;
        }
        allocated_chunks[i] = ptr;
        for(int j = 0; j < i; j++){
            diff = (unsigned long) ptr - (unsigned long) allocated_chunks[j];
            if( diff == cache){
                printf("\t%d => %d = %ld\n", j, i, diff );
                printf("\tdiff index: %d\n", i - j );
                count_alternate++;
            }
        }

        prev = ptr;
    }

    printf("%d/%d allocations were consecutives\n", count_consecutive, to_spray);
    printf("%d chunks were found alternatevely consecutives\n", count_alternate);

}



void uaf(){
    // Demostrate that UAF is still possible also with CONFIG_FREELIST_RANDOM|HARDEN
    int chunk_size = 1024;
    void* prev;
    void* ptr;
    // Since it is the latest freed obj should not be necessary
    //printf("Spraying a little bit to fill partial slabs\n");
    //for(int i = 0; i < 2000; i++){ ptr = kmalloc( chunk_size, GFP_KERNEL); }
    prev = kmalloc(chunk_size, GFP_KERNEL);
    printf("%p\n", prev);
    kfree(prev);
    ptr = kmalloc(chunk_size, GFP_KERNEL);
    printf("%p\n", ptr);
    if(prev == ptr){ 
        printf("The object has been re-allocated at the same address\n");
    }

}

void consecutive_chunks(){
    int chunk_size = 1024;
    void* obj_v;
    void* obj_t;
    void* ptr;
    for(int i = 0; i < 2000; i++){ kmalloc( chunk_size, GFP_KERNEL); }
    

    int n_spray = 50;
    void* array_obj_t[n_spray];
    unsigned long diff;
    obj_t = kmalloc(chunk_size, GFP_KERNEL);
    for(int i = 0; i < n_spray; i++){
        ptr = kmalloc( chunk_size, GFP_KERNEL);
        array_obj_t[i] = ptr;
        diff = (unsigned long) ptr - (unsigned long) obj_t;
        if(diff == chunk_size)
            printf("diff : %ld\n", diff );
    }

    //printf("obj_t: %p\n", obj_t);

}


void overflow(){
    int chunk_size = 1024;
    void* obj_v = (void*) 0xdeadbeefdeadbeef;
    void* obj_t = (void*) 0xdeadbeefdeadbeef;
    void* ptr;
    printf("Spraying to fill partial heaps and more ..\n");
    for(int i = 0; i < 2000; i++){ kmalloc( chunk_size, GFP_KERNEL); }
 
    int n_spray = 50;
    void* array_obj_t[n_spray];
    unsigned long diff;
    obj_t = kmalloc(chunk_size, GFP_KERNEL);
    for(int i = 0; i < n_spray; i++){
        ptr = kmalloc( chunk_size, GFP_KERNEL);
        array_obj_t[i] = ptr;
        diff = (unsigned long) ptr - (unsigned long) obj_t;
        if(diff == chunk_size){
            printf("diff : %ld\n", diff );
            obj_v = ptr;
            break;
        }
    }

    kwrite64(obj_t, 0x4141414141414141);
    kwrite64(obj_v, 0x4242424242424242);
    printf("obj_t: %p\n", obj_t);
    printf("obj_v: %p\n", obj_v);
    read_memory(obj_t + 1024, 0x40);
    read_memory(obj_v, 0x40);

}

int find_consecutive_chunks_oob_write(int chunk_size, int cache_size){
    // obj_A = the one with OOB write
    // obj_b = the pre-victim object    

    int N_SPRAY = 20000 ;
    int ret = false;

    void* obj_A = (void*) 0xdeadbeefdeadbeef;
    void* obj_B = (void*) 0xdeadbeefdeadbeef;
    void* ptr;
    printf("[*] Spraying to fill partial heaps and more ..\n");
    for(int i = 0; i < 20000; i++){ kmalloc( chunk_size, GFP_KERNEL); }
 
    void* array_obj_B[N_SPRAY];
    unsigned long diff;
    int allocated_chunks = 0;
    printf("[*] Allocating obj_A (the target object)\n");
    obj_A = kmalloc(chunk_size, GFP_KERNEL);
    printf("[*] Spraying lot of pre-victim objects (obj_B)\n");
    for(int i = 0; i < N_SPRAY; i++){
        ptr = kmalloc( chunk_size, GFP_KERNEL);
        kwrite64(ptr, MAGIC_DEFAULT);
        array_obj_B[i] = ptr;
        allocated_chunks++;
    }

    printf("[*] Simulating an OOB write from obj_A\n");
    kwrite64(obj_A + cache_size, MAGIC_OVF);
    printf("[*] Checking which one of allocated obj_B have been overflown\n");

    
    for(int i = 0; i < N_SPRAY; i++){
        if(kread64(array_obj_B[i]) == MAGIC_OVF){
            printf("[+] Found the overflow obj_B at index %d\n", i);
            obj_B = array_obj_B[i];
            ret = true;
            break;
        }
    }

    if( obj_B == (void*) 0xdeadbeefdeadbeef){
        printf("[-] Failed to find a consecutive chunk\n");
        return false;
    }
    void* final_object_A;
    void* final_object_B;

    printf("[*] Freeing obj_A (%p) and obj_B (%p) and reallocating\n", obj_A, obj_B);
    
    kfree(obj_A); 
    kfree(obj_B);
    for(int i = 0; i < 3; i++){ ptr = kmalloc( chunk_size, GFP_KERNEL); }
    final_object_B = kmalloc(chunk_size, GFP_KERNEL);
    final_object_A = kmalloc(chunk_size, GFP_KERNEL);
    printf("[*] final obj_A: %p\n", final_object_A);
    printf("[*] final obj_B: %p\n", final_object_B);

    if(obj_A == final_object_A && obj_B == final_object_B){
        printf("[+] Succesfully crafted two adjacent chunks (A => B)\n");
        printf("[+] Now you can overflow from object A to object B =)\n");
    }
    return ret;

}

void find_consecutive_chunks_oob_read(){
    // obj_A = the one with OOB read
    // obj_B = a victim object to read

    int chunk_size = 1024;
    int N_SPRAY = 100 ;

    void* obj_A = (void*) 0xdeadbeefdeadbeef;
    void* obj_B = (void*) 0xdeadbeefdeadbeef;
    void* ptr;
    printf("[*] Spraying to fill partial heaps and more ..\n");
    for(int i = 0; i < 2000; i++){ kmalloc( chunk_size, GFP_KERNEL); }
 
    void* array_obj_B[N_SPRAY];
    unsigned long diff;
    int allocated_chunks = 0;
    printf("[*] Allocating obj_A (the target object)\n");
    obj_A = kmalloc(chunk_size, GFP_KERNEL);
    printf("[*] Spraying lot of victim objects (obj_B)\n");
    for(int i = 0; i < N_SPRAY; i++){
        ptr = kmalloc( chunk_size, GFP_KERNEL);
        kwrite64(ptr, MAGIC_LEAK);
        array_obj_B[i] = ptr;
        allocated_chunks++;
    }

    unsigned long oob_read = kread64(obj_A + chunk_size);
    printf("[*] leaked from OOB read: 0x%lx\n", oob_read);
    if( oob_read == MAGIC_LEAK){
        printf("[+] Succesfully leaked value from obj_B\n");
    }

}
void find_consecutive_chunks_oob_read_write(){
    // obj_A = the one with OOB read
    // obj_B = a victim object to read

    int chunk_size = 1024;
    int N_SPRAY = 100 ;

    void* obj_A = (void*) 0xdeadbeefdeadbeef;
    void* obj_B = (void*) 0xdeadbeefdeadbeef;
    void* ptr;
    printf("[*] Spraying to fill partial heaps and more ..\n");
    for(int i = 0; i < 2000; i++){ kmalloc( chunk_size, GFP_KERNEL); }
 
    void* array_obj_B[N_SPRAY];
    unsigned long diff;
    int allocated_chunks = 0;
    printf("[*] Allocating obj_A (the target object)\n");
    obj_A = kmalloc(chunk_size, GFP_KERNEL);
    printf("[*] Spraying lot of victim objects (obj_B)\n");
    for(int i = 0; i < N_SPRAY; i++){
        ptr = kmalloc( chunk_size, GFP_KERNEL);
        kwrite64(ptr, MAGIC_LEAK);
        array_obj_B[i] = ptr;
        allocated_chunks++;
    }

    unsigned long oob_read = kread64(obj_A + chunk_size);
    printf("[*] leaked from OOB read: 0x%lx\n", oob_read);
    if( oob_read == MAGIC_LEAK){
        printf("[+] Succesfully leaked value from obj_B\n");
    }
    printf("[*] Now we want to replace obj_A with another obj_C that will trigger an overflow against obj_B\n");
    void* obj_C;
    printf("[*] obj_A: %p\n", obj_A);
    printf("[*] Freeing obj_A\n");
    kfree(obj_A);
    printf("[*] Do some allocations to skip some pointers from the free list (3 allocations)\n");
    for(int i = 0; i < 3; i++){ kmalloc( chunk_size, GFP_KERNEL); }
    printf("[*] Allocating our target chunk\n");
    obj_C = kmalloc( chunk_size, GFP_KERNEL);
    printf("[*] obj_C: %p\n", obj_C);
    if(obj_A == obj_C){
        printf("[+] Successfully replaced the OOB_READ object with the OOB_WRITE one\n");
        printf("[+] Now you can overflow from obj_C => obj_B =)\n");
    }
    
}

int main(int argc, char **argv){

    //allocate(30, 32);
    //uaf();
    //consecutive_chunks()
    //overflow();
    //int count = 0; 
    //while(find_consecutive_chunks_oob_write( 4090, 4096 ) == false){
    //    count++;
    //}
    //printf("[+] Mission completed in %d attempts\n", count);
    //find_consecutive_chunks_oob_read();
    //find_consecutive_chunks_oob_read_write();

    
}
