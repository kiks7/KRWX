#include "./lib/krwx.h"

void double_free(){
    void* obj_A;
    void* obj_B;
    obj_A = kmalloc(8, GFP_KERNEL);
    obj_B = kmalloc(8, GFP_KERNEL);
    kfree(obj_A);
    kfree(obj_A);
    
    printf("%p\n", kmalloc(1024, GFP_KERNEL));
    printf("%p\n", kmalloc(1024, GFP_KERNEL));
    printf("%p\n", kmalloc(1024, GFP_KERNEL));
}

void double_free_bypass(){
    void* obj_A;
    void* obj_B;
    obj_A = kmalloc(8, GFP_KERNEL);
    obj_B = kmalloc(8, GFP_KERNEL);
    kfree(obj_A);
    kfree(obj_B);
    kfree(obj_A);
    
    void* obj_first;
    void* obj_second;
    void* obj_third;
    obj_first = kmalloc(8, GFP_KERNEL);
    obj_second = kmalloc(8, GFP_KERNEL);
    obj_third = kmalloc(8, GFP_KERNEL);
    printf("obj_first: %p\n", obj_first);
    printf("obj_second: %p\n", obj_second);
    printf("obj_third: %p\n", obj_third);
    if(obj_first == obj_third){
        printf("[+] Double free successfull =)\n");
    }
    
}



int main(){
    //init_krwx(); // Not necessary anymore, initialized directly from the .SO
    // YOUR CODE
    //double_free();
    double_free_bypass();
}
