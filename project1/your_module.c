#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/list.h>

struct birthday{
    int day;
    int month;
    int year;
    struct list_head list;
};

static struct list_head mydata;

void add_birthday(int i, int year, int month, int day){
    struct birthday *node = (struct birthday*) kmalloc(sizeof(struct birthday), GFP_KERNEL);
    if (node == NULL) return;

    node->day = day;
    node->month = month;
    node->year = year;
    list_add_tail(&node->list, &mydata);
    printk(KERN_INFO "Add element %d\n", i);
}

void show_birthday(void) {
    struct birthday *ptr;
    list_for_each_entry(ptr, &mydata, list) {
        printk(KERN_INFO "Birth: %d/%d/%d\n", ptr->year, ptr->month, ptr->day);
    }
}

int your_module_init(void) {
    printk(KERN_INFO "Loading Module\n");
    INIT_LIST_HEAD(&mydata);

    show_birthday();
    return 0;
}

void your_module_exit(void) {
    int i = 1;
    struct birthday *ptr, *useless;

    printk(KERN_INFO "Removing Module\n");
    // delete all elements
    list_for_each_entry_safe(ptr, useless, &mydata, list) {
        printk(KERN_INFO "Delete element %d\n", i);
        list_del(&ptr->list);
        kfree(ptr);
        i++;
    }
    // check
    show_birthday();
}

module_init( your_module_init );
module_exit( your_module_exit );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("list birthday");
MODULE_AUTHOR("Yi-Feng Chen");

