#ifndef __GIKO_GENERATOR_HPP
#define __GIKO_GENERATOR_HPP

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include "ast.hpp"

namespace giko
{

namespace generator
{

using namespace giko::ast;
using namespace llvm;

class generator
{
  IRBuilder<> *builder;
  Module *module;

  BasicBlock *while_block_loopcond;
  BasicBlock *while_block_afterloop;

 public:
  generator() : builder(new IRBuilder<>(getGlobalContext())),
                module(new Module("output", getGlobalContext())),
                while_block_loopcond(), while_block_afterloop()
  {
    // none
  }

  ~generator()
  {
    delete this->builder;
    delete this->module;
  }

  // 定数
  Constant *generateNumber(int num)
  {
    return ConstantInt::getSigned(Type::getInt32Ty(getGlobalContext()), num);
  }

  // 識別子(loadする)
  Value *generateIdentifier(const std::string &id)
  {
    return this->builder->CreateLoad(this->module->getGlobalVariable(id), "");
  }

  Value *generateIdentifier(IdentifierAST *id)
  {
    return this->generateIdentifier(id->getIdentifier());
  }

  // 識別子(loadしない)
  Value *generateIdentifier2(IdentifierAST *id)
  {
    return this->module->getGlobalVariable(id->getIdentifier());
  }

  // 一項演算子
  Value *generateMonoExpr(MonoExprAST *mono_expr)
  {
    BaseAST *lhs = mono_expr->getLhs();

    Value *v_lhs;

    // オペランドの命令を生成
    if (isa<BinaryExprAST>(lhs)) {
      v_lhs = this->generateBinaryExpr(dyn_cast<BinaryExprAST>(lhs));
    }else if (isa<NumberAST>(lhs)) {
      v_lhs = this->generateNumber(dyn_cast<NumberAST>(lhs)->getVal());
    }else if (isa<IdentifierAST>(lhs)) {
      v_lhs = this->generateIdentifier(dyn_cast<IdentifierAST>(lhs));
    }else if (isa<MonoExprAST>(lhs)) {
      v_lhs = this->generateMonoExpr(dyn_cast<MonoExprAST>(lhs));
    }

    // 演算子に応じた命令を生成
    std::string &op = mono_expr->getOp();
    if (op == "!") {
      return this->builder->CreateNot(v_lhs, "not");
    }

    return nullptr;
  }

  // 二項演算子
  Value *generateBinaryExpr(BinaryExprAST *bin_expr)
  {
    BaseAST *lhs = bin_expr->getLhs();
    BaseAST *rhs = bin_expr->getRhs();

    Value *v_lhs;
    Value *v_rhs;

    // 左辺の命令を生成
    if (isa<BinaryExprAST>(lhs)) {
      v_lhs = this->generateBinaryExpr(dyn_cast<BinaryExprAST>(lhs));
    }else if (isa<NumberAST>(lhs)) {
      v_lhs = this->generateNumber(dyn_cast<NumberAST>(lhs)->getVal());
    }else if (isa<IdentifierAST>(lhs)) {
      v_lhs = this->generateIdentifier(dyn_cast<IdentifierAST>(lhs));
    }else if (isa<MonoExprAST>(lhs)) {
      v_lhs = this->generateMonoExpr(dyn_cast<MonoExprAST>(lhs));
    }

    // 右辺の命令を生成
    if (isa<BinaryExprAST>(rhs)) {
      v_rhs = this->generateBinaryExpr(dyn_cast<BinaryExprAST>(rhs));
    }else if (isa<NumberAST>(rhs)) {
      v_rhs = this->generateNumber(dyn_cast<NumberAST>(rhs)->getVal());
    }else if (isa<IdentifierAST>(rhs)) {
      v_rhs = this->generateIdentifier(dyn_cast<IdentifierAST>(rhs));
    }else if (isa<MonoExprAST>(rhs)) {
      v_rhs = this->generateMonoExpr(dyn_cast<MonoExprAST>(rhs));
    }

    // 演算子に応じた命令を生成
    std::string &op = bin_expr->getOp();
    if (op == "+") {
      return this->builder->CreateAdd(v_lhs, v_rhs, "add");
    }else if (op == "-") {
      return this->builder->CreateSub(v_lhs, v_rhs, "sub");
    }else if (op == "*") {
      return this->builder->CreateMul(v_lhs, v_rhs, "mul");
    }else if (op == "/") {
      return this->builder->CreateSDiv(v_lhs, v_rhs, "div");
    }else if (op == "%") {
      return this->builder->CreateSRem(v_lhs, v_rhs, "rem");
    }else if (op == "=") {
      return this->builder->CreateICmpEQ(v_lhs, v_rhs, "eq");
    }else if (op == "<") {
      return this->builder->CreateICmpSLT(v_lhs, v_rhs, "lt");
    }else if (op == ">") {
      return this->builder->CreateICmpSGT(v_lhs, v_rhs, "gt");
    }else if (op == "<=") {
      return this->builder->CreateICmpSLE(v_lhs, v_rhs, "leq");
    }else if (op == ">=") {
      return this->builder->CreateICmpSGE(v_lhs, v_rhs, "geq");
    }else if (op == "&&") {
      return this->builder->CreateAnd(v_lhs, v_rhs, "and");
    }else if (op == "||") {
      return this->builder->CreateOr(v_lhs, v_rhs, "or");
    }

    return nullptr;
  }

