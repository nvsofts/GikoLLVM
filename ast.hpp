#ifndef __GIKO_AST_HPP
#define __GIKO_AST_HPP

#include <iostream>
#include <string>
#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>

namespace giko
{

namespace ast
{

enum AstID
{
  BaseID,
  FunctionID,
  ModuleID,
  NumberID,
  IdentifierID,
  MonoExprID,
  BinaryExprID,
  BuiltinID,
  AssignID,
  StatementsID,
  IfStatementID,
  WhileStatementID
};

class BaseAST
{
  AstID ID;

 public:
  BaseAST(AstID id) : ID(id)
  {
    // none
  }

  virtual ~BaseAST()
  {
    // none
  }

  AstID getValueID() const
  {
    return this->ID;
  }
};

class FunctionAST : public BaseAST
{
 public:
  std::string Name;
  std::vector<BaseAST *> Inst;

  FunctionAST(const std::string &name) : BaseAST(AstID::FunctionID), Name(name)
  {
    std::cout << "FunctionAST(" << this << ") " << name << std::endl;
  }

  ~FunctionAST()
  {
    for (auto p : this->Inst) {
      delete p;
    }
  }

  static inline bool classof(BaseAST const *base)
  {
    return base->getValueID() == AstID::FunctionID;
  }

  std::string &getName(void)
  {
    return this->Name;
  }

  std::vector<BaseAST *> &getInst(void)
  {
    return this->Inst;
  }
};

class ModuleAST : public BaseAST
{
 public:
  std::vector<std::string> Vars;
  std::vector<FunctionAST *> Funcs;

  ModuleAST() : BaseAST(AstID::ModuleID)
  {
    std::cout << "ModuleAST(" << this << ") " << std::endl;
  }

  ~ModuleAST()
  {
    for (auto p : this->Funcs) {
      delete p;
    }
  }

  static inline bool classof(BaseAST const *base)
  {
    return base->getValueID() == AstID::ModuleID;
  }

  std::vector<std::string> &getVars(void)
  {
    return this->Vars;
  }

  std::vector<FunctionAST *> &getFuncs(void)
  {
    return this->Funcs;
  }
};

class NumberAST : public BaseAST
{
  int Val;

 public:
  NumberAST(int val) : BaseAST(AstID::NumberID), Val(val)
  {
    std::cout << "NumberAST(" << this << ") " << val << std::endl;
  }

  static inline bool classof(BaseAST const *base)
  {
    return base->getValueID() == AstID::NumberID;
  }

  int getVal(void)
  {
    return this->Val;
  }
};

class IdentifierAST : public BaseAST
{
  std::string Identifier;

 public:
  IdentifierAST(const std::string &identifier) : BaseAST(AstID::IdentifierID), Identifier(identifier)
  {
    std::cout << "IdentifierAST(" << this << ") " << identifier << std::endl;
  }

  static inline bool classof(BaseAST const *base)
  {
    return base->getValueID() == AstID::IdentifierID;
  }

  std::string &getIdentifier(void)
  {
    return this->Identifier;
  }
};

class MonoExprAST : public BaseAST
{
  std::string Op;
  BaseAST *Lhs;

 public:
  MonoExprAST(const std::string &op, BaseAST *lhs) : BaseAST(AstID::MonoExprID), Op(op), Lhs(lhs)
  {
    std::cout << "MonoExprAST(" << this << ") " << op << ' ' << lhs << std::endl;
  }

  ~MonoExprAST()
  {
    delete this->Lhs;
  }

  static inline bool classof(BaseAST const *base)
  {
    return base->getValueID() == AstID::MonoExprID;
  }

  std::string &getOp(void)
  {
    return this->Op;
  }

  BaseAST *getLhs(void)
  {
    return this->Lhs;
  }
};

class BinaryExprAST : public BaseAST
{
  std::string Op;
  BaseAST *Lhs;
  BaseAST *Rhs;

 public:
  BinaryExprAST(const std::string &op, BaseAST *lhs, BaseAST *rhs) : BaseAST(AstID::BinaryExprID), Op(op), Lhs(lhs), Rhs(rhs)
  {
    std::cout << "BinaryExprAST(" << this << ") " << lhs << ' ' << op << ' ' << rhs << std::endl;
  }

  ~BinaryExprAST()
  {
    delete this->Lhs;
    delete this->Rhs;
  }

  static inline bool classof(BaseAST const *base)
  {
    return base->getValueID() == AstID::BinaryExprID;
  }

  std::string &getOp(void)
  {
    return this->Op;
  }

  BaseAST *getLhs(void)
  {
    return this->Lhs;
  }

  BaseAST *getRhs(void)
  {
    return this->Rhs;
  }
};

class BuiltinAST : public BaseAST
{
 public:
  std::string Name;
  std::vector<BaseAST *> Args;

