all: native-compiler

clean:
	@rm -f parser.cpp parser.hpp lft-cc tokens.cpp *.ll *.out *.bc *.s *~ out 2> /dev/null

parser.cpp: parser.y
	bison -d -o $@ $^

parser.hpp: parser.cpp

tokens.cpp: tokens.l parser.hpp
	lex -o $@ $^

lft-cc: parser.cpp main.cpp tokens.cpp ast.cpp
	clang -o $@ *.cpp `llvm-config-3.4 --libs engine core jit native --cxxflags --ldflags` -lstdc++ -lm -ldl -Wno-c++11-extensions
    
llvm-as: run 
	llvm-as-3.4 -f out.ll

llc: llvm-as
	llc-3.4 -o out.s out.bc

native-compiler: llc
	gcc -o out out.s -g

run: lft-cc
	./lft-cc source.poulp
