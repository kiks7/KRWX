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

typedef unsigned int gfp_t;
#define _GFP_USER 0x100cc0
#define _GFP_KERN 0xcc0


struct msg_read{
    void* kaddress;
    uint64_t* content; // Init 0 from client-side
};

struct msg_write{
    void* kaddress;
    uint64_t* value;
};

struct io_kmalloc {
    size_t size;
    gfp_t flags;
    void* result; // userland kmalloc return address
};


int krwx_init(void);
void krwx_exit(void);

int krwx_open(struct inode *inode, struct file *file);
int krwx_release(struct inode *inode, struct file *file);
long int krwx_ioctl(struct file *fp, unsigned int cmd, unsigned long arg);
ssize_t krwx_write(struct file *, const char *, size_t, loff_t *);
ssize_t krwx_read(struct file *, char *, size_t, loff_t *);

