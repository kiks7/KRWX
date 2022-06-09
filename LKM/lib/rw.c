/*
 *
 * Written by Alessandro Groppo (@kiks)
 *
 */


#ifndef _HOOKED_FUNCS_H
#include "../hooking.c"
#endif


int ioctl_rw_read(struct msg_read* read_msg){
    //pr_info("ioctl_rw::ioctl_rw_read\n"); 
    /* NOT TESTED BELOW (coding blindly) */
    if( cc_copy_to_user(read_msg->content, (uint64_t *) read_msg->kaddress, sizeof(uint64_t)) )
       return -EFAULT;

    pr_info("msg_read->kaddress --> %p\n", read_msg->kaddress);
     
    return 0;
}

unsigned long ioctl_rw_write(struct msg_write* write_msg){
    //pr_info("ioctl_rw::ioctl_rw_write\n");
    //struct msg_write write_msg;
    if( cc_copy_from_user(write_msg->kaddress, (void*) write_msg->value, sizeof(uint64_t)) )
        return -EFAULT;

    return 0;
}

int ioctl_kmalloc(struct io_kmalloc* __user arg){
    /* NOT TESTED */
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
