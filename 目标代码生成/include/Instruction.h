#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__

#include "Operand.h"
#include "Ast.h"
#include "SymbolTable.h"
#include <optional>
#include <vector>
#include <map>

class BasicBlock;

class Instruction
{
public:
    Instruction() = default;
    Instruction(unsigned instType, BasicBlock *insert_bb = nullptr);
    virtual ~Instruction();
    BasicBlock *getParent();
    bool isUncond() const {return instType == UNCOND;};
    bool isCond() const {return instType == COND;};
    bool isRet() const {return instType == RET;};
    void setParent(BasicBlock *);
    void setNext(Instruction *);
    void setPrev(Instruction *);
    Instruction *getNext();
    Instruction *getPrev();
    unsigned getInstType() {return instType;}
    virtual Operand *getDef() { return nullptr; }
    virtual std::vector<Operand *> getUse() { return {}; }
    virtual void output() const = 0;
protected:
    unsigned instType;
    unsigned opcode;
    Instruction *prev{};
    Instruction *next{};
    BasicBlock *parent;
    std::vector<Operand*> operands;
    enum {BINARY, COND, UNCOND, RET, LOAD, STORE, CMP, ALLOCA, SITOFP,FPTOSI,ZEXT,GEP,GLOBAL,CONSTANT,UNARY,CALL};
};

// meaningless instruction, used as the head node of the instruction list.
class DummyInstruction : public Instruction
{
public:
    DummyInstruction() : Instruction(-1, nullptr) {};
    void output() const {};
};

class AllocaInstruction : public Instruction
{
public:
    AllocaInstruction(Operand *dst, SymbolEntry *se, BasicBlock *insert_bb = nullptr);
    ~AllocaInstruction();
    void output() const;
    Operand *getDef() { return operands[0]; }
private:
    SymbolEntry *se;
};

class ConstantInstruction : public Instruction
{
public:
    ConstantInstruction(Operand *dst, SymbolEntry *se,InitVal* v = nullptr, BasicBlock *insert_bb = nullptr);
    ~ConstantInstruction();
    void output() const;
    Operand *getDef() { return operands[0]; }
private:
    SymbolEntry *se;
    InitVal* val;
};

class GlobalInstruction : public Instruction
{
public:
    GlobalInstruction(Operand *dst, SymbolEntry *se,InitVal* v = nullptr, BasicBlock *insert_bb = nullptr);
    ~GlobalInstruction();
    void output() const;
    Operand *getDef() { return operands[0]; }
private:
    SymbolEntry *se;
    InitVal* val;
};

class GEPInstruction : public Instruction
{
public:
    GEPInstruction(Operand *dst, Operand *src, int index, BasicBlock *insert_bb,bool sp = false);
    GEPInstruction(Operand *dst, Operand *src, Operand *index, BasicBlock *insert_bb,bool sp = false);
    GEPInstruction(Operand *dst, Operand *src, std::vector<int> index, BasicBlock *insert_bb);
    ~GEPInstruction();
    void output() const;
    Operand *getDef() { return operands[0]; }
    std::vector<Operand *> getUse() { if(index.empty())return {operands[1],operands[2]};else return {operands[1]}; }
private:
    std::vector<int> index{};
    bool issp = false;
};

class ZextInstruction : public Instruction
{
public:
    ZextInstruction(Operand *dst, Operand *src, BasicBlock *insert_bb);
    ~ZextInstruction();
    void output() const;
    Operand *getDef() { return operands[0]; }
    std::vector<Operand *> getUse() { return {operands[1]}; }
};

class LoadInstruction : public Instruction
{
public:
    LoadInstruction(Operand *dst, Operand *src_addr, BasicBlock *insert_bb = nullptr);
    ~LoadInstruction();
    void output() const;
    Operand *getDef() { return operands[0]; }
    std::vector<Operand *> getUse() { return {operands[1]}; }
};

