#define USER_MODE 0x10
#define FIQ_MODE 0x11
#define IRQ_MODE 0x12
#define SVC_MODE 0x13
#define ABT_MODE 0x17
#define UND_MODE 0x1b
#define SYS_MODE 0x1f

extern int main();

void _exit(int code){
	while(1);
}

void switch_mode(int mode) __attribute__((naked));
void switch_mode(int mode) {
    __asm volatile("and r0,r0,#0x1f");
    __asm volatile("mrs r1,cpsr_all");
    __asm volatile("bic r1,r1,#0x1f");
    __asm volatile("orr r1,r1,r0");
    __asm volatile("mov r0,lr"); // GET THE RETURN ADDRESS **BEFORE** MODE CHANGE
    __asm volatile("msr cpsr_all,r1");
    __asm volatile("bx r0");
}

void __start(){
	//switch_mode(USER_MODE);
	__asm volatile("mov sp,#0x80000");
	
	(*((int (*)())(  (*((unsigned int *)0xC0072200))  )))();
	_exit(main());
	while(1);
}

