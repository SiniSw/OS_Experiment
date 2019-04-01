///Kernel/sys.c 121
///linux/Kernel/syscalls.h 379
///arch/x86/syscalls/syscall_64.tbl 342
#include<linux/fs.h>
#include<asm/segment.h>
#include<asm/uaccess.h>
#include<linux/buffer_head.h>
asmlinkage long sys_hello(void)	//334
{
	printk("Hello world!");
	return 1;
}
asmlinkage long sys_copyfile(const char* source_file,const char* dest_file)	//333
{
    const int BUF_SIZE=512;
    int fin,fout;
    char buf[BUF_SIZE];
    int copy_count;
    mm_segment_t old_fs=get_fs();   //获取当前线程的thread_info->addr_limit。
    set_fs(get_ds());               //将能访问的空间thread_info->addr_limit扩大到KERNEL_DS。
    if((fin=sys_open(source_file,O_RDONLY,0))==-1)
    {
        printk("Can't open %s\n",source_file);
        sys_exit(-1);
    }
    if((fout=sys_open(dest_file,O_CREAT|O_WRONLY|O_TRUNC,0666))==-1)
    {
        printk("Can't creat %s,mode %o\n",dest_file,0666);
        sys_exit(-1);
    }
    while((copy_count=sys_read(fin,buf,512))>0)
    {
        if(sys_write(fout,buf,copy_count)!=copy_count)
        {
            printk("Error on file %s\n",dest_file);
            sys_exit(-1);
        }
    }
    sys_close(fin);
    sys_close(fout);
    set_fs(old_fs);                 //将thread_info->addr_limit切换回原来值
    return 0;
}
