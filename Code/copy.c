#include<stdio.h>
#include<stdlib.h>
void main(int argc,char *argv[])
{
	int word;				//putchar以字符对应int值传参
	if(argc!=3){				//判断参数数量是否正确
		printf("parametric error!\n");	
		exit(1);
	}
	if(freopen(argc[1],"r",stdin)==NULL){
		printf("can't open %s\n", argv[1]);
		exit(1);
	}
	freopen(argv[2],"w",stdout);
	while((word=getchar()!=EOF)
		putchar(word);
	fclose(stdin);
	fclose(stdout);
}
