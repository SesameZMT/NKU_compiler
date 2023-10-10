.arch armv5t 

.data
    num:    .word 5
    result: .word 0
    newline: .asciz "\n"

.text
    // main 标签的声明
    .global main

    // main 标签的定义
    main:
        push {lr}          // 保存返回地址
        ldr r0, =num       // 加载 num 的地址到 r0
        ldr r0, [r0]       // 加载 num 的值到 r0
        bl factorial       // 调用 factorial 函数
        ldr r2, =result    // 加载 result 的地址到 r2
        str r0, [r2]       // 将结果保存在 result 中
        ldr r0, [r2]       // 加载 result 的值到 r0
        bl print           // 调用 print 函数
        pop {pc}           // 返回到调用 main 的地址

    // 其他标签的定义
    factorial:
        push {lr}          // 保存返回地址
        cmp r0, #0         // 判断 n 是否为 0
        beq base_case      // 相等则跳转到 base_case
        sub sp, sp, #4     // 为 n - 1 分配栈空间
        str r0, [sp]       // 将参数 n 保存在栈上
        sub r0, r0, #1     // r0 = n - 1
        bl factorial       // 递归调用 factorial(n - 1)
        ldr r1, [sp]       // 加载递归调用的结果到寄存器 r1
        add sp, sp, #4     // 释放栈空间
        mul r0, r1, r0     // r0 = r1 * r0
        pop {pc}           // 返回到调用 factorial 的地址

    base_case:
        mov r0, #1         // n = 0 时，返回 1
        pop {pc}           // 返回到调用 factorial 的地址

    print:
        push {lr}          // 保存返回地址
        mov r1, r0         // 将要打印的值保存在 r1
        ldr r0, =newline   // 加载换行符的地址到 r0
        bl printf          // 调用 printf 函数
        pop {pc}           // 返回到调用 print 的地址