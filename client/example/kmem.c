#include "./lib/krwx.h"

int main(){
  kmem_cache_create_usercopy(0, "leet", 128, 8, SLAB_ACCOUNT, 10, 20);
  kmem_cache_create_usercopy(1, "leet1", 128, 8, SLAB_HWCACHE_ALIGN | SLAB_ACCOUNT, 0, 0);
  printf("test: %p\n", kmem_cache_alloc(0, GFP_KERNEL));
  printf("test: %p\n", kmem_cache_alloc(1, GFP_KERNEL));
}
