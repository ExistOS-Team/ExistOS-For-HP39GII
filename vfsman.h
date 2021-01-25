#ifndef _VFSMAN_H_
#define _VFSMAN_H_

typedef struct file_descriptor_list
{
    struct file_descriptor_list *next_chain;
    char *path;
    int fd;
}file_descriptor_list;



#endif
