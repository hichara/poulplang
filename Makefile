all: lft-cc

clean:
	rm parser.cpp parser.hpp lft-cc tokens.cpp *.ll *.out *.bc *.s 2> /dev/null

parser.cpp: parser.y
	bison -d -o $@ $^

parser.hpp: parser.cpp

tokens.cpp: tokens.l parser.hpp
	lex -o $@ $^

lft-cc: parser.cpp main.cpp tokens.cpp ast.cpp
	clang -o $@ *.cpp `llvm-config-2.9 --libs engine core jit native --cxxflags --ldflags` -lstdc++
    
debug: parser.cpp main.cpp tokens.cpp ast.cpp
	clang -o lft-cc *.cpp `llvm-config-2.9 --libs engine core jit native --cxxflags --ldflags` -lstdc++ -g

assemble: run
	llvm-as-2.9 -f out.ll

gen-assembly: assemble
	llc-2.9 -o out.s out.bc

link: gen-assembly
	gcc -o out out.s -g

run: lft-cc
	./lft-cc dummy.src
