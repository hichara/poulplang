all: lft-cc

clean:
	rm parser.cpp parser.hpp lft-cc tokens.cpp

parser.cpp: parser.y
	bison -d -o $@ $^

parser.hpp: parser.cpp

tokens.cpp: tokens.l parser.hpp
	lex -o $@ $^

lft-cc: parser.cpp main.cpp tokens.cpp ast.cpp
	clang -o $@ `llvm-config --libs core jit native --cxxflags --ldflags` *.cpp -lstdc++
    
run: lft-cc
	./lft-cc dummy.src
