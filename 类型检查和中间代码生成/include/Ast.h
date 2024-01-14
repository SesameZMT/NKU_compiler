#ifndef __AST_H__
#define __AST_H__

#include <fstream>
#include <optional>

#include "Operand.h"

class SymbolEntry;
class Unit;
class Function;
class BasicBlock;
class Instruction;
class IRBuilder;

class Node {
 private:
  static int counter;
  int seq;

 protected:
  std::vector<BasicBlock *> true_list;
  std::vector<BasicBlock *> false_list;
  static IRBuilder *builder;
  void backPatch(std::vector<BasicBlock *> &list, BasicBlock *target,bool istrue);
  std::vector<BasicBlock *> merge(std::vector<BasicBlock *> &list1,
                                   std::vector<BasicBlock *> &list2);

 public:
  Node();
  int getSeq() const { return seq; };
  static void setIRBuilder(IRBuilder *ib) { builder = ib; };
  virtual void output(int level) = 0;
  virtual void typeCheck() = 0;
  virtual void genCode() = 0;
  std::vector<BasicBlock *> &trueList() { return true_list; }
  std::vector<BasicBlock *> &falseList() { return false_list; }
};

class ExprNode : public Node {
 protected:
  SymbolEntry *symbolEntry;
  Operand *dst;  // The result of the subtree is stored into dst.
  std::optional<float> fval = std::nullopt;
  std::optional<int> ival = std::nullopt;
 public:
  ExprNode(SymbolEntry *symbolEntry) : symbolEntry(symbolEntry){};
  void changeFloat(std::optional<float> f){fval = f;}
  void changeInt(std::optional<int> i){ival = i;}
  bool hasInt(){return ival!=std::nullopt;}
  bool hasFloat(){return fval!=std::nullopt;}
  bool hasValue(){return fval!=std::nullopt||ival!=std::nullopt;}
  std::optional<int> getIntop(){return ival;}
  std::optional<float> getFloatop(){return fval;}
  float getFloat(){if(ival!=std::nullopt)return static_cast<float>(ival.value());else return fval.value();}
  int getInt(){if(fval!=std::nullopt)return static_cast<int>(fval.value());else return ival.value();}
  Operand *getOperand() { return dst; };
  SymbolEntry *getSymPtr() { return symbolEntry; };
};

class BinaryExpr : public ExprNode {
 private:
  int op;
  ExprNode *expr1, *expr2;

 public:
  enum { ADD,MUL,DIV,MOD, SUB, AND, OR, LESS, EQUAL,NEQUAL,NLESS,NGREATER,GREATER };
  BinaryExpr(SymbolEntry *se, int op, ExprNode *expr1, ExprNode *expr2)
      : ExprNode(se), op(op), expr1(expr1), expr2(expr2) {
    dst = new Operand(se);
  };
  void output(int level);
  void typeCheck();
  void genCode();
};

class Constant : public ExprNode {
 public:
  Constant(SymbolEntry *se) : ExprNode(se) { dst = new Operand(se); };
  void output(int level);
  void typeCheck();
  void genCode();
};

class StmtNode : public Node {};

class CompoundStmt : public StmtNode {
 private:
  StmtNode *stmt;

 public:
  CompoundStmt(StmtNode *stmt) : stmt(stmt){};
  void output(int level);
  void typeCheck();
  void genCode();
};

class PlainStmt : public StmtNode
{
private:
    ExprNode *exp= nullptr;
public:
    PlainStmt(){};
    PlainStmt(ExprNode *exp) : exp(exp){};
    void output(int level);
    void typeCheck();
    void genCode();
};

class SeqNode : public StmtNode {
 private:
  std::vector<StmtNode *>stmts;

 public:
  SeqNode(){};
  SeqNode(StmtNode *stmt){stmts.emplace_back(stmt);};
  inline void emplace_back(StmtNode *stmt) {stmts.emplace_back(stmt);}
  void output(int level);
  void typeCheck();
  void genCode();
};


class TDeclStmt : public StmtNode //TODO
{
private:
    StmtNode* st = nullptr;
public:
    TDeclStmt() {}
    TDeclStmt(StmtNode* s) :st(s) {}
    void output(int level);
    void typeCheck();
    void genCode();
};
class FArrayNode : public StmtNode
{
private:
    ExprNode *dim= nullptr;
public:
    FArrayNode(ExprNode *d) : dim(d){};
    ExprNode * getExprNode(){return dim;}
    void output(int level);
    void typeCheck();
    void genCode();
};

