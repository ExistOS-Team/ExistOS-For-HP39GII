
#include <stdlib.h>
#include <stdio.h>
#include <sys/errno.h>
#include <fcntl.h>
#include "ff.h"
#include "vfsman.h"

file_descriptor *file_descriptor_list = NULL;
int fd_count = 3;


int fatfs_error_to_vfs(FRESULT fr){
	switch(fr)
	{
		case FR_OK:					return 0;			//(0) 无错误
		case FR_DISK_ERR:			return -EIO;		//(1) 低级磁盘I / O层发生硬错误
		case FR_INT_ERR:			return -EPIPE;		//(2) 断言失败
		case FR_NOT_READY:			return -EIO;		//(3) 物理驱动器无法工作
		case FR_NO_FILE:			return -ENOENT;		//(4) 找不到文件
		case FR_NO_PATH:			return -ENOENT;		//(5) 找不到路径
		case FR_INVALID_NAME:		return -ENOEXEC;	//(6) 路径名格式无效
		case FR_DENIED:				return -ENOSPC;		//(7) 由于访问被禁止或目录已满而拒绝访问
		case FR_EXIST:				return -EACCES;		//(8) 由于禁止访问而拒绝访问 
		case FR_INVALID_OBJECT:		return -ENXIO;		//(9) 文件/目录对象无效
		case FR_WRITE_PROTECTED:	return -EROFS;		//(10) 物理驱动器受写保护
		case FR_INVALID_DRIVE:		return -ENXIO;		//(11) 逻辑驱动器号无效
		case FR_NOT_ENABLED:		return -ENXIO;		//(12) 该卷没有工作区
		case FR_NO_FILESYSTEM:		return -EPERM;		//(13) 没有有效的FAT卷
		case FR_MKFS_ABORTED:		return -EPERM;		//(14) 由于任何问题f_mkfs（）中止
		case FR_TIMEOUT:			return -EBUSY;		//(15) 无法在限定时间内获得访问卷的授权
		case FR_LOCKED:				return -EACCES;		//(16) 根据文件共享策略拒绝该操作
		case FR_NOT_ENOUGH_CORE:	return -ENOMEM;		//(17) 无法分配LFN工作缓冲区
		case FR_TOO_MANY_OPEN_FILES:return -EMFILE;		//(18) 打开文件数> FF_FS_LOCK
		case FR_INVALID_PARAMETER:	return -EINVAL;		//(19) 给定参数无效
		default:return -EPERM;
	}

}

int vfs_init(){

    return 0;
}

void dump_vfs_descriptor_chains(){
    file_descriptor *current_file_descriptor;
    current_file_descriptor = file_descriptor_list;
    if(current_file_descriptor == NULL){
        return;
    }
    printf("-----------------dump vfs descriptor-----------\n");
    do{
        printf("#%d:\n",current_file_descriptor->fd);
        printf("\t path:%s\n",current_file_descriptor->path);
        printf("\t open count:%d\n",current_file_descriptor->open_count);
        printf("\t fatfs_fd:%08X\n",current_file_descriptor->fatfs_fd);
        printf("\t flags:%08X\n",current_file_descriptor->flags);
        current_file_descriptor = current_file_descriptor->next_chain;
    }while(current_file_descriptor != NULL);
    printf("------------------------------------------------\n");
}

FIL *fd_to_fatfs_obj(int fd){
    file_descriptor *current_file_descriptor = file_descriptor_list;
    if(fd <= 0)return NULL;
    if(current_file_descriptor == NULL)return NULL;
    do{
        if(current_file_descriptor -> fd == fd){
            return current_file_descriptor -> fatfs_fd;
        }
        current_file_descriptor = current_file_descriptor -> next_chain;
    }while(current_file_descriptor != NULL);
    return NULL;
}

file_descriptor *get_fd_chain(int fd){
    file_descriptor *current_file_descriptor = file_descriptor_list;
    if(fd <= 0)return NULL;
    if(current_file_descriptor == NULL)return NULL;
    do{
        if(current_file_descriptor -> fd == fd){
            return current_file_descriptor;
        }
        current_file_descriptor = current_file_descriptor -> next_chain;
    }while(current_file_descriptor != NULL);
    return NULL;    
}

file_descriptor *get_fd_last_chain(int fd){
    file_descriptor *current_file_descriptor = file_descriptor_list;
    file_descriptor *last_chain = NULL;
    if(fd <= 0)return NULL;
    if(current_file_descriptor == NULL)return NULL;
    do{
        if(current_file_descriptor -> fd == fd){
            return last_chain;
        }
        last_chain = current_file_descriptor;
        current_file_descriptor = current_file_descriptor -> next_chain;
    }while(current_file_descriptor != NULL);
    return NULL;    
}

