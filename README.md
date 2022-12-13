# KRWX: Kernel Read Write Execute
## What
KRWX offers the possibility to call a set of memory-related kernel APIs from user-land through a device driver (`/dev/krwx`) and read/write arbitary memory addresses (without interfering with SMAP/PAN).
It can be used for educational purposes (in order to study some kernel internals like the memory managament) and to assist the exploitation phase (I developed [this entire exploit](https://1day.dev/notes/Linux-Kernel-n-day-exploit-development) without touching gdb a single time).

It's architecture independent and works with all linux kernels (tested on >=4.9) and devices (Android as well).

## How
### Kernel
If you are interested in the kernel implemenation, look at the `LKM/` folder.
Its implementation is based on IOCTL commands (`LKM/krwx.c`) that wrap the incoming requests with their original function (implemented in `LKM/lib/rw.c`) and return the result back to user-land.

### User-land
In order to simplify the user-land communication, a library in `client/lib/krwx.h` has been developed with wrappers that do the ioctl operations behind the scenes. If the library is included from C, the device character is automatically opened (`/dev/krwx`) and it exposes custom functions and kernel API wrappers.

#### API Wrappers
These wrappers are implemented in the same way as the kernel calls them. So, you can find necessary arguments by using their man page. Actually, the following APIs have been implemented:
- `kmalloc`
- `kfree`
- `kmem_cache_create_usercopy`
- `kmem_cache_alloc`
- `kmem_cache_free`

#### Custom Functions
Other than classic wrappers, the library offers more functions that can be useful while exploiting or understanding a specific subject:
- `kread64(void* address)`/`kwrite64(void* address, uint64_t value)` in order to read and write 8 bytes of kernel memory.
- `multiple_kmalloc(void** array, uint32_t n_objs, uint32_t size)` and `multiple_kfree(void** array, uint64_t to_free[], uint64_t to_free_size)` respectevly used to allocate and free multiple chunks using `kmalloc`/`kfree`.
- ` read_memory(void* start_address, size_t size)` read `size` bytes of memory starting from `start_address`.
- `read_userland_memory(void* start_address, size_t size)` reads userland memory.
- `kmem_cache_get(char* name)` retrieve the address of the specified `kmem_cache`. It can be used to allocate/free objects in the requested cache using `kmem_cache_alloc` and `kmem_cache_free` (see `client/example/kmem_cache_get.c`).

## Setup
To compile the module change the `K` variable in the `Makefile` with your compiled kernel root directory and compile with `make`, then `insmod`.

## Learning by example
The best way to understand this project, is by looking at some hands-on examples. Personally I used it to understand Linux memory management, basic vulnerabilities and mitigations/hardening features (that's why I started its development but later I found it useful also while exploiting).

### Skeleton
Just include the C library:
```C
#include "./lib/krwx.h"

int main(){
    // YOUR CODE
}
```
### Allocate, free and read arbitrary chunks
You can find the full source code in `example/01.c`. Here will follows some snippets and a little walkthrough.

10 chunks with size 256 are allocated using `multiple_kmalloc`, and the memory of the 7th allocation is read using `read_memory` after writing `0x4141414141414141` at its first bytes:

```C
void* chunks[10];
multiple_kmalloc(&chunks, 10, 256);
kwrite64(chunks[7], 0x4141414141414141);
read_memory(chunks[7], 0x10);
```

The indexes 3, 4 and 7 of the `chunks` array are freed using `multiple_kfree`:

```C
uint64_t to_free[] = {3, 4, 7};
multiple_kfree(&chunks, &to_free, ( sizeof(to_free) / sizeof(uint64_t) ) );
```

Once they are freed, new chunks with the same size are allocated and initialized with `0x4343434343434343`, and the memory of the 7h freed chunk is displayed using `read_memory` again:

```C
kwrite64(kmalloc(256, GFP_KERNEL), 0x4343434343434343);
kwrite64(kmalloc(256, GFP_KERNEL), 0x4343434343434343);
kwrite64(kmalloc(256, GFP_KERNEL), 0x4343434343434343);
kwrite64(kmalloc(256, GFP_KERNEL), 0x4343434343434343);
kwrite64(kmalloc(256, GFP_KERNEL), 0x4343434343434343);
read_memory(chunks[7], 0x10);
```

And the result is:

```bash
[*] Allocating 10 chunks with size 256
[*] Allocated @0xffffffc00503b900
[*] Allocated @0xffffffc00503b600
[*] Allocated @0xffffffc00503b100
[*] Allocated @0xffffffc00503bc00
[*] Allocated @0xffffffc00503b400
[*] Allocated @0xffffffc00503b000
[*] Allocated @0xffffffc00503b500
[*] Allocated @0xffffffc00503b800
[*] Allocated @0xffffffc00503ba00
[*] Allocated @0xffffffc00503bd00
0xffffffc00503b800:     0x4141414141414141 0xffffffc0001a8928
[*] Freeing @0xffffffc00503bc00
[*] Freeing @0xffffffc00503b400
[*] Freeing @0xffffffc00503b800
0xffffffc00503b800:     0x4343434343434343 0xffffffc0001a8928
```

With few lines of code has been demostrated how our 7th chunk has been replaced with a new one after it has been freed (the `read_memory` targeted the `chunks[7]`). As simple as it is, it has been written for demonstration purposes.

### Use-After-Free
To simulate a UAF scenario it’s simple as few lines of code :

```C
void* chunk = kmalloc(<SIZE>, <FLAGS>);
kfree(chunk);
// Allocate your target chunk
// Simulate UAF using k[write|read]64()
```

For example, if we want to simulate an attack scenario where we want to replace our vulnerable freed chunk with a target object (for example an `iovec`struct) we can allocate a chunk with `kmalloc` and later `kfree` it just before allocating the target structure:

```C
// Allocate the vulnerable object
void* chunk = kmalloc(150, GFP_KERNEL);
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
	kfree(chunk); // Freeing the chunk just before allocating the iovec
	readv(pp[0], iov, 10); // allocate iovec and blocks (keeping the object in the kernel) 
	exit(0);
}
sleep(1); // Give time to the child process
read_memory(chunk, 0x40);
```

Then, with `read_memory` we can show the block of memory in our interest and as you can see from the following output, our arbitrary allocated/freed object has been replaced with the target object:

```C
Allocated chunk @0xffffffc0052c5a00
0xffffffc0052c5a00:     0x0000007fd311ff58 0x0000000000001000
0xffffffc0052c5a10:     0x0000007fd311ff58 0x0000000000001337
0xffffffc0052c5a20:     0x0000000000000000 0x0000000000000000
0xffffffc0052c5a30:     0x0000000000000000 0x0000000000000000
```

Instead of just print the content, you can simulate a UAF read/write using `k[read|write]` and play with it.

The full code of this example can be found in `client/example/02.c`


