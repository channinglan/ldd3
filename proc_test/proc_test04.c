    #include <linux/module.h>  
    #include <linux/sched.h>  
    #include <linux/proc_fs.h>  
    #include <linux/seq_file.h>  
    #include <linux/uaccess.h>  
    #include <linux/slab.h>  
      
    struct proc_head  
    {  
        struct list_head lhead;  
        int item_count;  
        int str_count;  
    };  
      
    struct proc_item  
    {  
        struct list_head litem;  
        char *buf;  
        int num;  
    };  
      
    struct proc_head *gp_head;  
      
    // linux/seq_file.h  
    // void * (*start) (struct seq_file *m, loff_t *pos);  
    // void (*stop) (struct seq_file *m, void *v);  
    // void * (*next) (struct seq_file *m, void *v, loff_t *pos);  
    // int (*show) (struct seq_file *m, void *v);  
      
    /** 
    * author:  aran 
    * fuction: seq_operations -> start 
    */  
    static void *my_seq_start(struct seq_file *m, loff_t *pos)  
    {  
        struct proc_item *entry;  
      
        if (0 == *pos)  
        {  
            seq_printf(m, "List has %d items, total %d bytes\n", gp_head->item_count, gp_head->str_count);  
        }  
      
        // get first item  
        ++*pos;  
        list_for_each_entry(entry, &gp_head->lhead, litem)  
        {  
            if (*pos == entry->num)  
            {  
                return entry;  
            }  
        }  
        return NULL;  
    }  
      
    /** 
    * author:  aran 
    * fuction: seq_operations -> next 
    */  
    static void *my_seq_next(struct seq_file *m, void *v, loff_t *pos)  
    {  
        struct proc_item *entry;  
      
        // get next item  
        ++*pos;  
        list_for_each_entry(entry, &gp_head->lhead, litem)  
        {  
            if (*pos == entry->num)  
            {  
                return entry;  
            }  
        }  
        return NULL;  
    }  
      
    /** 
    * author:  aran 
    * fuction: seq_operations -> stop 
    */  
    static void my_seq_stop(struct seq_file *m, void *v)  
    {  
        // clean sth.  
        // nothing to do  
    }  
      
    /** 
    * author:  aran 
    * fuction: seq_operations -> show 
    */  
    static int my_seq_show(struct seq_file *m, void *v)  
    {  
        struct proc_item *tmp = v;  
        seq_printf(m, "%s", tmp->buf);  
      
        return 0;  
    }  
      
    // global var  
    static struct seq_operations my_seq_fops =   
    {  
        .start  = my_seq_start,  
        .next   = my_seq_next,  
        .stop   = my_seq_stop,  
        .show   = my_seq_show,  
    };  
      
    // file_operations  
    // int (*open) (struct inode *, struct file *)  
    // ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *)  
      
    /** 
    * author:  aran 
    * fuction: file_operations -> open 
    */  
    static int proc_seq_open(struct inode *inode, struct file *file)  
    {  
        return seq_open(file, &my_seq_fops);  
    }  
      
    /** 
    * author:  aran 
    * fuction: file_operations -> write 
    */  
    static ssize_t proc_seq_write(struct file *file, const char __user *buffer, size_t count, loff_t *f_pos)  
    {  
        struct proc_item *tmp;  
        int ret;  
      
        // allocate proc_item  
        tmp = (struct proc_item *)kzalloc(sizeof(*tmp), GFP_KERNEL);  
        if (NULL == tmp)  
        {  
            ret = -ENOMEM;  
            goto err_kzalloc1;  
        }  
        INIT_LIST_HEAD(&tmp->litem);  
      
        // allocate str buf  
        tmp->buf = (char *)kzalloc(count, GFP_KERNEL);  
        if (NULL == tmp->buf)  
        {  
            ret = -ENOMEM;  
            goto err_kzalloc2;  
        }  
      
        if (0 != copy_from_user(tmp->buf, buffer, count))  
        {  
            ret = -1;  
            goto err_copy;  
        }  
      
        list_add(&tmp->litem, &gp_head->lhead);  
        gp_head->item_count++;  
        gp_head->str_count += count;  
        tmp->num = gp_head->item_count;   
      
        return count;  
      
    err_copy:  
        kfree(tmp->buf);  
    err_kzalloc2:  
        kfree(tmp);  
    err_kzalloc1:  
        return ret;  
    }  
      
    // global var  
    static struct file_operations proc_seq_fops =   
    {  
        .owner      = THIS_MODULE,  
        .open       = proc_seq_open,  
        .read       = seq_read,  
        .write      = proc_seq_write,  
        .llseek     = seq_lseek,  
        .release    = seq_release,  
    };  
      
    static int __init my_init(void)  
    {  
        struct proc_dir_entry *file;  
        int ret;  
      
        // allocate & init proc_head  
        gp_head = (struct proc_head *)kzalloc(sizeof(*gp_head), GFP_KERNEL);  
        if (NULL == gp_head)  
        {  
            ret = -ENOMEM;  
            goto err_kzalloc;  
        }  
        gp_head->item_count = 0;  
        gp_head->str_count = 0;  
        INIT_LIST_HEAD(&gp_head->lhead);  
      
        // create "/proc/proc_seq" file  
        file = proc_create_data(  
            "proc_seq", // name  
            0666,       // mode  
            NULL,       // parent dir_entry  
            &proc_seq_fops, // file_operations  
            NULL        // data  
            );  
        if (NULL == file)  
        {  
            printk("Count not create /proc/proc_seq file!\n");  
            ret = -1;  
            goto err_proc_create_data;  
        }  
      
        return 0;  
      
    err_proc_create_data:  
        kfree(gp_head);  
    err_kzalloc:  
        return ret;  
    }  
      
    static void __exit my_exit(void)  
    {  
        struct proc_item *tmp1, *tmp2;  
      
        remove_proc_entry("proc_seq", NULL);  
        list_for_each_entry_safe(tmp1, tmp2, &gp_head->lhead,litem)  
        {  
            list_del(&tmp1->litem);  
            kfree(tmp1->buf);  
            kfree(tmp1);  
        }  
        kfree(gp_head);  
    }  
      
    module_init(my_init);  
    module_exit(my_exit);  
      
    MODULE_AUTHOR("aran");  
    MODULE_LICENSE("GPL");  