int vfs_open(const char *path, int flags, int mode){
    file_descriptor *current_file_descriptor;
    file_descriptor *last_file_descriptor;
    unsigned int open_mode = 0;
    FRESULT fr;
    if(flags & O_WRONLY){
        open_mode |= FA_WRITE;
    }
    if(flags & O_RDWR){
        open_mode |= FA_WRITE | FA_READ;
    }
    if((open_mode & (FA_WRITE | FA_READ)) == 0){
        open_mode |= FA_READ;
    }

	if((flags & O_CREAT) && (flags & O_EXCL))			//如果文件存在而且同时指定的标志位O_CREAT，则返回-EEXIST；如果文件不存在，则创建文件
	{
		open_mode |= FA_CREATE_NEW;
	}
	else if(flags & O_CREAT)				//不存在则创建，存在则打开
	{
		open_mode |= FA_OPEN_ALWAYS;
	}

	if(flags & O_TRUNC)						//截断，相当于每次新建
	{
		open_mode |= FA_CREATE_ALWAYS;	
	}
	if(flags & O_APPEND)						//追加方式
	{
		open_mode |= FA_OPEN_APPEND;	
	}

    char *upcase_path;
    upcase_path = pvPortMalloc(strlen(path));
    if(upcase_path == NULL){
        return -ENOMEM;
    }
    strcpy(upcase_path, path);
    upcase_path = strupr(upcase_path);
    
    if(file_descriptor_list == NULL){
        file_descriptor_list = pvPortMalloc(sizeof(file_descriptor));
        if(file_descriptor_list == NULL){
            return -ENOMEM;
        }
        file_descriptor_list->path = NULL;
        file_descriptor_list->flags = 0;
        file_descriptor_list->fd = 0;
        file_descriptor_list->open_count = 0;
        file_descriptor_list->next_chain = NULL;
        file_descriptor_list->fatfs_fd = NULL;
        current_file_descriptor = file_descriptor_list;
        goto insert_do_first_chain;
    }

    current_file_descriptor = file_descriptor_list;

    do{
        if(current_file_descriptor->path != NULL){
            if(strcmp(upcase_path,current_file_descriptor->path) == 0){
                current_file_descriptor->open_count++;
                return current_file_descriptor->fd;
            }
        }

        last_file_descriptor = current_file_descriptor;
        current_file_descriptor = current_file_descriptor -> next_chain;
    }while(current_file_descriptor != NULL);

    

    last_file_descriptor->next_chain = pvPortMalloc(sizeof(file_descriptor));
    if(last_file_descriptor->next_chain == NULL){
        return -ENOMEM;
    }

    current_file_descriptor = last_file_descriptor->next_chain;


    insert_do_first_chain:

    current_file_descriptor->fatfs_fd = pvPortMalloc(sizeof(FIL));
    if(current_file_descriptor->fatfs_fd == NULL){
        return -ENOMEM;
    }
     
    fr = f_open(current_file_descriptor->fatfs_fd,path,open_mode);
    if(fr != FR_OK){

        vPortFree(current_file_descriptor->fatfs_fd);
        vPortFree(last_file_descriptor->next_chain);
        vPortFree(upcase_path);
        last_file_descriptor->next_chain = NULL;

        if((flags & O_CREAT) && (flags & O_EXCL) && (fr ==FR_EXIST))	 //如果文件存在而且同时指定的标志位O_CREAT，则返回-EEXIST；
		{
			return -EEXIST;
		}
        return fatfs_error_to_vfs(fr);
    }

    current_file_descriptor->fd = fd_count;
    fd_count++;
    current_file_descriptor->flags = flags;
    current_file_descriptor->open_count = 1;
    current_file_descriptor->next_chain = NULL;
    current_file_descriptor->path = upcase_path;

    return current_file_descriptor->fd;

}

int vfs_read(int fd, void *buf, size_t nbytes){
    FRESULT fr;
	UINT br;
    FIL *fatfs_fd;
    if(nbytes == 0){
        return 0;
    }
    if(fd <= 0)return -ENXIO;
    fatfs_fd = fd_to_fatfs_obj(fd);
    if(fatfs_fd == NULL)return -ENXIO;

    *((char *)buf) = 0;
    fr = f_read(fatfs_fd,buf,nbytes,&br);
    if(fr != FR_OK){
        return fatfs_error_to_vfs(fr);
    }
    return (int)br;
}

