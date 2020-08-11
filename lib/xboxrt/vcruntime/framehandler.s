// int __cdecl __CxxFrameHandler3 (PEXCEPTION_RECORD, PEXCEPTION_REGISTRATION, _CONTEXT *, void *);
// Additionally, a function information table is passed in the eax register
.globl ___CxxFrameHandler3
___CxxFrameHandler3:
    pushl %eax
    pushl 20(%esp)
    pushl 20(%esp)
    pushl 20(%esp)
    pushl 20(%esp)
    calll _CxxFrameHandlerVC8
    addl $20, %esp
    retl
