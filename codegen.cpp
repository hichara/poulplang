#include "ast.h"
#include "codegen.h"
#include "log.h"
#include "parser.hpp"
#include <iostream>
#include <typeinfo>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/IRBuilder.h>

using namespace std;

int PrintfMethodCall::instanceCount = 0;
int BranchStatement::instanceCount = 0;

static llvm::Function* getPutcharPrototype(llvm::LLVMContext& ctx, llvm::Module *mod)
{
    vector<Type*> argTypes;
    //argTypes.push_back(Type::getInt32Ty(ctx));
    FunctionType *ftype = FunctionType::get(Type::getVoidTy(getGlobalContext()), makeArrayRef(argTypes), false);
    Function* func = Function::Create(ftype, GlobalValue::ExternalLinkage, "putchar", mod);
    func->setCallingConv(llvm::CallingConv::C);
    
    return func;
}


static llvm::Function* getPrintfPrototype(llvm::LLVMContext& ctx, llvm::Module *mod)
{
    /*
    std::vector<Type*> argTypes;
    argTypes.push_back(Type::getInt8PtrTy(ctx));
    FunctionType* ftype = FunctionType::get(Type::getVoidTy(getGlobalContext()), makeArrayRef(argTypes), false);
    Function *func = Function::Create( ftype, Function::ExternalLinkage, Twine("printf"), mod );
    func->setCallingConv(llvm::CallingConv::C);

    return func;
    /*/
    std::vector<llvm::Type*> argTypes;
    argTypes.push_back(llvm::Type::getInt8PtrTy(getGlobalContext())); //char*

    llvm::FunctionType* printf_type =
        llvm::FunctionType::get(
            llvm::Type::getInt32Ty(getGlobalContext()), argTypes, true);

    llvm::Function *func = llvm::Function::Create(
                printf_type, llvm::Function::ExternalLinkage,
                llvm::Twine("printf"),
                mod
           );
    func->setCallingConv(llvm::CallingConv::C);
    return func;
    //*/
}

/* Compile the AST into a module */
std::string CodeGenContext::generateCode(StatementBlock& root)
{
    Log::Debug() << "Generating code...\n";

    /* Create the top level interpreter function to call as entry */
    vector<Type*> argTypes;
    FunctionType *ftype = FunctionType::get(Type::getVoidTy(getGlobalContext()), makeArrayRef(argTypes), false);
    mainFunction = Function::Create(ftype, GlobalValue::ExternalLinkage, "main", module);
    mainFunction->setCallingConv(llvm::CallingConv::C);
    BasicBlock *bblock = BasicBlock::Create(getGlobalContext(), "entry", mainFunction, 0);

    /* Push a new variable/block context */
    pushBlock(bblock);

    /* Create the printf function declaration */
    printfFunction = getPrintfPrototype( getGlobalContext(), module );

    /* Create the putchar function declaration */
    getPutcharPrototype( getGlobalContext(), module );
    
    root.codeGen(*this); /* emit bytecode for the toplevel block */

    //ReturnInst::Create(getGlobalContext(), ConstantInt::get(Type::getInt32Ty(getGlobalContext()), 0), bblock);
    ReturnInst::Create(getGlobalContext(), bblock);
    popBlock();

    /* Print the bytecode in a human-readable format
       to see if our program compiled properly
     */
    Log::Debug() << "Code is generated.\n";

    std::string outputString;
    raw_ostream* outputStream = new raw_string_ostream(outputString);
    PassManager pm;
    pm.add(createPrintModulePass(outputStream));
    pm.run(*module);

    return outputString;
}

/* Executes the AST by running the main function */
GenericValue CodeGenContext::runCode() {
    Log::Debug() << "Running code...\n";
    std::string error;
    llvm::InitializeNativeTarget();
    ExecutionEngine *ee = ExecutionEngine::create(module, false, &error);
    vector<GenericValue> noargs;
    if( ee == 0 ){
        Log::Error() << "Error: " << error << endl;
        return GenericValue();
    } else {
        GenericValue v = ee->runFunction(mainFunction, noargs);
        Log::Debug() << "Code was run.\n";
        return v;
    }
}