  // 代入
  Value *generateAssign(AssignAST *inst)
  {
    Value *var = this->module->getGlobalVariable(inst->getName());
    Value *val = this->generateInst(inst->getVal());

    return this->builder->CreateStore(val, var);
  }

  // 組み込み命令
  Value *generateBuiltin(BuiltinAST *inst)
  {
    std::string &name = inst->getName();

    if (name == "return") {
      return this->builder->CreateRet(nullptr);
    }else if (name == "exit") {
      std::vector<Type *> args;

      args.push_back(Type::getInt32Ty(getGlobalContext()));

      FunctionType *func_type = FunctionType::get(Type::getVoidTy(getGlobalContext()), args, false);
      Function *F = dyn_cast<Function>(this->module->getOrInsertFunction("exit", func_type));
      F->addFnAttr(Attribute::NoReturn);

      return this->builder->CreateCall(F, this->generateNumber(0));
    }else if (name == "print") {
      std::vector<Type *> args;

      args.push_back(Type::getInt32Ty(getGlobalContext()));

      FunctionType *func_type = FunctionType::get(Type::getVoidTy(getGlobalContext()), args, false);
      Function *F = dyn_cast<Function>(this->module->getOrInsertFunction("print", func_type));

      return this->builder->CreateCall(F, this->generateInst(inst->getArgs()[0]));
    }else if (name == "scan") {
      std::vector<Type *> args;
      FunctionType *func_type = FunctionType::get(Type::getInt32Ty(getGlobalContext()), args, false);
      Function *F = dyn_cast<Function>(this->module->getOrInsertFunction("scan", func_type));

      return this->builder->CreateStore(this->builder->CreateCall(F),
                                        this->generateIdentifier2(dyn_cast<IdentifierAST>(inst->getArgs()[0])));
    }else if (name == "rand") {
      std::vector<Type *> args;
      FunctionType *func_type = FunctionType::get(Type::getInt32Ty(getGlobalContext()), args, false);
      Function *F = dyn_cast<Function>(this->module->getOrInsertFunction("rand", func_type));

      return this->builder->CreateStore(this->builder->CreateCall(F),
                                        this->generateIdentifier2(dyn_cast<IdentifierAST>(inst->getArgs()[0])));
    }else if (name == "call") {
      Function *F = dyn_cast<Function>(this->module->getFunction(dyn_cast<IdentifierAST>(inst->getArgs()[0])->getIdentifier()));

      return this->builder->CreateCall(F);
    }else if (name == "continue") {
      if (this->while_block_loopcond) {
        return this->builder->CreateBr(this->while_block_loopcond);
      }

      return nullptr;
    }else if (name == "break") {
      if (this->while_block_afterloop) {
        return this->builder->CreateBr(this->while_block_afterloop);
      }

      return nullptr;
    }

    return nullptr;
  }

  // 文の集合
  void generateStatements(StatementsAST *inst)
  {
    for (auto s : inst->getStatements()) {
      this->generateInst(s);
    }
  }

  // if文
  void generateIfStatement(IfStatementAST *inst)
  {
    Value *cond = this->generateInst(inst->getCond());
    Function *func = this->builder->GetInsertBlock()->getParent();
    bool falseAvail = (inst->getElseStatement() != nullptr);

    BasicBlock *ThenBB = BasicBlock::Create(getGlobalContext(), "then", func);
    BasicBlock *ElseBB = BasicBlock::Create(getGlobalContext(), "else");
    BasicBlock *MergeBB = BasicBlock::Create(getGlobalContext(), "cont");

    // 分岐命令を生成
    this->builder->CreateCondBr(cond, ThenBB, (falseAvail) ? ElseBB : MergeBB);

    // Then節の処理
    this->builder->SetInsertPoint(ThenBB);
    this->generateInst(inst->getThenStatement());

    if (!ThenBB->getInstList().empty() && !isa<TerminatorInst>(ThenBB->back())) {
      this->builder->CreateBr(MergeBB);
    }

    // Else節の処理(ある場合)
    if (falseAvail) {
      func->getBasicBlockList().push_back(ElseBB);
      this->builder->SetInsertPoint(ElseBB);
      this->generateInst(inst->getElseStatement());
      this->builder->CreateBr(MergeBB);

      if (!ElseBB->getInstList().empty() && !isa<TerminatorInst>(ElseBB->back())) {
        this->builder->CreateBr(MergeBB);
      }
    }

    // 終端部の処理
    func->getBasicBlockList().push_back(MergeBB);
    this->builder->SetInsertPoint(MergeBB);
  }

