#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/init.h>
#include<linux/types.h>
#include<linux/uaccess.h>


#include<linux/version.h>

static int device_num=0;
static char buffer[1024]="Driver";
static int open_nr=0;   //打开设备的进程数，用于内核的互斥

static int driver_open(struct inode *inode,struct file *filp);
static int driver_release(struct inode *inode,struct file *filp);
static ssize_t driver_read(struct file *filp,char __user *buf,size_t count,loff_t *f_pos);
static ssize_t driver_write(struct file *filp,const char __user *buf,size_t count,loff_t *f_pos);

static struct file_operations driver_fops={
    .read=driver_read,
    .write=driver_write,
    .open=driver_open,
    .release=driver_release,
};

static int driver_open(struct inode *inode,struct file *flip)
{
    printk("\nMain device is %d, and the slave device is %d\n", MAJOR(inode->i_rdev), MINOR(inode->i_rdev));
    if (open_nr == 0) {
        open_nr++;
        try_module_get(THIS_MODULE);    //判断module模块是否处于活动状态，该模块处于活动状态且对它引用计数加1,正确返回true，错误返回false
        return 0;
    }
    else {
        printk(KERN_ALERT "Another process open the char device.\n");//进程挂起,KERN_ALERT表示printk必须立即响应
        return -1;
    }
}
static ssize_t driver_read(struct file *file, char __user *buf, size_t count, loff_t *f_pos)
{
//if (buf == NULL) return 0;
    if (copy_to_user(buf, buffer, sizeof(buffer))) //读缓冲,完成用户空间到内核空间的复制
    {
    return -1;
    }
    return sizeof(buffer);
}
static ssize_t driver_write(struct file *file, const char __user *buf, size_t count, loff_t *f_pos)
{
    //if (buf == NULL) return 0;
    if (copy_from_user(buffer, buf, sizeof(buffer))) //写缓冲,完成内核空间到用户空间的复制
    {
        return -1;
    }
    return sizeof(buffer);
}
static int driver_release(struct inode *inode, struct file* filp)
{
    open_nr--; //该驱动使用数减1
    printk("The device is released!\n");
    module_put(THIS_MODULE);
    return 0;
}
static int __init driver_init(void)
{
    int result;
    printk(KERN_ALERT "Begin to init Char Device!"); //注册设备
    //向系统的字符登记表登记一个字符设备
    result = register_chrdev(device_num, "newdriver", &driver_fops);
    if (result < 0) {
        printk(KERN_WARNING "mydriver: register failure\n");
    return -1;
    }
    else {
        printk("mydriver: register success!\n");
        device_num = result;
        return 0;
    }
}
static void __exit driver_exit(void)
{
    printk(KERN_ALERT "Unloading...\n");
    unregister_chrdev(device_num, "newdriver"); //注销设备
    printk("unregister success!\n");
}

module_init(driver_init);
module_exit(driver_exit);
MODULE_LICENSE("GPL");