/* Returns an LLVM type based on the identifier */
static Type *typeOf(const Identifier& type)
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
    Log::Debug() << "Creating integer: " << value << std::endl;
    return ConstantInt::get(Type::getInt32Ty(getGlobalContext()), value, true);
}

Value* Double::codeGen(CodeGenContext& context)
{
    Log::Debug() << "Creating double: " << value << std::endl;
    return ConstantFP::get(Type::getDoubleTy(getGlobalContext()), value);
}

Value* Identifier::codeGen(CodeGenContext& context)
{
    if( context.functionArguments.find(name) != context.functionArguments.end() )
    {
        return context.functionArguments[name];
    }

    Log::Debug() << "Creating identifier reference: " << name << std::endl;
    if (context.locals().find(name) == context.locals().end()) {
        Log::Error() << "undeclared variable " << name << std::endl;
        return NULL;
    }
    Log::Debug() << context.locals()[name] << "\n";
    return new LoadInst(context.locals()[name], "", false, context.currentBlock());
}

Value* MethodCall::codeGen(CodeGenContext& context)
{
    /*
    Function *function = context.module->getFunction(methodName.name.c_str());
    if (function == NULL) {
        Log::Error() << " no such function " << methodName.name << std::endl;
        exit( -1 );
        return NULL;
    }
    std::vector<Value*> args;
    ExpressionList::const_iterator it;
    for (it = arguments.begin(); it != arguments.end(); it++) {
        args.push_back((**it).codeGen(context));
    }
    CallInst *call = CallInst::Create(function, args.begin(), args.end(), "", context.currentBlock());
    Log::Debug() << "Creating method call: " << methodName.name << std::endl;
    return call;
    /*/

    Function *function = context.module->getFunction(methodName.name.c_str());
    if (function == NULL) {
        std::cerr << "no such function " << methodName.name << endl;
    }
    std::vector<Value*> args;
    ExpressionList::const_iterator it;
    for (it = arguments.begin(); it != arguments.end(); it++) {
        args.push_back((**it).codeGen(context));
    }
    CallInst *call = CallInst::Create(function, makeArrayRef(args), "", context.currentBlock());
    std::cout << "Creating method call: " << methodName.name << endl;
    return call;
    //*/
}

Value* PrintfMethodCall::codeGen(CodeGenContext& context)
{
    String name = getUniqueName();
    String fmt = format.substr(1, format.size()-2);
    
    /* replace \n */
    int i = 0;
    while ( i < fmt.size()-1 ){
        if( fmt[i] == '\\' && fmt[i+1] == 'n' ){
            fmt.replace(i , i+1, "\13\15");
        }
        ++i;
    }
    
    /* format string */
    //Constant *format_const = ConstantArray::get(getGlobalContext(), fmt.c_str());
    Constant *format_const = ConstantDataArray::getString(getGlobalContext(), fmt.c_str());
    GlobalVariable *var = new GlobalVariable(
        *context.module, ArrayType::get(IntegerType::get(getGlobalContext(), 8), fmt.size()+1),
        true, GlobalValue::PrivateLinkage, format_const, name.c_str());

    /* helper zero constant */
    Constant *zero = Constant::getNullValue( IntegerType::getInt32Ty(getGlobalContext()));

    Constant* indices[2];
    indices[0] = zero;
    indices[1] = zero;
    
    Constant *var_ref = ConstantExpr::getGetElementPtr(var, indices, 2);

    std::vector<Value*> args;
    args.push_back(var_ref);
    
    /* add the rest of the arguments */
    ExpressionList::const_iterator it;
    for (it = arguments.begin(); it != arguments.end(); it++) {
        args.push_back((**it).codeGen(context));
    }

    CallInst *call = CallInst::Create(context.printfFunction, makeArrayRef(args), "", context.currentBlock());
  //CallInst *call = CallInst::Create(function, args.begin(), args.end(), "", context.currentBlock());

  //CallInst *call = CallInst::Create(context.printfFunction, args.begin(), args.end(), "", context.currentBlock());
    call->setTailCall(false);
    
    return call;
}