off_t vfs_lseek(int fd, off_t offset, int whence){
    FRESULT fr;
    FIL *fatfs_fd;
    long long temp;

    if(fd <= 0) return -ENXIO;
    fatfs_fd = fd_to_fatfs_obj(fd);
    if(fatfs_fd == NULL)return -ENXIO;

    switch (whence)
    {
    case SEEK_SET:
        fr = f_lseek(fatfs_fd, offset);
        if(fr == FR_OK)
		{
			return (off_t)fatfs_fd->fptr;
        }
        return fatfs_error_to_vfs(fr);
    case SEEK_CUR:
        temp = (off_t)fatfs_fd->fptr;
        temp += offset;
        if(temp >= UINT32_MAX) temp = UINT32_MAX-1;
        fr = f_lseek(fatfs_fd, temp);
        if(fr == FR_OK)
		{
			return (off_t)fatfs_fd->fptr;
        }
        return fatfs_error_to_vfs(fr);
    case SEEK_END:
        temp = (off_t)fatfs_fd->obj.objsize;
        temp -= offset;
        if(temp <= 0) temp = 0;
        fr = f_lseek(fatfs_fd, temp);
        if(fr == FR_OK)
		{
			return (off_t)fatfs_fd->fptr;
        }
        return fatfs_error_to_vfs(fr);       
    default:
        return -EINVAL;
    }
} 



int vfs_write(int fd, const void *buf, size_t nbytes){
    FRESULT fr;
	UINT br;
    FIL *fatfs_fd;
    volatile char page_test;
    fatfs_fd = fd_to_fatfs_obj(fd);
    if(fatfs_fd == NULL)return -ENXIO;
    if(nbytes == 0){
        return 0;
    }
    page_test = *((char *)buf);
    fr = f_write(fatfs_fd, buf, nbytes, (UINT *)&br);    
    if(fr != FR_OK){
        return fatfs_error_to_vfs(fr);
    }
    return br;
}

int vfs_fsync(int fd){
    FRESULT fr;
    FIL *fatfs_fd;
    fatfs_fd = fd_to_fatfs_obj(fd);
    if(fatfs_fd == NULL)return -ENXIO;

    fr = f_sync(fatfs_fd);
    return fatfs_error_to_vfs(fr);

}

int vfs_fclose(int fd){
    FRESULT fr;
    FIL *fatfs_fd;
    file_descriptor *current_chain;
    file_descriptor *last_chain;
    fatfs_fd = fd_to_fatfs_obj(fd);
    current_chain = get_fd_chain(fd);
    if(fatfs_fd == NULL)return -ENXIO;
    if(current_chain == NULL){
        return -ENXIO;
    }
    last_chain = get_fd_last_chain(fd);
    if(current_chain -> open_count > 1){
        fr = f_sync(fatfs_fd);
        if(fr != FR_OK)return fatfs_error_to_vfs(fr);
        current_chain -> open_count--;
        return fatfs_error_to_vfs(fr);
    }

    fr = f_close(fatfs_fd);
    if(fr != FR_OK){
        return fatfs_error_to_vfs(fr);
    }
    vPortFree(fatfs_fd);
    current_chain->fatfs_fd = NULL;
    if(last_chain != NULL){
        last_chain->next_chain = current_chain->next_chain;
    }else{
        file_descriptor_list = current_chain->next_chain;
    }
    vPortFree(current_chain);
    return fatfs_error_to_vfs(fr);
}


int vfs_fstat(int fd, struct stat *st)
{
    FRESULT fr;
    FIL *fatfs_fd;
    fatfs_fd = fd_to_fatfs_obj(fd);
    if(fatfs_fd == NULL)return -ENXIO;       

	st->st_size = fatfs_fd->obj.objsize;	//文件大小
	st->st_mode = 0777;				    //文件属性

    return 0;
}

int vfs_stat(const char *path, struct stat *st)
{
    FRESULT fr;
    FILINFO *file_info;
    file_info = pvPortMalloc(sizeof(FILINFO));
    if(file_info == NULL){
        return -ENOMEM;
    }
    fr = f_stat(path, file_info);
    if(fr == FR_OK){
		st->st_size = file_info->fsize;		//文件大小
		st->st_mode = file_info->fattrib;	//文件属性
    }
    vPortFree(file_info);
    return fatfs_error_to_vfs(fr);
}

int vfs_mkdir(const char *path)
{
    FRESULT fr;
    if(path == NULL) return -ENXIO;
    fr = f_mkdir (path);
    return fatfs_error_to_vfs(fr);
    
}

