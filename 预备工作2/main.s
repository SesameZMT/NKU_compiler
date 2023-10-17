.arch armv5t // 指定目标架构为 ARMv5T

.data // 数据段开始
    num:    .word 5 // 定义一个名为 num 的单词，初始值为 5
    result: .word 0 // 定义一个名为 result 的单词，初始值为 0
    format: .asciz "%d\n" // 定义一个名为 format 的 ASCIIZ 字符串，内容为 "%d\n"

.text // 文本段开始
    .global main // 声明 main 标签为全局标签

    main: // main 标签的定义开始
        push {lr} // 将链接寄存器（lr）压入栈中以保存返回地址
        ldr r0, =num // 将 num 的地址加载到寄存器 r0 中
        ldr r0, [r0] // 将 num 的值加载到寄存器 r0 中
        bl factorial // 调用 factorial 函数计算阶乘
        ldr r2, =result // 将 result 的地址加载到寄存器 r2 中
        str r0, [r2] // 将阶乘的结果（在寄存器 r0 中）存储到 result 中
        ldr r1, [r2] // 将 result 的值加载到寄存器 r1 中
        ldr r0, =format // 将 format 的地址加载到寄存器 r0 中
        bl printf // 调用 printf 函数打印结果
        pop {pc} // 从栈中弹出返回地址并跳转到该地址

    factorial: // factorial 标签的定义开始
        push {lr} // 将链接寄存器（lr）压入栈中以保存返回地址
        cmp r0, #0 // 比较寄存器 r0 和 0 的值
        beq base_case // 如果 r0 等于 0，则跳转到 base_case 标签处执行
        sub sp, sp, #4 // 在栈中为参数 n - 1 分配空间
        str r0, [sp] // 将参数 n 存储在栈中
        sub r0, r0, #1 // 计算 n - 1 的值并将结果保存在寄存器 r0 中
        bl factorial // 递归调用 factorial 函数计算 (n - 1) 的阶乘
        ldr r1, [sp] // 将递归调用的结果加载到寄存器 r1 中
        add sp, sp, #4 // 释放栈空间
        mul r0, r1, r0 // 计算 n * (n - 1) 的阶乘并将结果保存在寄存器 r0 中
        pop {pc} // 从栈中弹出返回地址并跳转到该地址

       base_case: // base_case 标签的定义开始
        mov r0, #1 // 当 n 等于 0 时，将寄存器 r0 的值设置为 1 并返回该值
        pop {pc} // 从栈中弹出返回地址并跳转到该地址