class FArrayList : public StmtNode
{
private:
    std::vector<FArrayNode *> dims{};
public:
    FArrayList(){};
    FArrayList(FArrayNode *i){dims.push_back(i);};
    std::vector<FArrayNode *> getVector(){return dims;}
    void push_back(FArrayNode *i){dims.push_back(i);};
    bool empty(){return dims.empty();}
    int size(){return dims.size();}
    void output(int level);
    void typeCheck();
    void genCode();
};

class Id : public ExprNode
{
private:
  FArrayList *arr = nullptr;
public:
  Id(SymbolEntry *se) : ExprNode(se) {
  SymbolEntry *temp =
      new TemporarySymbolEntry(se->getType(), SymbolTable::getLabel());
  dst = new Operand(temp);
  };
  Id(SymbolEntry *se,FArrayList *a) : ExprNode(se),arr(a){
  SymbolEntry *temp =
      new TemporarySymbolEntry(se->getType(), SymbolTable::getLabel());
  dst = new Operand(temp);
  };
  void output(int level);
  void typeCheck();
  void genCode();
  Operand * genStoreCode();
};

class ArrayNode : public StmtNode
{
private:
    ExprNode *dim= nullptr;
public:
    ArrayNode(ExprNode *d) : dim(d){};
    ExprNode * getExprNode(){return dim;}
    void output(int level);
    void typeCheck();
    void genCode(){};
};

class ArrayList : public StmtNode
{
private:
    std::vector<ArrayNode *> dims{};
public:
    ArrayList(){};
    ArrayList(ArrayNode *i){dims.push_back(i);};
    std::vector<ArrayNode *> getVector(){return dims;}
    void push_back(ArrayNode *i){dims.push_back(i);};
    void output(int level);
    void typeCheck();
    void genCode();
};

class InitVal : public StmtNode
{
private:
    ExprNode * expr= nullptr;
    std::vector<InitVal*> ids{};
public:
    InitVal() {}
    InitVal(ExprNode * e) : expr(e){}
    InitVal(std::vector<InitVal*> e) : ids(e){}
    void push_back(InitVal *id) {ids.push_back(id);}
    void output(int level);
    void typeCheck();
    ExprNode * getExprNode() {return expr;}
    std::vector<InitVal*> getChilds() {return ids;}
    void genCode();
};

class InitVals : public StmtNode
{
public:
    std::vector<InitVal*> vals{};
    InitVals(){}
    void output(int level){};
    void typeCheck();
    void genCode();
};

class CallParams : public StmtNode
{
private:
    std::vector<ExprNode*> params{};
public:
    CallParams(ExprNode *e) {params.push_back(e);}
    void push_back(ExprNode *e) {params.push_back(e);}
    std::vector<ExprNode*> getVector() {return params;}
    void output(int level);
    void typeCheck();
    void genCode();
};

class UnaryExpr : public ExprNode
{
private:
    int op;
    ExprNode *expr= nullptr;
    CallParams* params = nullptr;
    SymbolEntry *funcse = nullptr;
public:
    enum {ADD, SUB, NOT, FUNCCALL, NONE};
    UnaryExpr(SymbolEntry *se, int op, ExprNode*expr) : ExprNode(se), op(op), expr(expr){
    dst = new Operand(se);}
    UnaryExpr(SymbolEntry *se,SymbolEntry *func, CallParams* f) : ExprNode(se), op(FUNCCALL), params(f),funcse(func){
    dst = new Operand(se);}
    void output(int level);
    void typeCheck();
    void genCode();
};

class VarId : public ExprNode
{
private:
    ArrayList* arrlist= nullptr;
    InitVal* initval=nullptr;
    bool isconstant = false;
    bool isinited = false;
public:
    VarId(SymbolEntry *se,ArrayList* a) : ExprNode(se),arrlist(a){};
    VarId(SymbolEntry *se,ArrayList* a,InitVal* init) : ExprNode(se),arrlist(a),initval(init){};
    void changeinit(bool res) { isinited = res;}
    bool isinit() { return isinited;}
    void changeconst(bool res) { isconstant = res;}
    bool isconst() { return isconstant;}
    void output(int level);
    ArrayList* getArrayList() {return arrlist;}
    void typeCheck();
    void genCode();
};

