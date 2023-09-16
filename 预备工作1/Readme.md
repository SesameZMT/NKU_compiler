# 预初理
```sh
gcc -E main.c -o main.i
```



# 编译
1. 词法分析
   ```sh
   clang -E -Xclang -dump-tokens main.c
   ```
   运行后会在命令行中显示如下内容：
   ![Alt text](picture/%E8%AF%8D%E6%B3%95%E5%88%86%E6%9E%90.png)
2. 语法分析
   ```sh
   clang -E -Xclang -ast-dump main.c
   ```
   运行后会在命令行中显示如下内容：
   ![Alt text](picture/%E8%AF%AD%E6%B3%95%E5%88%86%E6%9E%90.png)
3. 生成 llvm IR 代码
   ```sh 
   clang -S -emit-llvm main.c
   ```
4. 代码生成


   x86格式代码生成
   ```sh
   gcc main.i -S -o main.S
   ```


   arm格式代码生成
   ```sh
   arm-linux-gnueabihf-gcc main.i -S -o main.S
   ```


   llvm生成代码
   ```sh
   llc main.ll -o main.S
   ```



# 汇编


x86格式汇编
```sh
gcc -c main.s -o main.o
```


arm格式汇编
```sh
arm-linux-gnueabihf-gcc main.S -o main.o
```


llvm格式汇编
```sh
llvm-as main.ll -o main.bc
llc main.bc -filetype=obj -o main.o
```

# 链接
x86链接
```sh
gcc main.o -o main
```

arm链接
```sh
arm-linux-gnueabihf-gcc factorial_arm.S -o factorial_arm -static
```


llvm链接
```sh
clang main.o -o main
```