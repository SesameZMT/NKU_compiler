本次实验主要借助FLEX工具构建了一个简单的词法分析器


# 文件分布
```
词法分析器
├── Makefile
├── Readme.md
├── source_code
│   └── Lexical_Analyzer.l
└── test_code
    ├── compare.sy
    └── test.sy
```
我们在 `source_code` 文件夹下存放了词法分析器的lex程序源码，在 `test_code` 文件夹下存放了用于验证词法分析器的程序。



# 执行测试
在主目录下执行 `make lex` 即可执行词法分析器，之后在命令行中输入需要进行词法分析的程序路径即可执行分析。程序路径形式如下：
```sh
test_code/test.sy
```



# 文件分析
## Makefile
在 `Makefile` 中我们定义了两个操作，即 `make lex` 和 `make clean`。`make lex` 作用为编译并执行词法分析器， `make clean` 作用为清除生成的词法分析器后续代码，保护仓库的整洁。

## source_code/Lexical_Analyzer.l
这是词法分析器的lex源码，在其中使用正则表达式定义了如下可供分析的单词：
* 整数：包括十进制整数、八进制整数、十六进制整数。
* 标识符：以非数字开头的非保留字。
* 特殊字符：包括(、,、)、;、{、}和运算符。
* 保留字：包括if、while、for、break等。

除此以外，通过定义一些特殊函数实现了输出行数和列数的功能。