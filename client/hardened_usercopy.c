#include "./lib/krwx.h"


int usercopy(){
    void* ptr;
    unsigned long res;
    ptr = kmalloc(16, GFP_KERNEL);
    
    kwrite64(ptr , 0x4343434343434343);

    read_memory(ptr + 16, 32);
    unsigned long* test_r = kread(ptr + 16, 32);
    read_userland_memory(test_r, 32);
    
    return 1;
}

int main(){
    //init_krwx(); // Not necessary anymore, initialized directly from the .SO
    // YOUR CODE
    usercopy();
    
}
