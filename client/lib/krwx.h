/*
 *
 * Written by Alessandro Groppo (@kiks)
 *
 */

#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define Yellow(string) "\e[0;33m" string "\x1b[0m"
#define BYellow(string) "\e[1;33m" string "\x1b[0m"

#define DEV_NAME "/dev/krwx"
#define IOCTL_RW_READ   0xffd4
#define IOCTL_RW_WRITE  0xffd5
#define IOCTL_KMALLOC   0x34
#define IOCTL_KFREE     0x35

typedef unsigned int gfp_t;
#define _GFP_USER 0x100cc0
#define _GFP_KERN 0xcc0

struct msg_read{
    void* kaddress;
    uint64_t* content;
};

struct msg_write{
    void* kaddress;
    uint64_t* value;
};

struct io_kmalloc {
    size_t size;
    gfp_t flags;
    void* result;
};

int fd_dev;

void* kmalloc(size_t arg_size, gfp_t flags){
    struct io_kmalloc km;
    void* result = 0x0;
    km.size = arg_size;
    km.flags = flags;
    km.result = &result;
    if ( ioctl(fd_dev, IOCTL_KMALLOC, &km)){
        perror("[-] IOCTL_KMALLOC failed.\n");
        return 0;
    }

    return result;
}

int kfree(void* address){
    if ( ioctl(fd_dev, IOCTL_KFREE, address ) ){
        perror("[-] IOCTL_KFREE failed.\n");
        return -1;
    }
    return 0;

}

unsigned long int kread64(void* address){
    struct msg_read msg;
    uint64_t content;
    content = 0x0;
    msg.kaddress = address;
    msg.content = &content;
    
    if( ioctl(fd_dev, IOCTL_RW_READ, &msg) ){
        perror("[-] IOCTL_RW_READ Failed\n");
        return -1;
    }
    return content;
}

void print_qword(void* address){
    unsigned long first = 0x0;
    unsigned long second = 0x1337;
    first = kread64(address);
    second = kread64(address + 8);
    printf(BYellow("%p:\t"), address);
    printf(Yellow("0x%016lx 0x%016lx\n"), first, second);

}

void read_memory(void* start_address, size_t size){
    // Read from kernel memory and print better, it will be padded to 8 ofc
    void* end_address = start_address + size; // also if not padded to 8 it will be fine in the loop condition
    //printf("[D] [read_memory] start_addres: %p\n[D][read_memory] end_address: %p\n", start_address, end_address);
    while(start_address < end_address){
        print_qword(start_address);
        start_address = start_address + (8 * 2);
    }
}

int kwrite64(void* address, uint64_t value){
    struct msg_write mw;
    mw.kaddress = address;
    mw.value = &value;
    if( ioctl(fd_dev, IOCTL_RW_WRITE, &mw) ){
        perror("[-] IOCTL_RW_WRITE Failed\n");
        return -1;
    }
    return 0;
}

void multiple_kmalloc(void** array, uint32_t n_objs, uint32_t size){
    gfp_t _flags = _GFP_KERN;
    printf("[*] Allocating %d chunks with size %d\n", n_objs, size);
    for( int i = 0; i < n_objs; i = i + 1) {
        array[i] = kmalloc(size, _flags);
        printf("[*] Allocated @0x%lx\n", (unsigned long) array[i]);
    } 
}

void multiple_kfree(void** array, uint64_t to_free[], uint64_t to_free_size){
    for( int i = 0; i < to_free_size ; i = i + 1){
        printf("[*] Freeing @0x%lx\n", (unsigned long) array[to_free[i]]);
        kfree(array[to_free[i]]);
    }
}


int init_krwx(){
    fd_dev = open(DEV_NAME, O_RDWR);
    if (fd_dev == -1){
        printf("[-] Error opening handle to %s\n", DEV_NAME);
        exit(-1);
    }
    return 0;
}

void __attribute__ ((constructor)) setup(void) {
    init_krwx();
}