Value* BinaryOperation::codeGen(CodeGenContext& context)
{
    Log::Debug() << "Creating binary operation " << op << std::endl;
    Instruction::BinaryOps instr;
    switch (op) {
   
    // Arithmetic Operations
    case T_PLUS:    return BinaryOperator::Create( Instruction::Add,
            lhs.codeGen(context), rhs.codeGen(context), "", context.currentBlock());
    case T_MINUS:   return BinaryOperator::Create( Instruction::Sub,
            lhs.codeGen(context), rhs.codeGen(context), "", context.currentBlock());
    case T_MUL:     return BinaryOperator::Create( Instruction::Mul,
            lhs.codeGen(context), rhs.codeGen(context), "", context.currentBlock());
    case T_DIV:     return BinaryOperator::Create( Instruction::SDiv,
            lhs.codeGen(context), rhs.codeGen(context), "", context.currentBlock());
    
    // Logical Operations
    case T_CMP_EQ:  return  CmpInst::Create( Instruction::ICmp, CmpInst::ICMP_EQ,
            lhs.codeGen(context), rhs.codeGen(context), "", context.currentBlock());
    case T_CMP_NE:  return  CmpInst::Create( Instruction::ICmp, CmpInst::ICMP_NE,
            lhs.codeGen(context), rhs.codeGen(context), "", context.currentBlock());
    case T_CMP_LT:  return  CmpInst::Create( Instruction::ICmp, CmpInst::ICMP_SLT,
            lhs.codeGen(context), rhs.codeGen(context), "", context.currentBlock());
    case T_CMP_GT:  return  CmpInst::Create( Instruction::ICmp, CmpInst::ICMP_SGT,
            lhs.codeGen(context), rhs.codeGen(context), "", context.currentBlock());
    case T_CMP_LE:  return  CmpInst::Create( Instruction::ICmp, CmpInst::ICMP_SLE,
            lhs.codeGen(context), rhs.codeGen(context), "", context.currentBlock());
    case T_CMP_GE:  return  CmpInst::Create( Instruction::ICmp, CmpInst::ICMP_SGE,
            lhs.codeGen(context), rhs.codeGen(context), "", context.currentBlock());
    }

    return NULL;
}

Value* Assignment::codeGen(CodeGenContext& context)
{
    Log::Debug() << "Creating assignment for " << lhs.name << std::endl;
    if (context.locals().find(lhs.name) == context.locals().end()) {
        std::cerr << "undeclared variable " << lhs.name << std::endl;
        exit( -1 );
        return NULL;
    }
    return new StoreInst(rhs->codeGen(context), context.locals()[lhs.name], false, context.currentBlock());
}

Value* StatementBlock::codeGen(CodeGenContext& context)
{
    StatementList::const_iterator it;
    Value *last = NULL;
    for (it = statements.begin(); it != statements.end(); it++) {
    Log::Debug() << "Generating code for " << typeid(**it).name() << std::endl;
        last = (**it).codeGen(context);
    }
    Log::Debug() << "Creating block" << std::endl;
    return last;
}

Value* ExpressionStatement::codeGen(CodeGenContext& context)
{
    Log::Debug() << "Generating code for " << typeid(expression).name() << std::endl;
    return expression.codeGen(context);
}

Value* VariableDeclaration::codeGen(CodeGenContext& context)
{
    /*
    Log::Debug() << "Creating variable declaration " << type.name << " " << name.name << std::endl;
    AllocaInst *alloc = new AllocaInst(typeOf(type), name.name.c_str(), context.currentBlock());
    context.locals()[name.name] = alloc;
    if (assignmentExpression != NULL) {
        Assignment assn(name, assignmentExpression);
        assn.codeGen(context);
    }
    return alloc;
    /*/
    std::cout << "Creating variable declaration " << type.name << " " << name.name << endl;
    AllocaInst *alloc = new AllocaInst(typeOf(type), name.name.c_str(), context.currentBlock());
    context.locals()[name.name] = alloc;
    if (assignmentExpression != NULL) {
        Assignment assn(name, assignmentExpression);
        assn.codeGen(context);
    }

    return alloc;
    //*/
}