class SitofpInstruction : public Instruction
{
public:
    SitofpInstruction(Operand *dst, Operand *src, BasicBlock *insert_bb);
    ~SitofpInstruction();
    void output() const;
    Operand *getDef() { return operands[0]; }
    std::vector<Operand *> getUse() { return {operands[1]}; }
};

class FptosiInstruction : public Instruction
{
public:
    FptosiInstruction(Operand *dst, Operand *src, BasicBlock *insert_bb);
    ~FptosiInstruction();
    void output() const;
    Operand *getDef() { return operands[0]; }
    std::vector<Operand *> getUse() { return {operands[1]}; }
};

class StoreInstruction : public Instruction
{
public:
    StoreInstruction(Operand *dst_addr, Operand *src, BasicBlock *insert_bb = nullptr);
    ~StoreInstruction();
    void output() const;
    std::vector<Operand *> getUse() { return {operands[0], operands[1]}; }
};

class BinaryInstruction : public Instruction
{
public:
    BinaryInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb = nullptr);
    ~BinaryInstruction();
    void output() const;
    enum {SUB, ADD, AND, OR,MUL,DIV,MOD};
    Operand *getDef() { return operands[0]; }
    std::vector<Operand *> getUse() { return {operands[1], operands[2]}; }
};

class CallInstruction : public Instruction
{
public:
    CallInstruction(Operand *d, SymbolEntry* se, std::vector<Operand*> params, BasicBlock *insert_bb);
    ~CallInstruction();
    void output() const;
    Operand *getDef() { return operands[0]; }
    std::vector<Operand *> getUse() { auto vec = operands;vec.erase(vec.begin());return vec; }
private:
    Operand *dst{};
    SymbolEntry *se;
};

class UnaryInstruction : public Instruction
{
public:
    UnaryInstruction(unsigned opcode, Operand *dst, Operand *src, BasicBlock *insert_bb);
    ~UnaryInstruction();
    void output() const;
    enum {SUB, ADD,NOT};
    Operand *getDef() { return operands[0]; }
    std::vector<Operand *> getUse() { return {operands[1]}; }
};

class CmpInstruction : public Instruction
{
public:
    CmpInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb = nullptr);
    ~CmpInstruction();
    void output() const;
    enum { LESS, EQUAL,NEQUAL,NLESS,NGREATER,GREATER };
    Operand *getDef() { return operands[0]; }
    std::vector<Operand *> getUse() { return {operands[1], operands[2]}; }
};

// unconditional branch
class UncondBrInstruction : public Instruction
{
public:
    UncondBrInstruction(BasicBlock*, BasicBlock *insert_bb = nullptr);
    void output() const;
    void setBranch(BasicBlock *);
    BasicBlock *getBranch();
    BasicBlock **patchBranch() {return &branch;};
protected:
    BasicBlock *branch;
};

// conditional branch
class CondBrInstruction : public Instruction
{
public:
    CondBrInstruction(BasicBlock*, BasicBlock*, Operand *, BasicBlock *insert_bb = nullptr);
    CondBrInstruction(BasicBlock *insert_bb,Operand *cond);
    ~CondBrInstruction();
    void output() const;
    void setTrueBranch(BasicBlock*);//!!
    BasicBlock* getTrueBranch();
    void setFalseBranch(BasicBlock*);//!!
    BasicBlock* getFalseBranch();
    BasicBlock **patchBranchTrue() {return &true_branch;};
    BasicBlock **patchBranchFalse() {return &false_branch;};
    std::vector<Operand *> getUse() { return {operands[0]}; }
protected:
    BasicBlock* true_branch = nullptr;
    BasicBlock* false_branch = nullptr;
};

class RetInstruction : public Instruction
{
public:
    RetInstruction(Operand *src, BasicBlock *insert_bb = nullptr);
    ~RetInstruction();
    std::vector<Operand *> getUse()
    {
        if (!operands.empty())
            return {operands[0]};
        else
            return {};
    }
    void output() const;
};

#endif