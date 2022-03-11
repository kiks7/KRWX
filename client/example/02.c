#include "./lib/krwx.h"
#include <netinet/in.h>

int main(){
    init_krwx();
    void* chunk = kmalloc(150, _GFP_KERN);
    printf("Allocated chunk @%p\n", chunk);

    // Allocate target object
    struct iovec iov[10] = {0};
    char iov_buf[0x100];
    iov[0].iov_base = iov_buf;
    iov[0].iov_len = 0x1000;
    iov[1].iov_base = iov_buf;
    iov[1].iov_len = 0x1337;
    int pp[2];
    pipe(pp);
    if(!fork()){
        kfree(chunk);
        readv(pp[0], iov, 10);
        exit(0);
    }
    sleep(1);
    read_memory(chunk, 0x40);
}
