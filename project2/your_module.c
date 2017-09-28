#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/list.h>

void dfs_process(struct task_struct *parent, int depth)
{
    struct task_struct *task;
    struct list_head *list;

    printk(KERN_INFO "%*s%d %s\n", depth, "", parent->pid, parent->comm);
    list_for_each(list, &parent->children) {
        task = list_entry(list, struct task_struct, sibling);
        dfs_process(task, depth + 1);
    }
}

int my_module_init(void)
{
    struct task_struct *task;
    printk(KERN_INFO "Loading Module\n");
    printk(KERN_INFO "LINEAR:\n");
    for_each_process(task) {
        printk(KERN_INFO "%d %s\n", task->pid, task->comm);
    }

    printk(KERN_INFO "DFS:\n");
    dfs_process(&init_task, 0);
    return 0;
}

void my_module_exit(void)
{
    printk(KERN_INFO "Removing Module\n");
}

module_init( my_module_init );
module_exit( my_module_exit );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("list task");
MODULE_AUTHOR("Yi-Feng Chen");