class VarIds : public StmtNode
{
private:
    std::vector<VarId *> ids{};
public:
    VarIds(VarId *i){ids.push_back(i);};
    std::vector<VarId *> getVector(){return ids;}
    void push_back(VarId *i){ids.push_back(i);};
    void output(int level){};
    void typeCheck();
    void genCode();
};

class DeclStmt : public StmtNode
{
private:
    std::vector<VarId*> ids{};
public:
    DeclStmt() {}
    DeclStmt(VarIds* v) :ids(v->getVector()) {}
    DeclStmt(std::vector<VarId*> id): ids(id){}
    void push_back(VarId *id) {ids.push_back(id);}
    VarId* get_back() {return ids.back();}
    void output(int level);
    void typeCheck();
    void genCode();
};

class IfStmt : public StmtNode {
 private:
  ExprNode *cond;
  StmtNode *thenStmt;

 public:
  IfStmt(ExprNode *cond, StmtNode *thenStmt) : cond(cond), thenStmt(thenStmt){};
  void output(int level);
  void typeCheck();
  void genCode();
};

class IfElseStmt : public StmtNode {
 private:
  ExprNode *cond;
  StmtNode *thenStmt;
  StmtNode *elseStmt;

 public:
  IfElseStmt(ExprNode *cond, StmtNode *thenStmt, StmtNode *elseStmt)
      : cond(cond), thenStmt(thenStmt), elseStmt(elseStmt){};
  void output(int level);
  void typeCheck();
  void genCode();
};

class WhileStmt : public StmtNode
{
private:
    ExprNode *cond= nullptr;
    StmtNode *thenStmt= nullptr;
public:
    WhileStmt(ExprNode *cond, StmtNode *thenStmt) : cond(cond), thenStmt(thenStmt){};
    void output(int level);
    void typeCheck();
    void genCode();
};

class ContinueStmt : public StmtNode
{
public:
    ContinueStmt(){};
    void output(int level);
    void typeCheck();
    void genCode();
};

class BreakStmt : public StmtNode
{
public:
    BreakStmt(){};
    void output(int level);
    void typeCheck();
    void genCode();
};

class ReturnStmt : public StmtNode {
 private:
  ExprNode *retValue;

 public:
  ReturnStmt(ExprNode *retValue) : retValue(retValue){};
  void output(int level);
  void typeCheck();
  void genCode();
};

class AssignStmt : public StmtNode {
 private:
  ExprNode *lval;
  ExprNode *expr;

 public:
  AssignStmt(ExprNode *lval, ExprNode *expr) : lval(lval), expr(expr){};
  void output(int level);
  void typeCheck();
  void genCode();
};

class FuncParam : public ExprNode
{
private:
    FArrayList* arr= nullptr;
    bool isarr = false;
public:
    FuncParam(SymbolEntry *s):ExprNode(s){};
    FuncParam(SymbolEntry *s,FArrayList* a) : ExprNode(s),arr(a){};
    FArrayList* getArr() {return arr;}
    Type* getType(){return symbolEntry->getType();}
    void changearray(bool a) {isarr = a;}
    bool isarray() {return isarr;}
    void output(int level);
    void typeCheck();
    void genCode();
};

class FuncParams : public StmtNode
{
private:
    std::vector<FuncParam*> params{};
public:
    FuncParams() {}
    FuncParams(FuncParam * e){params.push_back(e);}
    void push_back(FuncParam *id) {params.push_back(id);}
    std::vector<FuncParam*> getVector() {return params;}
    void output(int level);
    void typeCheck();
    void genCode();
};
class FunctionDef : public StmtNode {
 private:
  SymbolEntry *se;
  StmtNode *stmt;
  FuncParams * fparams = nullptr;

 public:
  FunctionDef(SymbolEntry *se, StmtNode *stmt) : se(se), stmt(stmt){};
  FunctionDef(SymbolEntry *se, StmtNode *stmt,FuncParams * f) : se(se), stmt(stmt) , fparams(f){};
  void output(int level);
  void typeCheck();
  void genCode();
};

class Ast {
 private:
  Node *root;

 public:
  Ast() { root = nullptr; }
  void setRoot(Node *n) { root = n; }
  void output();
  void typeCheck();
  void genCode(Unit *unit);
};

#endif
