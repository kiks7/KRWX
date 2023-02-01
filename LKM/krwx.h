/*
 *
 * Written by Alessandro Groppo (@kiks)
 *
 */


#define DEV_NAME "/dev/krwx"
#define IOCTL_RW_READ   0xffd4
#define IOCTL_RW_WRITE  0xffd5
#define IOCTL_KMALLOC   0x34
#define IOCTL_KFREE             0x35
#define IOCTL_MEMK_CREATE   0xdd00
#define IOCTL_MEMK_ALLOC    0xdd01
#define IOCTL_MEMK_FREE     0xdd02
#define IOCTL_TEST_KMEM     0xaaff
#define IOCTL_MEMK_GET      0xbab3

#define MAX_KMEM    10
#define NAME_SZ     20


typedef unsigned int gfp_t;
#define _GFP_USER 0x100cc0
#define _GFP_KERN 0xcc0


// Since it's not possible to use kmem_cache*, here you go :/
// It doesn't contain everything, but just what we need
struct kmem_cache {
  char pad[0x60];
  const char* name;
  struct list_head list;
};

struct msg_read{
    void* kaddress;
    uint64_t content; // Init 0 from client-side
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
    void* result; // userland kmalloc return address
};


struct io_kmem_create {
    void* result;
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



int krwx_init(void);
void krwx_exit(void);

int krwx_open(struct inode *inode, struct file *file);
int krwx_release(struct inode *inode, struct file *file);
long int krwx_ioctl(struct file *fp, unsigned int cmd, unsigned long arg);
ssize_t krwx_write(struct file *, const char *, size_t, loff_t *);
ssize_t krwx_read(struct file *, char *, size_t, loff_t *);



