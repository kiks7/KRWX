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
#include <string.h>

#define Yellow(string) "\e[0;33m" string "\x1b[0m"
#define BYellow(string) "\e[1;33m" string "\x1b[0m"
#define BCyan(string) "\e[1;36m" string "\x1b[0m"
#define Cyan(string) "\e[0;36m" string "\x1b[0m"

#define DEV_NAME "/dev/krwx"
#define IOCTL_RW_READ   0xffd4
#define IOCTL_RW_WRITE  0xffd5
#define IOCTL_KMALLOC   0x34
#define IOCTL_KFREE     0x35
#define IOCTL_MEMK_CREATE   0xdd00
#define IOCTL_MEMK_ALLOC    0xdd01
#define IOCTL_MEMK_FREE     0xdd02
#define IOCTL_MEMK_GET      0xbab3
#define IOCTL_SLAB_PTR      0xccdf

typedef unsigned int gfp_t;
#define GFP_USER 0x100cc0
#define GFP_KERNEL 0xcc0
#define GFP_KERNEL_ACCOUNT 0x400cc0

#define SLAB_ACCOUNT 0x4000000
#define SLAB_HWCACHE_ALIGN 0x2000

#define NAME_SZ   20

struct msg_read{
  void* kaddress;
  uint64_t content;
  uint64_t size;
};

struct msg_write{
  void* kaddress;
  uint64_t value;
  uint64_t size;
};

struct io_kmalloc {
  size_t size;
  gfp_t flags;
  void* result;
};

struct io_kmem_create {
  void* result; // EMPTY, the kernel will put the kmem address here
  size_t obj_size;
  size_t align;
  unsigned long flags;
  size_t useroffset;
  size_t usersize;
  char name[NAME_SZ];
};

struct io_kmem_alloc{
  void* kmem_addr;
  gfp_t flags;
  void* result;
};

struct io_kmem_free{
  void* kmem_addr;
  void* pointer;
};

struct io_kmem_get{
  void* result;
  char name[NAME_SZ];
};

struct io_slab_ptr{
  void* ptr;
  char* name;
};

#define MAX_ALLOCATIONS 1000
unsigned int io_slab_ptr_allocs_idx;
char* io_slab_ptr_allocs[MAX_ALLOCATIONS];

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
  msg.kaddress = address;
  msg.content = 0x0;
  msg.size = sizeof(uint64_t);
  if( ioctl(fd_dev, IOCTL_RW_READ, &msg )){
    perror("[-] IOCTL_RW_READ Failed\n");
    return -1;
  }
  return msg.content;
}

uint64_t* kread(void* address, uint64_t size){
  // TODO: To change, not working anymore this function
  struct msg_read msg;
  uint64_t* ptr_content = malloc(size);
  memset(ptr_content, 0x0, size * sizeof(uint64_t));
  msg.kaddress = address;
  msg.content = (uint64_t) ptr_content;
  msg.size = size;
  if( ioctl(fd_dev, IOCTL_RW_READ, &msg) ){
    perror("[-] IOCTL_RW_READ Failed\n");
    return NULL;
  }
  return ptr_content;
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
  mw.value = value;
  mw.size = sizeof(uint64_t);
  if( ioctl(fd_dev, IOCTL_RW_WRITE, &mw) ){
    perror("[-] IOCTL_RW_WRITE Failed\n");
    return -1;
  }
  return 0;
}

void multiple_kmalloc(void** array, uint32_t n_objs, uint32_t size){
  gfp_t _flags = GFP_KERNEL;
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

void l_print_qword(void* address){
  unsigned long first;
  unsigned long second;
  first = *(int64_t *) address;
  second = *(int64_t *) (address + 0x8);
  printf(BCyan("%p:\t"), address);
  printf(Cyan("0x%016lx 0x%016lx\n"), first, second);

}

void read_userland_memory(void* start_address, size_t size){
  void* end_address = start_address + size; // also if not padded to 8 it will be fine in the loop condition
                                            //printf("[D] [read_memory] start_addres: %p\n[D][read_memory] end_address: %p\n", start_address, end_address);
  printf("\n");
  while(start_address < end_address){
    l_print_qword(start_address);
    start_address = start_address + (8 * 2);
  }
}

void* kmem_cache_create_usercopy(char* name, unsigned int obj_size, unsigned long align, unsigned long flags, size_t useroffset, size_t usersize){
  struct io_kmem_create km;
  km.result = NULL;
  km.obj_size = obj_size;
  km.align = align;
  km.flags = flags;
  km.useroffset = useroffset;
  km.usersize = usersize;
  strncpy(km.name, name, NAME_SZ);

  if( ioctl(fd_dev, IOCTL_MEMK_CREATE, &km) ){
    perror("[-] IOCTL_MEMK_CREATE failed\n");
    return NULL;
  }

  return km.result;
}

void*kmem_cache_alloc(void* addr, unsigned long flags){
  /* The flags are only relevant if the cache has no available objects. */
  struct io_kmem_alloc km;
  km.kmem_addr = addr;
  km.flags = flags;
  km.result = NULL;

  if( ioctl(fd_dev, IOCTL_MEMK_ALLOC, &km) ){
    perror("[-] IOCTL_MEMK_ALLOC failed\n");
    return NULL;
  }

  return km.result;
}

int kmem_cache_free(void* addr, void* pointer){
  struct io_kmem_free km;
  km.kmem_addr = addr;
  km.pointer = pointer;

  if( ioctl(fd_dev, IOCTL_MEMK_FREE, &km) ){
    perror("[-] IOCTL_MEMK_FREE failed\n");
    return -1;
  }

  return 0;
}

void* kmem_cache_get(char* name){
  struct io_kmem_get km;
  strncpy(km.name, name, NAME_SZ);
  km.result = NULL;

  if( ioctl(fd_dev, IOCTL_MEMK_GET, &km) ){
    perror("[-] IOCTL_MEMK_GET failed\n");
    return NULL;
  }

  return km.result;
}

char* slab_ptr(void* ptr){
  struct io_slab_ptr km;
  char* out_string;
  // NAME_SZ is the max cache size
  if(io_slab_ptr_allocs_idx > MAX_ALLOCATIONS){
    printf("Too many slab_ptr allocations");
    return (char*) -1;
  }
  out_string = malloc(NAME_SZ);
  io_slab_ptr_allocs_idx++;
  memset(out_string, 0x0, NAME_SZ);
  km.ptr = ptr;
  km.name = out_string;

  if( ioctl(fd_dev, IOCTL_SLAB_PTR, &km) ){
    perror("[-] IOCTL_MEMK_GET failed\n");
  }
  return km.name;
}

void __attribute__ ((constructor)) setup(void) { init_krwx(); }
void __attribute__((destructor)) destruct();

void destruct(){
  // Cleanup
  // Cleaneup allocation made from slab_ptr
  for(int i=0; i < io_slab_ptr_allocs_idx; i++){
    free(io_slab_ptr_allocs[i]);
  }
}
