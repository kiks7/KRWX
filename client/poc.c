#include "./lib/krwx.h"
#include <fcntl.h>

int main(){

  void* filp;
  void* chunk;
  int fd;

  filp = kmem_cache_get("filp"); // Get the kmem_cache* ptr to the filp cache
  printf("[*] filp: %p\n", filp);
  chunk = kmem_cache_alloc(filp, GFP_KERNEL); // Allocate a chunk in the filp cache
  printf("[*] chunk: %p\n", chunk);
  kmem_cache_free(filp, chunk); // Since now it's freed, the next allocation will take this chunk ! 
  kmem_cache_create_usercopy("testa", 16, 8, 0, 0, NULL);
  fd = open("/etc/passwd", O_RDWR);
  printf("[*] Now the chunk should contains the struct file of /etc/passwd\n");
  read_memory(chunk, 256);
}

/*

  [*] filp: 0xffff888003509500
  [*] chunk: 0xffff888003734400
  [*] Now the chunk should contains the struct file of /etc/passwd
  0xffff888003734400:     0x0000000000000000 0x0000000000000000
  0xffff888003734410:     0xffff8880036ca2a0 0xffff888004380000
  0xffff888003734420:     0xffff8880048a8590 0xffffffff82025e00
  0xffff888003734430:     0x0000000000000000 0x0000000000000001
  0xffff888003734440:     0x480f801f00008002 0x0000000000000000
  0xffff888003734450:     0x0000000000000000 0xffff888003734458
  0xffff888003734460:     0xffff888003734458 0x0000000000000000
  0xffff888003734470:     0x0000000000000000 0x0000000000000000
  0xffff888003734480:     0x0000000000000000 0x0000000000000000
  0xffff888003734490:     0xffff888003e69480 0x0000000000000000
  0xffff8880037344a0:     0x0000000000000000 0x0000000000000020
  0xffff8880037344b0:     0xffffffffffffffff 0x0000000000000000
  0xffff8880037344c0:     0xffff888003735020 0x0000000000000000
  0xffff8880037344d0:     0x0000000000000000 0xffff8880048a86f8
  0xffff8880037344e0:     0x0000000000000000 0x0000000000000000
  0xffff8880037344f0:     0x0000000000000000 0x0000000000000000

*/