Value* FunctionDeclaration::codeGen(CodeGenContext& context)
{
    /*
    vector<const Type*> argTypes;
    VariableList::const_iterator it;
    for (it = arguments.begin(); it != arguments.end(); it++) {
        argTypes.push_back(typeOf((**it).type));
    }
    FunctionType *ftype = FunctionType::get(typeOf(functionType), argTypes, false);
    Function *function = Function::Create(ftype, GlobalValue::ExternalLinkage, functionName.name.c_str(), context.module);
    function->setCallingConv(llvm::CallingConv::C);

    context.currentFunction = function;
    context.functionArguments.clear();
    int i = 0;
    for( auto it = function->arg_begin(); it != function->arg_end(); ++it ){
        it->setName( arguments.at(i)->name.name );
        context.functionArguments[arguments.at(i++)->name.name] = it;
    }

    BasicBlock *bblock = BasicBlock::Create(getGlobalContext(), "entry", function);
    context.pushBlock(bblock);

    block.codeGen(context);
    //ReturnInst::Create(getGlobalContext(), bblock);

    context.popBlock();
    Log::Debug() << "Creating function: " << functionName.name << std::endl;
    context.currentFunction = context.mainFunction;
    return function;
    /*/
    vector<Type*> argTypes;
    VariableList::const_iterator it;
    for (it = arguments.begin(); it != arguments.end(); it++) {
        argTypes.push_back(typeOf((**it).type));
    }
    FunctionType *ftype = FunctionType::get(typeOf(functionType), makeArrayRef(argTypes), false);
    Function *function = Function::Create(ftype, GlobalValue::ExternalLinkage, functionName.name.c_str(), context.module);
    BasicBlock *bblock = BasicBlock::Create(getGlobalContext(), "entry", function, 0);

    context.pushBlock(bblock);

    Function::arg_iterator argsValues = function->arg_begin();
    Value* argumentValue;

    for (it = arguments.begin(); it != arguments.end(); it++) {
        (**it).codeGen(context);
        
        argumentValue = argsValues++;
        argumentValue->setName((*it)->name.name.c_str());
        StoreInst *inst = new StoreInst(argumentValue, context.locals()[(*it)->name.name], false, bblock);
    }
    
    block.codeGen(context);
    //ReturnInst::Create(getGlobalContext(), context.getCurrentReturnValue(), bblock);

    context.popBlock();
    std::cout << "Creating function: " << functionName.name << endl;
    return function;
    //*/
}

Value* ReturnStatement::codeGen(CodeGenContext& context)
{
    Log::Debug() << "Generating code for " << typeid(this).name() << std::endl;
    return ReturnInst::Create(getGlobalContext(), value->codeGen(context), context.currentBlock());
}

Value* BranchStatement::codeGen(CodeGenContext& context)
{
    std::cout << "Generating code for " << typeid(this).name() << std::endl;
    IRBuilder<> builder(context.currentBlock());
    Value* test = testExpression->codeGen( context );
    Function *TheFunction = builder.GetInsertBlock()->getParent();

    BasicBlock *btrue = BasicBlock::Create(getGlobalContext(), getUniqueName(), TheFunction);
        BasicBlock *bfalse = NULL;
    if( hasFalseBranch ){
        bfalse = BasicBlock::Create(getGlobalContext(), getUniqueName(), TheFunction);
    }

    builder.CreateCondBr(test, btrue, bfalse);

    context.pushBlock(btrue);
    blockTrue.codeGen(context);
    context.popBlock();
 
    if( hasFalseBranch ){   
        context.pushBlock(bfalse);
        blockFalse.codeGen(context);
        context.popBlock();
    }
    std::cout << "Generated. " << std::endl;
    return NULL;
}
