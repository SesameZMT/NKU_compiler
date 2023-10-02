.PHONY:test,clean
lex:
	lex Lexical_Analyzer.l
	gcc lex.yy.c -o Lexical_Analyzer -ll
	./Lexical_Analyzer
clean:
	rm -fr lex.yy.c Lexical_Analyzer