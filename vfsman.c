
#include <stdlib.h>
#include <sys/errno.h>
#include "ff.h"
#include "vfsman.h"

file_descriptor *file_descriptor_list = NULL;

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
    file_descriptor_list = pvPortMalloc(sizeof(file_descriptor));
    if(file_descriptor_list == NULL){
        return -1;
    }
    file_descriptor_list -> fd = 0;
    file_descriptor_list -> next_chain = NULL;



    return 0;
}

int vfs_open(const char *path, int flags, int mode){
    if(file_descriptor_list == NULL){
        file_descriptor_list = pvPortMalloc(sizeof(file_descriptor));
        if(file_descriptor_list == NULL){
            return -ENOMEM;
        }


    }

}