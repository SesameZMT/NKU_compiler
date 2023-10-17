.arch armv5t // ָ��Ŀ��ܹ�Ϊ ARMv5T

.data // ���ݶο�ʼ
    num:    .word 5 // ����һ����Ϊ num �ĵ��ʣ���ʼֵΪ 5
    result: .word 0 // ����һ����Ϊ result �ĵ��ʣ���ʼֵΪ 0
    format: .asciz "%d\n" // ����һ����Ϊ format �� ASCIIZ �ַ���������Ϊ "%d\n"

.text // �ı��ο�ʼ
    .global main // ���� main ��ǩΪȫ�ֱ�ǩ

    main: // main ��ǩ�Ķ��忪ʼ
        push {lr} // �����ӼĴ�����lr��ѹ��ջ���Ա��淵�ص�ַ
        ldr r0, =num // �� num �ĵ�ַ���ص��Ĵ��� r0 ��
        ldr r0, [r0] // �� num ��ֵ���ص��Ĵ��� r0 ��
        bl factorial // ���� factorial ��������׳�
        ldr r2, =result // �� result �ĵ�ַ���ص��Ĵ��� r2 ��
        str r0, [r2] // ���׳˵Ľ�����ڼĴ��� r0 �У��洢�� result ��
        ldr r1, [r2] // �� result ��ֵ���ص��Ĵ��� r1 ��
        ldr r0, =format // �� format �ĵ�ַ���ص��Ĵ��� r0 ��
        bl printf // ���� printf ������ӡ���
        pop {pc} // ��ջ�е������ص�ַ����ת���õ�ַ

    factorial: // factorial ��ǩ�Ķ��忪ʼ
        push {lr} // �����ӼĴ�����lr��ѹ��ջ���Ա��淵�ص�ַ
        cmp r0, #0 // �ȽϼĴ��� r0 �� 0 ��ֵ
        beq base_case // ��� r0 ���� 0������ת�� base_case ��ǩ��ִ��
        sub sp, sp, #4 // ��ջ��Ϊ���� n - 1 ����ռ�
        str r0, [sp] // ������ n �洢��ջ��
        sub r0, r0, #1 // ���� n - 1 ��ֵ������������ڼĴ��� r0 ��
        bl factorial // �ݹ���� factorial �������� (n - 1) �Ľ׳�
        ldr r1, [sp] // ���ݹ���õĽ�����ص��Ĵ��� r1 ��
        add sp, sp, #4 // �ͷ�ջ�ռ�
        mul r0, r1, r0 // ���� n * (n - 1) �Ľ׳˲�����������ڼĴ��� r0 ��
        pop {pc} // ��ջ�е������ص�ַ����ת���õ�ַ

       base_case: // base_case ��ǩ�Ķ��忪ʼ
        mov r0, #1 // �� n ���� 0 ʱ�����Ĵ��� r0 ��ֵ����Ϊ 1 �����ظ�ֵ
        pop {pc} // ��ջ�е������ص�ַ����ת���õ�ַ

