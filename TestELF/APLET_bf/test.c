#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#define max 30000
char code[max]={0};
int sourc[max]={0};
char data[8]={'+','-','>','<',',','.','[',']'};//指令表
int ptr=0;//数据指针
int eip;//指令指针
int ret[max]={0};//循环返回地址
int loop=0;//跟踪返回指针
extern void sleep(int ms);

bool check(int length)
{
    int i,k;
    for(i=0;i<length;i++)//编程时要注意边界检查
    {
        for(k=0;k<8;k++)
        {
            if(code[i]==data[k])
            {
                break;
            }
        }
        if(k==8)
        {
            printf("代码第%d含有非法命令:%c",i,code[i]);
            return false;
        }
    }
    return true;
}
int format(int length)//扫描并保存代码中的“[”和“]”一个表中
{
    int i,j;
    for(i=0,j=0;i<length;i++)
    {
        if((code[i]=='[')||(code[i]==']'))
        {
            ret[j++]=i;
        }
    }
    return j;
}
 
void addnum()//+操作
{
    sourc[ptr]++;
    eip++;
    return;
}
void subnum()//-操作
{
    sourc[ptr]--;
    eip++;
    return;
}
void addptr()//指针右移操作>
{
    ptr++;
    eip++;
    return;
}
void subptr()//指针左移操作<
 
{
    ptr--;
    eip++;
    return;
}
void print()//打印操作
{
    printf("%c",sourc[ptr]);
    eip++;
    return;
}
void get()//读取操作
{
    scanf("%d",sourc+ptr);
    eip++;
    return;
}
void leftloop(int len)//当执行到[，表示循环开始，就像C语言的while（...）{，
{
    sourc[ptr]<=0?eip=ret[loop=(len-1-loop)]+1:eip++;//sourc数组中保存整个程序需要的数据，初始全部为0，可以用他们来控制循环
    return;//len-1-loop这样理解，有[  [  ]  ]，在数组中的位置用下标loop跟踪，0,1,2,3，len是长度，就是C语言中循环嵌套循环体的意思
 
}
void rightloop(int len)
{
    eip=ret[loop=(len-1-loop)];//作用类似C语言的  }
    return;
}
int main()
{/*
    char name[200];
    int len,p=0;
    scanf("%s",name);
    FILE *fp;
    if(!(fp=fopen(name,"r")))
    {
        printf("错误文件！\n");
        system("pause");
        return;
    }
    while((code[p++]=fgetc(fp))!=EOF);//读取脚本文件
    len=strlen(code)-1;//因为fgetc会多读入一个文件末尾标志数据，所以长度要减一
	*/
	char inputcode[] = "++++++++++[>+>+++>+++++++>++++++++++<<<<-]>>>++.>+.+++++++..+++.<<++.>+++++++++++++++.>.+++.------.--------.<<+.<.";
	strcpy(code,inputcode);
	int len=strlen(inputcode);
	printf("code:%s\n",inputcode);
	
    if(!check(len))
    {
        //system("pause");
        return 1;
    }
    int length=format(len);
	printf("BF output:");
    for(eip=0;eip<len;)
    {
        switch(code[eip])
        {
            case '+':addnum();break;
            case '-':subnum();break;
            case '>':addptr();break;
            case '<':subptr();break;
            case '.':print();break;
            case ',':get();break;
            case '[':leftloop(length);loop++;break;
            case ']':rightloop(length);break;
        }
    }
    //fclose(fp);
    //system("pause");
    return 1;
}