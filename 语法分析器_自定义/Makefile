.PHONY:test,clean
lex:
	bison -d source_code/analysis.y -o source_code/analysis.tab.c
	flex -o source_code/lex.yy.c source_code/analysis.l 
	gcc -o test source_code/ast.c source_code/analysis.tab.c source_code/lex.yy.c source_code/main.c

clean:
	rm -fr source_code/lex.yy.c source_code/analysis.tab.* test