  BuiltinAST(const std::string &name) : BaseAST(AstID::BuiltinID), Name(name)
  {
    std::cout << "BuiltinAST(" << this << ") " << name << std::endl;
  }

  ~BuiltinAST()
  {
    for (auto arg : this->Args) {
      delete arg;
    }
  }

  static inline bool classof(BaseAST const *base)
  {
    return base->getValueID() == AstID::BuiltinID;
  }

  std::string &getName(void)
  {
    return this->Name;
  }

  std::vector<BaseAST *> &getArgs(void)
  {
    return this->Args;
  }
};

class AssignAST : public BaseAST
{
 public:
  std::string Name;
  BaseAST *Val;

  AssignAST(const std::string &name) : BaseAST(AstID::AssignID), Name(name)
  {
    std::cout << "AssignAST(" << this << ") " << name << std::endl;
  }

  ~AssignAST()
  {
    delete this->Val;
  }

  static inline bool classof(BaseAST const *base)
  {
    return base->getValueID() == AstID::AssignID;
  }

  std::string &getName(void)
  {
    return this->Name;
  }

  BaseAST *getVal(void)
  {
    return this->Val;
  }
};

class StatementsAST : public BaseAST
{
 public:
  std::vector<BaseAST *> Statements;

  StatementsAST() : BaseAST(AstID::StatementsID)
  {
    std::cout << "StatementsAST(" << this << ")" << std::endl;
  }

  ~StatementsAST()
  {
    for (auto s : this->Statements) {
      delete s;
    }
  }

  static inline bool classof(BaseAST const *base)
  {
    return base->getValueID() == AstID::StatementsID;
  }

  std::vector<BaseAST *> &getStatements(void)
  {
    return this->Statements;
  }
};

class IfStatementAST : public BaseAST
{
 public:
  BaseAST *Cond;
  BaseAST *ThenStatement;
  BaseAST *ElseStatement;

  IfStatementAST() : BaseAST(AstID::IfStatementID), Cond(), ThenStatement(), ElseStatement()
  {
    std::cout << "IfStatementAST(" << this << ")" << std::endl;
  }

  ~IfStatementAST()
  {
    delete this->Cond;
    delete this->ThenStatement;
    delete this->ElseStatement;
  }

  static inline bool classof(BaseAST const *base)
  {
    return base->getValueID() == AstID::IfStatementID;
  }

  BaseAST *getCond(void)
  {
    return this->Cond;
  }

  BaseAST *getThenStatement(void)
  {
    return this->ThenStatement;
  }

  BaseAST *getElseStatement(void)
  {
    return this->ElseStatement;
  }
};

class WhileStatementAST : public BaseAST
{
 public:
  BaseAST *Cond;
  std::vector<BaseAST *> LoopStatement;

  WhileStatementAST() : BaseAST(AstID::WhileStatementID), Cond()
  {
    std::cout << "WhileStatementAST(" << this << ")" << std::endl;
  }

  ~WhileStatementAST()
  {
    delete this->Cond;
    for (auto s : this->LoopStatement) {
      delete s;
    }
  }

  static inline bool classof(BaseAST const *base)
  {
    return base->getValueID() == AstID::WhileStatementID;
  }

  BaseAST *getCond(void)
  {
    return this->Cond;
  }

  std::vector<BaseAST *> &getLoopStatement(void)
  {
    return this->LoopStatement;
  }
};

}

}

BOOST_FUSION_ADAPT_STRUCT(
    giko::ast::ModuleAST,
    (std::vector<std::string>, Vars)
    (std::vector<giko::ast::FunctionAST *>, Funcs))

BOOST_FUSION_ADAPT_STRUCT(
    giko::ast::FunctionAST,
    (std::string, Name)
    (std::vector<giko::ast::BaseAST *>, Inst))

BOOST_FUSION_ADAPT_STRUCT(
    giko::ast::BuiltinAST,
    (std::string, Name)
    (std::vector<giko::ast::BaseAST *>, Args))

BOOST_FUSION_ADAPT_STRUCT(
    giko::ast::AssignAST,
    (std::string, Name)
    (giko::ast::BaseAST *, Val))

BOOST_FUSION_ADAPT_STRUCT(
    giko::ast::IfStatementAST,
    (giko::ast::BaseAST *, Cond)
    (giko::ast::BaseAST *, ThenStatement)
    (giko::ast::BaseAST *, ElseStatement))

BOOST_FUSION_ADAPT_STRUCT(
    giko::ast::WhileStatementAST,
    (giko::ast::BaseAST *, Cond)
    (std::vector<giko::ast::BaseAST *>, LoopStatement))

BOOST_FUSION_ADAPT_STRUCT(
    giko::ast::StatementsAST,
    (std::vector<giko::ast::BaseAST *>, Statements))

#endif
