.PHONY:test,clean
lex:analysis.l analysis.y
	bison -d analysis.y
	flex analysis.l
	gcc -o test ast.c analysis.tab.c lex.yy.c main.c

clean:
	rm -fr lex.yy.c analysis.tab.* test