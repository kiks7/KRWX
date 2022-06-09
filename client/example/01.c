/*
 *
 * Written by Alessandro Groppo (@kiks)
 *
 */

#include "./lib/krwx.h"

int main(){
    //init_krwx();
    void* chunks[10];
    multiple_kmalloc(&chunks, 10, 256);
    kwrite64(chunks[7], 0x4141414141414141);
    read_memory(chunks[7], 0x10);
    uint64_t to_free[] = {3, 4, 7};
    multiple_kfree(&chunks, &to_free, ( sizeof(to_free) / sizeof(uint64_t) ) );
    kwrite64(kmalloc(256, _GFP_KERN), 0x4343434343434343);
    kwrite64(kmalloc(256, _GFP_KERN), 0x4343434343434343);
    kwrite64(kmalloc(256, _GFP_KERN), 0x4343434343434343);
    kwrite64(kmalloc(256, _GFP_KERN), 0x4343434343434343);
    kwrite64(kmalloc(256, _GFP_KERN), 0x4343434343434343);
    read_memory(chunks[7], 0x10);
}