  // while文
  void generateWhileStatement(WhileStatementAST *inst)
  {
    Function *func = this->builder->GetInsertBlock()->getParent();

    BasicBlock *LoopCondBB = BasicBlock::Create(getGlobalContext(), "loopcond", func);
    BasicBlock *LoopBB = BasicBlock::Create(getGlobalContext(), "loop");
    BasicBlock *AfterLoopBB = BasicBlock::Create(getGlobalContext(), "afterloop");

    // ループ条件判定へジャンプ
    this->builder->CreateBr(LoopCondBB);

    // 分岐命令を生成
    this->builder->SetInsertPoint(LoopCondBB);
    Value *cond = this->generateInst(inst->getCond());
    this->builder->CreateCondBr(cond, LoopBB, AfterLoopBB);

    // ループ内の処理
    func->getBasicBlockList().push_back(LoopBB);
    this->builder->SetInsertPoint(LoopBB);
    this->while_block_loopcond = LoopCondBB;
    this->while_block_afterloop = AfterLoopBB;
    for (auto s : inst->getLoopStatement()) {
      this->generateInst(s);
    }
    this->while_block_loopcond = nullptr;
    this->while_block_afterloop = nullptr;
    this->builder->CreateBr(LoopCondBB);

    // 終端部の処理
    func->getBasicBlockList().push_back(AfterLoopBB);
    this->builder->SetInsertPoint(AfterLoopBB);
  }

  // 命令
  Value *generateInst(BaseAST *inst)
  {
    // 種類別の生成関数を実行
    if (isa<AssignAST>(inst)) {
      return this->generateAssign(dyn_cast<AssignAST>(inst));
    }else if (isa<BinaryExprAST>(inst)) {
      return this->generateBinaryExpr(dyn_cast<BinaryExprAST>(inst));
    }else if (isa<BuiltinAST>(inst)) {
      return this->generateBuiltin(dyn_cast<BuiltinAST>(inst));
    }else if (isa<IdentifierAST>(inst)) {
      return this->generateIdentifier(dyn_cast<IdentifierAST>(inst));
    }else if (isa<IfStatementAST>(inst)) {
      this->generateIfStatement(dyn_cast<IfStatementAST>(inst));
    }else if (isa<NumberAST>(inst)) {
      return this->generateNumber(dyn_cast<NumberAST>(inst)->getVal());
    }else if (isa<MonoExprAST>(inst)) {
      return this->generateMonoExpr(dyn_cast<MonoExprAST>(inst));
    }else if (isa<WhileStatementAST>(inst)) {
      this->generateWhileStatement(dyn_cast<WhileStatementAST>(inst));
    }else if (isa<StatementsAST>(inst)) {
      this->generateStatements(dyn_cast<StatementsAST>(inst));
    }

    return nullptr;
  }

  // モジュール
  Module *generateModule(ModuleAST *mod)
  {
    Type *int_type = Type::getInt32Ty(getGlobalContext());
    FunctionType *func_type = FunctionType::get(Type::getVoidTy(getGlobalContext()), std::vector<Type *>(), false);

    // グローバル変数を作成
    for (const auto &var : mod->getVars()) {
      auto V = new GlobalVariable(*this->module, int_type, false, GlobalVariable::LinkageTypes::CommonLinkage, nullptr, var);

      V->setAlignment(4);
      V->setInitializer(this->generateNumber(0));
    }

    // 関数を作成
    for (auto func : mod->getFuncs()) {
      auto F = Function::Create(func_type, GlobalVariable::LinkageTypes::ExternalLinkage, func->getName(), this->module);
      auto B = BasicBlock::Create(getGlobalContext(), "entry", F);

      this->builder->SetInsertPoint(B);

      for (auto inst : func->getInst()) {
        this->generateInst(inst);
      }
    }

    this->module->dump();
    return this->module;
  }
};

}

}

#endif
