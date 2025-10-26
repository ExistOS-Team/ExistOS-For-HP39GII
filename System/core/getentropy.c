#include <errno.h>
#include <unistd.h>
int _getentropy(void *buffer, size_t length) {
    
    if (buffer == NULL) {
        errno = EFAULT;
        return -1;
    }
    
    if (length > 256) {
        errno = EIO;
        return -1;
    }
    unsigned char *buf = (unsigned char *)buffer;
    static unsigned int seed = 0x12345678;
    
    for (size_t i = 0; i < length; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        buf[i] = (seed >> 16) & 0xFF;
    }
    
    return 0;
}

int _getentropy_r(struct _reent *reent, void *buffer, size_t length) {
    return _getentropy(buffer, length);
}