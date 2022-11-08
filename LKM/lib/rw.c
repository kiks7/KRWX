/*
 *
 * Written by Alessandro Groppo (@kiks)
 *
 */


#ifndef _HOOKED_FUNCS_H
#include <linux/slab.h>
#include <asm/uaccess.h>
#include "../hooking.c"
#endif

extern struct kmem_cache* global_kmem[];

int ioctl_rw_read(struct msg_read* user_read_msg){
    //pr_info("ioctl_rw::ioctl_rw_read\n"); 
    //if( cc_copy_to_user(read_msg->content, (uint64_t *) read_msg->kaddress, read_msg->size) ){
    //    pr_info("EFAULT GENERATED\n"); 
    //   return -EFAULT;
    //}

    struct msg_read read_msg;
    unsigned long result;
    if( cc_copy_from_user(&read_msg, (void*) user_read_msg, sizeof(struct msg_read)) )
        return -EFAULT;
    memcpy(&result, read_msg.kaddress, sizeof(unsigned long));
    if( put_user(result, &user_read_msg->content) )
      return -EFAULT;
     
    return 0;
}

unsigned long ioctl_rw_write(struct msg_write* user_write_msg){
    //pr_info("ioctl_rw::ioctl_rw_write\n");
    //if( cc_copy_from_user(write_msg->kaddress, (void*) write_msg->value, write_msg->size) )
    //    return -EFAULT;

    struct msg_write write_msg;
    if( cc_copy_from_user(&write_msg, (void*) user_write_msg, sizeof(struct msg_write)) )
        return -EFAULT;
    memcpy(write_msg.kaddress, &write_msg.value, write_msg.size);
    return 0;
}

int ioctl_kmalloc(struct io_kmalloc* __user arg){
    struct io_kmalloc sk;
    void* k_res;
    if( cc_copy_from_user(&sk, (void*) arg, sizeof(struct io_kmalloc)) )
        return -EFAULT;
    k_res = cc_kmalloc(sk.size, sk.flags);
    if( cc_copy_to_user(sk.result, &k_res, sizeof(void*)) ) 
        return -EFAULT;
    return 0;
}

int ioctl_kfree(void* arg){
    cc_kfree(arg);
    return 0;
}

#ifdef CONFIG_HARDENED_USERCOPY
int ioctl_kmem_create_usercopy(struct io_kmem_create* __user user_kmem){
    struct io_kmem_create kmem;
    if( cc_copy_from_user(&kmem, (void*) user_kmem, sizeof(struct io_kmem_create)) )
        return -EFAULT;
    if(kmem.index >= MAX_KMEM){
        pr_info("Index too large\n");
        return -EFAULT;
    }
    global_kmem[kmem.index] = kmem_cache_create_usercopy(kmem.name, kmem.obj_size, kmem.align, kmem.flags, kmem.useroffset, kmem.usersize, NULL);
    if(!global_kmem[kmem.index])
        return -EFAULT;

    return 0;
}
#else
int ioctl_kmem_create_usercopy(struct io_kmem_create* __user user_kmem){
    pr_info("ioctl_kmem_create_usercopy not supported\n");
    return -EINVAL;
}

#endif

int ioctl_kmem_alloc(struct io_kmem_alloc* __user user_kmem){
    void* obj;
    struct io_kmem_alloc kmem;
    if( cc_copy_from_user(&kmem, (void*) user_kmem, sizeof(struct io_kmem_alloc)) )
        return -EFAULT;

    if(kmem.index >= MAX_KMEM){
        pr_info("Index too large\n");
        return -EFAULT;
    }

    obj = kmem_cache_alloc(global_kmem[kmem.index], kmem.flags);
    if( put_user(obj, &user_kmem->result) )
        return -EFAULT;
    return 0;
}

int ioctl_kmem_free(struct io_kmem_free* __user user_kmem){
    struct io_kmem_free kmem;
    if( cc_copy_from_user(&kmem, (void*) user_kmem, sizeof(struct io_kmem_free)) )
        return -EFAULT;
    if(kmem.index >= MAX_KMEM){
        pr_info("Index too large\n");
        return -EFAULT;
    }
    kmem_cache_free(global_kmem[kmem.index], kmem.pointer);
    return 0;
}



#ifdef CONFIG_HARDENED_USERCOPY
int ioctl_kmem_test(unsigned long arg){
    const char* name = "leet";
    void* obj;
    switch(arg){
        case 1:
            global_kmem[0] = kmem_cache_create_usercopy(name, 128, 8, SLAB_ACCOUNT, 0, 0, NULL);
            obj = kmem_cache_alloc(global_kmem[0], GFP_KERNEL);
            pr_info("obj: 0x%lx\n", (unsigned long)obj);
            kmem_cache_free(global_kmem[0], obj);
            pr_info("freed");
            break;
        case 2:
            obj = kmem_cache_alloc(global_kmem[0], GFP_KERNEL);
            pr_info("[2] obj: 0x%lx\n", (unsigned long)obj);
            break;
        default:
            break;
    }
    return 0;

}
#else
int ioctl_kmem_test(unsigned long arg){
    pr_info("ioctl_kmem_test not supported\n");
    return EINVAL;
}
#endif
