define i32 @main() {
B7:
  %t9 = alloca i32, align 4
  %t8 = alloca i32, align 4
  store i32 1, i32* %t8, align 4
  store i32 2, i32* %t9, align 4
  %t2 = load i32, i32* %t8, align 4
  %t3 = load i32, i32* %t9, align 4
  %t4 = icmp sle i32 %t2, %t3
  br i1 %t4, label %B10, label %B15
B10:                               	; preds = %B7
  store i32 1, i32* %t8, align 4
  br label %B12
B15:                               	; preds = %B7
  br label %B11
B12:                               	; preds = %B10, %B11
  ret i32 0
B11:                               	; preds = %B15
  store i32 2, i32* %t8, align 4
  br label %B12
}
