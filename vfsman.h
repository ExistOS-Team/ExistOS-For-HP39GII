#ifndef _VFSMAN_H_
#define _VFSMAN_H_

typedef struct file_descriptor_list
{
    struct file_descriptor_list *next_chain;
    int fd;
    char *path;
    int flags;
    int mode;
}file_descriptor;


int vfs_open(const char *path, int flags, int mode);



#endif
