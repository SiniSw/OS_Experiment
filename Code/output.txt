#include<stdio.h>
#include<linux/kernel.h>
#include<sys/syscall.h>
#include<unistd.h>
int main()
{
    long int re=syscall(334);
    printf("Syscall hello returned %ld\n",re);
    re=syscall(333,"syscall_test.c","output.txt");
    printf("Syscall copyfile returned %ld\n",re);
    return 0;
}
