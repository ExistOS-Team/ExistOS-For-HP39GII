#ifndef _VFSMAN_H_
#define _VFSMAN_H_

#include "ff.h"

typedef struct file_descriptor_list
{
    struct file_descriptor_list *next_chain;
    int fd;
    char *path;
    int flags;
    int open_count;
    FIL *fatfs_fd;
}file_descriptor;


void dump_vfs_descriptor_chains();

int vfs_open(const char *path, int flags, int mode);
int vfs_read(int fd, void *buf, size_t nbytes);
int vfs_write(int fd, const void *buf, size_t nbytes);
off_t vfs_lseek(int fd, off_t offset, int whence);
int vfs_stat(const char *path, struct stat *st);
int vfs_fstat(int fd, struct stat *st);
int vfs_fsync(int fd);
int vfs_fclose(int fd);
int vfs_mkdir(const char *path);


#endif
