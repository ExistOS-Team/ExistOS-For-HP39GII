        .section .pretext
        .global initialize
initialize:
        sts.l   pr, @-r15
 
        ! set up TLB
        mov.l   Hmem_SetMMU, r3
        mov.l   address_one, r4 ! 0x8102000
        mov.l   address_two, r5 ! 0x8801E000
        jsr     @r3    ! _Hmem_SetMMU
        mov     #108, r6
 
        ! clear the BSS
        mov.l   bbss, r4   ! start
        mov.l   ebss, r5   ! end
        bra     L_check_bss
        mov     #0, r6
L_zero_bss:
        mov.l   r6, @r4        ! zero and advance
        add     #4, r4
L_check_bss:
        cmp/hs  r5, r4
        bf      L_zero_bss
 
        ! Copy the .data
        mov.l   bdata, r4  ! dest
        mov.l   edata, r5  ! dest limit
        mov.l   romdata, r6        ! source
        bra     L_check_data
        nop
L_copy_data:
        mov.l   @r6+, r3
        mov.l   r3, @r4
        add     #4, r4
L_check_data:
        cmp/hs  r5, r4
        bf      L_copy_data
 
        mov.l   bbss, r4
        mov.l   edata, r5
        sub     r4, r5              ! size of .bss and .data sections
        add     #4, r5
        mov.l   bssdatasize, r4
        mov.l   r5, @r4
 
        mov.l   GLibAddinAplExecutionCheck, r2
        mov     #0, r4
        mov     #1, r5
        jsr     @r2    ! _GLibAddinAplExecutionCheck(0,1,1);
        mov     r5, r6
 
        mov.l   CallbackAtQuitMainFunction, r3
        mov.l   exit_handler, r4
        jsr     @r3    ! _CallbackAtQuitMainFunction(&exit_handler)
        nop
        mov.l   main, r3
        jmp     @r3    ! _main()
        lds.l   @r15+, pr
 
_exit_handler:
        mov.l   r14, @-r15
        mov.l   r13, @-r15
        mov.l   r12, @-r15
        sts.l   pr, @-r15
 
        mov.l   Bdel_cychdr, r14
        jsr     @r14 ! _Bdel_cychdr
        mov     #6, r4
        jsr     @r14 ! _Bdel_cychdr
        mov     #7, r4
        jsr     @r14 ! _Bdel_cychdr
        mov     #8, r4
        jsr     @r14 ! _Bdel_cychdr
        mov     #9, r4
        jsr     @r14 ! _Bdel_cychdr
        mov     #10, r4
        
        mov.l   BfileFLS_CloseFile, r12
        mov     #4, r14
        mov     #0, r13
L_close_files:
        jsr     @r12 ! _BfileFLS_CloseFile
        mov     r13, r4
        add     #1, r13
        cmp/ge  r14, r13
        bf      L_close_files
 
        mov.l   flsFindClose, r12
        mov     #0, r13
L_close_finds:
        jsr     @r12 ! _flsFindClose
        mov     r13, r4
        add     #1, r13
        cmp/ge  r14, r13
        bf      L_close_finds
 
        lds.l   @r15+, pr
        mov.l   @r15+, r12
        mov.l   @r15+, r13
        mov.l   Bkey_Set_RepeatTime_Default, r2
        jmp     @r2    ! _Bkey_Set_RepeatTime_Default
        mov.l   @r15+, r14
 
.align 4
address_two:    .long 0x8801E000
address_one:    .long 0x8102000
Hmem_SetMMU:    .long _Hmem_SetMMU
GLibAddinAplExecutionCheck:     .long _GLibAddinAplExecutionCheck
CallbackAtQuitMainFunction:     .long _CallbackAtQuitMainFunction
Bdel_cychdr:    .long _Bdel_cychdr
BfileFLS_CloseFile:     .long _BfileFLS_CloseFile
flsFindClose:   .long _flsFindClose
Bkey_Set_RepeatTime_Default:    .long _Bkey_Set_RepeatTime_Default
bbss:      .long _bbss
ebss:      .long _ebss
edata:    .long _edata
bdata:    .long _bdata
romdata:        .long _romdata
bssdatasize:    .long _bssdatasize
 
exit_handler:   .long _exit_handler
main:      .long _main
 
_Hmem_SetMMU:
        mov.l   sc_addr, r2
        mov.l   1f, r0
        jmp     @r2
        nop
1:      .long 0x3FA
 
_Bdel_cychdr:
        mov.l   sc_addr, r2
        mov.l   1f, r0
        jmp     @r2
        nop
1:      .long 0x119
 
_BfileFLS_CloseFile:
        mov.l   sc_addr, r2
        mov.l   1f, r0
        jmp     @r2
        nop
1:      .long 0x1E7
 
_Bkey_Set_RepeatTime_Default:
        mov.l   sc_addr, r2
        mov.l   1f, r0
        jmp     @r2
        nop
1:      .long 0x244
 
_CallbackAtQuitMainFunction:
        mov.l   sc_addr, r2
        mov.l   1f, r0
        jmp     @r2
        nop
1:      .long 0x494
 
_flsFindClose:
        mov.l   sc_addr, r2
        mov.l   1f, r0
        jmp     @r2
        nop
1:      .long 0x218
 
_GLibAddinAplExecutionCheck:
        mov.l   sc_addr, r2
        mov     #0x13, r0
        jmp     @r2
        nop
sc_addr:        .long 0x80010070
.end
