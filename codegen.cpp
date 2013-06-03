#include "ast.h"
#include "codegen.h"
#include "parser.hpp"
#include <iostream>
#include <typeinfo>
#include <llvm/Support/raw_ostream.h>

using namespace std;

/* Compile the AST into a module */
std::string CodeGenContext::generateCode(StatementBlock& root)
{
    std::cout << "Generating code...\n";

    /* Create the top level interpreter function to call as entry */
    vector<const Type*> argTypes;
    FunctionType *ftype = FunctionType::get(Type::getInt32Ty(getGlobalContext()), argTypes, false);
    mainFunction = Function::Create(ftype, GlobalValue::ExternalLinkage, "main", module);
    mainFunction->setCallingConv(llvm::CallingConv::C);
    BasicBlock *bblock = BasicBlock::Create(getGlobalContext(), "entry", mainFunction, 0);

    /* Push a new variable/block context */
    pushBlock(bblock);

    /* Create the putchar function */
    vector<const Type*> putcharArgTypes;
    putcharArgTypes.push_back(Type::getInt32Ty(getGlobalContext()));
    FunctionType *putcharFunctionType = FunctionType::get(Type::getInt32Ty(getGlobalContext()), putcharArgTypes, false);
    Function* putcharFunction = Function::Create(putcharFunctionType, GlobalValue::ExternalLinkage, "putchar", module);
    putcharFunction->setCallingConv(llvm::CallingConv::C);

    root.codeGen(*this); /* emit bytecode for the toplevel block */

    ReturnInst::Create(getGlobalContext(), ConstantInt::get(Type::getInt32Ty(getGlobalContext()), 0), bblock);

    popBlock();

    /* Print the bytecode in a human-readable format
       to see if our program compiled properly
     */
    std::cout << "Code is generated.\n";

    std::string outputString;
    raw_ostream* outputStream = new raw_string_ostream(outputString);
    PassManager pm;
    pm.add(createPrintModulePass(outputStream));
    pm.run(*module);

    return outputString;
}

/* Executes the AST by running the main function */
GenericValue CodeGenContext::runCode() {
    std::cout << "Running code...\n";
    std::string error;
    llvm::InitializeNativeTarget();
    ExecutionEngine *ee = ExecutionEngine::create(module, false, &error);
    vector<GenericValue> noargs;
    if( ee == 0 ){
        cout << "Error: " << error << endl;
        return GenericValue();
    } else {
        GenericValue v = ee->runFunction(mainFunction, noargs);
        std::cout << "Code was run.\n";
        return v;
    }
}

/* Returns an LLVM type based on the identifier */
static const Type *typeOf(const Identifier& type)
{
    if (type.name.compare("int") == 0) {
        return Type::getInt32Ty(getGlobalContext());
    }
    else if (type.name.compare("double") == 0) {
        return Type::getDoubleTy(getGlobalContext());
    }
    return Type::getVoidTy(getGlobalContext());
}

/* -- Code Generation -- */

Value* Integer::codeGen(CodeGenContext& context)
{
    std::cout << "Creating integer: " << value << std::endl;
    return ConstantInt::get(Type::getInt32Ty(getGlobalContext()), value, true);
}

Value* Double::codeGen(CodeGenContext& context)
{
    std::cout << "Creating double: " << value << std::endl;
    return ConstantFP::get(Type::getDoubleTy(getGlobalContext()), value);
}

Value* Identifier::codeGen(CodeGenContext& context)
{
    std::cout << "Creating identifier reference: " << name << std::endl;
    if (context.locals().find(name) == context.locals().end()) {
        std::cerr << "undeclared variable " << name << std::endl;
        return NULL;
    }
    return new LoadInst(context.locals()[name], "", false, context.currentBlock());
}

Value* MethodCall::codeGen(CodeGenContext& context)
{
    Function *function = context.module->getFunction(methodName.name.c_str());
    if (function == NULL) {
        std::cerr << "no such function " << methodName.name << std::endl;
    }
    std::vector<Value*> args;
    ExpressionList::const_iterator it;
    for (it = arguments.begin(); it != arguments.end(); it++) {
        args.push_back((**it).codeGen(context));
    }
    CallInst *call = CallInst::Create(function, args.begin(), args.end(), "", context.currentBlock());
    std::cout << "Creating method call: " << methodName.name << std::endl;
    return call;
}

Value* BinaryOperation::codeGen(CodeGenContext& context)
{
    std::cout << "Creating binary operation " << op << std::endl;
    Instruction::BinaryOps instr;
    switch (op) {
    case T_PLUS: 	instr = Instruction::Add; goto math;
    case T_MINUS: 	instr = Instruction::Sub; goto math;
    case T_MUL: 		instr = Instruction::Mul; goto math;
    case T_DIV: 		instr = Instruction::SDiv; goto math;

        /* TODO comparison */
    }

    return NULL;
math:
    return BinaryOperator::Create(instr, lhs.codeGen(context),
                                  rhs.codeGen(context), "", context.currentBlock());
}

Value* Assignment::codeGen(CodeGenContext& context)
{
    std::cout << "Creating assignment for " << lhs.name << std::endl;
    if (context.locals().find(lhs.name) == context.locals().end()) {
        std::cerr << "undeclared variable " << lhs.name << std::endl;
        return NULL;
    }
    return new StoreInst(rhs->codeGen(context), context.locals()[lhs.name], false, context.currentBlock());
}

Value* StatementBlock::codeGen(CodeGenContext& context)
{
    StatementList::const_iterator it;
    Value *last = NULL;
    for (it = statements.begin(); it != statements.end(); it++) {
        std::cout << "Generating code for " << typeid(**it).name() << std::endl;
        last = (**it).codeGen(context);
    }
    std::cout << "Creating block" << std::endl;
    return last;
}

Value* ExpressionStatement::codeGen(CodeGenContext& context)
{
    std::cout << "Generating code for " << typeid(expression).name() << std::endl;
    return expression.codeGen(context);
}

Value* VariableDeclaration::codeGen(CodeGenContext& context)
{
    std::cout << "Creating variable declaration " << type.name << " " << name.name << std::endl;
    AllocaInst *alloc = new AllocaInst(typeOf(type), name.name.c_str(), context.currentBlock());
    context.locals()[name.name] = alloc;
    if (assignmentExpression != NULL) {
        Assignment assn(name, assignmentExpression);
        assn.codeGen(context);
    }
    return alloc;
}

Value* FunctionDeclaration::codeGen(CodeGenContext& context)
{
    vector<const Type*> argTypes;
    VariableList::const_iterator it;
    for (it = arguments.begin(); it != arguments.end(); it++) {
        argTypes.push_back(typeOf((**it).type));
    }
    FunctionType *ftype = FunctionType::get(typeOf(functionType), argTypes, false);
    Function *function = Function::Create(ftype, GlobalValue::ExternalLinkage, functionName.name.c_str(), context.module);
    function->setCallingConv(llvm::CallingConv::C);

    int i = 0;
    for( Function::arg_iterator it = function->arg_begin(); it != function->arg_end(); ++it ){
        it->setName( arguments.at(i++)->name.name );
    }

    BasicBlock *bblock = BasicBlock::Create(getGlobalContext(), "entry", function, 0);
    context.pushBlock(bblock);

    for (it = arguments.begin(); it != arguments.end(); it++) {
        (**it).codeGen(context);
    }

    block.codeGen(context);
    ReturnInst::Create(getGlobalContext(), bblock);

    context.popBlock();
    std::cout << "Creating function: " << functionName.name << std::endl;
    return function;
}
