/*
	Mostly adapted from the Gint syscall file
*/

	.global	_DisplayFKeyIcons
	.global	_GetFKeyIconPointer
	.global	_DisplayFKeyIcon
	.global _Cursor_SetPosition
	.global	_Cursor_SetFlashStyle
	.global _Cursor_SetFlashMode
	.global _Cursor_GetSettings
	.global _Cursor_SetFlashOff
	.global	_Cursor_SetFlashOn
	.global	_Setup_GetEntry
	.global	_Setup_SetEntry
	.global	_TestMode
	.global _GetVRAMAddress
	.global _SetQuitHandler
	.global _IsAnyKeyDown
	.global _RTC_GetTicks
	.global _RTC_SetDateTime
	.global _getkeywait
	.global _MB_ElementCount
	.global _itoa
	.global _Bkey_SetAllFlags
	.global ___free
	.global ___malloc



_DisplayFKeyIcons:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0x04b0

_GetFKeyIconPointer:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0x0268

_DisplayFKeyIcon:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0x04d1

_Cursor_SetPosition:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0x0138

_Cursor_SetFlashStyle:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0x0139

_Cursor_SetFlashMode:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0x013A

_Cursor_GetSettings:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0x013B

_Cursor_SetFlashOff:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0x0812

_Cursor_SetFlashOn:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0x0811

_Setup_GetEntry:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0x04dc

_Setup_SetEntry:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0x04dd

_TestMode:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0x0924

_GetVRAMAddress:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0x0135

_SetQuitHandler:	
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0x0494
	
_IsAnyKeyDown:	
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0x024a

_RTC_GetTicks:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0x003b
	
_RTC_SetDateTime:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0x023e
	
_MB_ElementCount:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0x0533
	
_getkeywait:	
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0x0247
	
_itoa:	
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0x0541
	
_Bkey_SetAllFlags:	
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0x0EA1
	
___malloc:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0xacd

___free:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0xacc




	.align	4

syscall_table:
	.long	0x80010070
