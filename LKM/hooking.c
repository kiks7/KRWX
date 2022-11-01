/*
 *
 * Written by Alessandro Groppo (@kiks)
 *
 */

#define _GFP_USER 0x100cc0
#define _GFP_KERN 0xcc0
#define _HOOKED_FUNC_H

unsigned long cc_copy_from_user(void* to, const void* __user *from, unsigned long n){ 
    unsigned long res = copy_from_user(to, from, n); 
    //printk("copy_from_user(to: 0x%px, from: 0x%px, n: %ld) --> 0x%lx\n", to, from, n, res);
    return res;
}

unsigned long cc_copy_to_user(void* __user to, const void* from, unsigned long n){ 
    unsigned long res = copy_to_user(to, from, n); 
    //printk("copy_to_user(to: 0x%px, from: 0x%px, n: %ld) --> 0x%lx\n", to, from, n, res);
    return res;
}

void * cc_kmalloc(size_t size, gfp_t flags){
    void * res = kmalloc(size, flags);
    //if( (flags & _GFP_USER) == _GFP_USER)
    //    printk("kmalloc(%zd, GFP_USER) --> 0x%px\n", size, res);
    //else if( (flags & _GFP_KERN) == _GFP_KERN)
    //    printk("kmalloc(%zd, GFP_KERN) --> 0x%px\n", size, res);
    //else
    //    printk("kmalloc(%zd, %x) --> 0x%px\n", size, flags, res);
    
    return res;
}

void cc_kfree(const void* objp){
    //printk("kfree(%px)\n", objp);
    kfree(objp); 
}
