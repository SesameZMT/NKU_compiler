#include "Instruction.h"
#include "BasicBlock.h"
#include <iostream>
#include <sstream>
#include <memory>
#include <optional>
#include "Function.h"
#include "Type.h"
#include <cassert>
extern FILE* yyout;

Instruction::Instruction(unsigned instType, BasicBlock *insert_bb)
{
    prev = next = this;
    opcode = -1;
    this->instType = instType;
    if (insert_bb != nullptr)
    {
        insert_bb->insertBack(this);
        parent = insert_bb;
    }
}

Instruction::~Instruction()
{
    parent->remove(this);
}

BasicBlock *Instruction::getParent()
{
    return parent;
}

void Instruction::setParent(BasicBlock *bb)
{
    parent = bb;
}

void Instruction::setNext(Instruction *inst)
{
    next = inst;
}

void Instruction::setPrev(Instruction *inst)
{
    prev = inst;
}

Instruction *Instruction::getNext()
{
    return next;
}

Instruction *Instruction::getPrev()
{
    return prev;
}

UnaryInstruction::UnaryInstruction(unsigned opcode, Operand *dst, Operand *src, BasicBlock *insert_bb) : Instruction(UNARY, insert_bb)
{
    this->opcode = opcode;
    operands.push_back(dst);
    operands.push_back(src);
    dst->setDef(this);
    src->addUse(this);
    if(src->getType()==TypeSystem::floatType)
    {
        operands[0]->changeType(TypeSystem::floatType);
    }
}

UnaryInstruction::~UnaryInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
}

void UnaryInstruction::output() const
{
    std::string s1, s2,  type;
    s1 = operands[0]->toStr();
    s2 = operands[1]->toStr();
    type = operands[0]->getType()->toStr();
    switch (opcode)
    {
    case ADD:
        if(type=="float")
            fprintf(yyout, "  %s = fadd %s 0.000000e+00, %s\n", s1.c_str(), type.c_str(), s2.c_str());
        else
            fprintf(yyout, "  %s = add %s 0, %s\n", s1.c_str(), type.c_str(), s2.c_str());
        break;
    case SUB:
        if(type=="float")
            fprintf(yyout, "  %s = fsub %s 0.000000e+00, %s\n", s1.c_str(), type.c_str(), s2.c_str());
        else
            fprintf(yyout, "  %s = sub %s 0, %s\n", s1.c_str(), type.c_str(), s2.c_str());
        break;
    case NOT:
        fprintf(yyout, "  %s = xor %s 1, %s\n", s1.c_str(), type.c_str(), s2.c_str());
        break;
    default:
        break;
    }
}

BinaryInstruction::BinaryInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb)
{
    this->opcode = opcode;
    if(src1->getType()==TypeSystem::intType&&src2->getType()==TypeSystem::floatType)
    {
        auto dst2 = new Operand(new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel()));
        new SitofpInstruction(dst2, src1, insert_bb);
        operands.push_back(dst);
        operands.push_back(dst2);
        operands.push_back(src2);
        dst->setDef(this);
        dst2->addUse(this);
        src2->addUse(this);
        operands[0]->changeType(TypeSystem::floatType);
    }
    else if(src1->getType()==TypeSystem::floatType&&src2->getType()==TypeSystem::intType)
    {
        auto dst2 = new Operand(new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel()));
        new SitofpInstruction(dst2, src2, insert_bb);
        operands.push_back(dst);
        operands.push_back(src1);
        operands.push_back(dst2);
        dst->setDef(this);
        src1->addUse(this);
        dst2->addUse(this);
        operands[0]->changeType(TypeSystem::floatType);
    }
    else if(src1->getType()==TypeSystem::floatType&&src2->getType()==TypeSystem::floatType)
    {
        operands.push_back(dst);
        operands.push_back(src1);
        operands.push_back(src2);
        dst->setDef(this);
        src1->addUse(this);
        src2->addUse(this);
        operands[0]->changeType(TypeSystem::floatType);
    }
    else
    {
        operands.push_back(dst);
        operands.push_back(src1);
        operands.push_back(src2);
        dst->setDef(this);
        src1->addUse(this);
        src2->addUse(this);
    }
    prev = next = this;
    opcode = -1;
    this->instType = Instruction::BINARY;
    if (insert_bb != nullptr)
    {
        insert_bb->insertBack(this);
        parent = insert_bb;
    }
}

BinaryInstruction::~BinaryInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
    operands[2]->removeUse(this);
}

void BinaryInstruction::output() const
{
    std::string s1, s2, s3, op, type;
    s1 = operands[0]->toStr();
    s2 = operands[1]->toStr();
    s3 = operands[2]->toStr();
    type = operands[1]->getType()->toStr();
    switch (opcode)
    {
    case ADD:
        if(type=="float")
            op = "fadd";
        else
            op = "add";
        break;
    case SUB:
        if(type=="float")
            op = "fsub";
        else
            op = "sub";
        break;
    case MUL:
        if(type=="float")
            op = "fmul";
        else
            op = "mul";
        break;
    case DIV:
        if(type=="float")
            op = "fdiv";
        else
            op = "sdiv";
        break;
    case MOD:
        op = "srem";
        break;
    default:
        break;
    }//TOOOOOOOOOOOOOOOODO
    fprintf(yyout, "  %s = %s %s %s, %s\n", s1.c_str(), op.c_str(), type.c_str(), s2.c_str(), s3.c_str());
}

CmpInstruction::CmpInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb){
    this->opcode = opcode;
    if(src1->getType()==TypeSystem::boolType&&src2->getType()==TypeSystem::intType)
    {
        auto dst1 = new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));
        new ZextInstruction(dst1, src1, insert_bb);
        operands.push_back(dst);
        operands.push_back(dst1);
        operands.push_back(src2);
        dst->setDef(this);
        dst1->addUse(this);
        src2->addUse(this);
    }
    else if(src2->getType()==TypeSystem::boolType&&src1->getType()==TypeSystem::intType)
    {
        auto dst2 = new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));
        new ZextInstruction(dst2, src2, insert_bb);
        operands.push_back(dst);
        operands.push_back(src1);
        operands.push_back(dst2);
        dst->setDef(this);
        src1->addUse(this);
        dst2->addUse(this);
    }
    else if(src1->getType()==TypeSystem::intType&&src2->getType()==TypeSystem::floatType)
    {
        auto dst2 = new Operand(new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel()));
        new SitofpInstruction(dst2, src1, insert_bb);
        operands.push_back(dst);
        operands.push_back(dst2);
        operands.push_back(src2);
        dst->setDef(this);
        dst2->addUse(this);
        src2->addUse(this);
    }
    else if(src1->getType()==TypeSystem::floatType&&src2->getType()==TypeSystem::intType)
    {
        auto dst2 = new Operand(new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel()));
        new SitofpInstruction(dst2, src2, insert_bb);
        operands.push_back(dst);
        operands.push_back(src1);
        operands.push_back(dst2);
        dst->setDef(this);
        src1->addUse(this);
        dst2->addUse(this);
    }
    else if(src1->getType()==TypeSystem::boolType&&src2->getType()==TypeSystem::floatType)
    {
        auto dst1 = new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));
        new ZextInstruction(dst1, src1, insert_bb);
        auto dst2 = new Operand(new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel()));
        new SitofpInstruction(dst2, dst1, insert_bb);
        operands.push_back(dst);
        operands.push_back(dst2);
        operands.push_back(src2);
        dst->setDef(this);
        dst2->addUse(this);
        src2->addUse(this);
    }
    else if(src1->getType()==TypeSystem::floatType&&src2->getType()==TypeSystem::boolType)
    {
        auto dst1 = new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));
        new ZextInstruction(dst1, src2, insert_bb);
        auto dst2 = new Operand(new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel()));
        new SitofpInstruction(dst2, dst1, insert_bb);
        operands.push_back(dst);
        operands.push_back(src1);
        operands.push_back(dst2);
        dst->setDef(this);
        src1->addUse(this);
        dst2->addUse(this);
    }
    else
    {
        operands.push_back(dst);
        operands.push_back(src1);
        operands.push_back(src2);
        dst->setDef(this);
        src1->addUse(this);
        src2->addUse(this);
        prev = next = this;
        opcode = -1;
    }
    this->instType = Instruction::CMP;
    if (insert_bb != nullptr)
    {
        insert_bb->insertBack(this);
        parent = insert_bb;
    }
}

CmpInstruction::~CmpInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
    operands[2]->removeUse(this);
}

void CmpInstruction::output() const
{
    std::string s1, s2, s3, op, type;
    s1 = operands[0]->toStr();
    s2 = operands[1]->toStr();
    s3 = operands[2]->toStr();
    type = operands[1]->getType()->toStr();
    switch (opcode)
    {
    case EQUAL:
        if(type=="float")
            op = "ueq";
        else
            op = "eq";
        break;
    case NEQUAL:
        if(type=="float")
            op = "une";
        else
            op = "ne";
        break;
    case LESS:
        if(type=="float")
            op = "ult";
        else
            op = "slt";
        break;
    case NGREATER:
        if(type=="float")
            op = "ule";
        else
            op = "sle";
        break;
    case GREATER:
        if(type=="float")
            op = "ugt";
        else
            op = "sgt";
        break;
    case NLESS:
        if(type=="float")
            op = "uge";
        else
            op = "sge";
        break;
    default:
        op = "";
        break;
    }
    if(type=="float")
        fprintf(yyout, "  %s = fcmp %s %s %s, %s\n", s1.c_str(), op.c_str(), type.c_str(), s2.c_str(), s3.c_str());
    else
        fprintf(yyout, "  %s = icmp %s %s %s, %s\n", s1.c_str(), op.c_str(), type.c_str(), s2.c_str(), s3.c_str());

}

UncondBrInstruction::UncondBrInstruction(BasicBlock *to, BasicBlock *insert_bb) : Instruction(UNCOND, insert_bb)
{
    branch = to;
}

void UncondBrInstruction::output() const
{
    fprintf(yyout, "  br label %%B%d\n", branch->getNo());
}

void UncondBrInstruction::setBranch(BasicBlock *bb)
{
    branch = bb;
}

BasicBlock *UncondBrInstruction::getBranch()
{
    return branch;
}

CondBrInstruction::CondBrInstruction(BasicBlock*true_branch, BasicBlock*false_branch, Operand *cond, BasicBlock *insert_bb) : Instruction(COND, insert_bb){
    this->true_branch = true_branch;
    this->false_branch = false_branch;
    cond->addUse(this);
    operands.push_back(cond);
}

CondBrInstruction::CondBrInstruction(BasicBlock *insert_bb,Operand *cond) : Instruction(COND, insert_bb){
    cond->addUse(this);
    operands.push_back(cond);
}

CondBrInstruction::~CondBrInstruction()
{
    operands[0]->removeUse(this);
}

void CondBrInstruction::output() const
{
    std::string cond, type;
    cond = operands[0]->toStr();
    int true_label = true_branch->getNo();
    int false_label = false_branch->getNo();
    fprintf(yyout, "  br i1 %s, label %%B%d, label %%B%d\n", cond.c_str(), true_label, false_label);
}

void CondBrInstruction::setFalseBranch(BasicBlock *bb)
{
    false_branch = bb;
}

BasicBlock *CondBrInstruction::getFalseBranch()
{
    return false_branch;
}

void CondBrInstruction::setTrueBranch(BasicBlock *bb)
{
    true_branch = bb;
}

BasicBlock *CondBrInstruction::getTrueBranch()
{
    return true_branch;
}

RetInstruction::RetInstruction(Operand *src, BasicBlock *insert_bb) : Instruction(RET, insert_bb)
{
    if(src != nullptr)
    {
        operands.push_back(src);
        src->addUse(this);
    }
}

RetInstruction::~RetInstruction()
{
    if(!operands.empty())
        operands[0]->removeUse(this);
}

void RetInstruction::output() const
{
    if(operands.empty())
    {
        fprintf(yyout, "  ret void\n");
    }
    else
    {
        std::string ret, type;
        ret = operands[0]->toStr();
        type = operands[0]->getType()->toStr();
        fprintf(yyout, "  ret %s %s\n", type.c_str(), ret.c_str());
    }
}

CallInstruction::CallInstruction(Operand *d, SymbolEntry* se, std::vector<Operand*> params, BasicBlock *insert_bb)
{
    if(d!=nullptr)
    {
        d->setDef(this);
        dst = d;
    }
    this->se = se;
    auto p = dynamic_cast<FunctionType*>(se->getType())->getParams();
    assert(params.size()==p.size());
    for(size_t i = 0;i<p.size();++i)
    {
        if(p[i]->isInt()&&params[i]->getType()->isFloat())
        {
            auto dst = new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));
            new FptosiInstruction(dst, params[i], insert_bb);
            params[i] = dst;
        }
        if(p[i]->isFloat()&&params[i]->getType()->isInt())
        {
            auto dst = new Operand(new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel()));
            new SitofpInstruction(dst, params[i], insert_bb);
            params[i] = dst;
        }
        if(p[i]->isFloat()&&params[i]->getType()==TypeSystem::boolType)
        {
            auto dst = new Operand(new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel()));
            new ZextInstruction(dst, params[i], insert_bb);
            auto dst2 = new Operand(new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel()));
            new SitofpInstruction(dst2, dst, insert_bb);
            params[i] = dst2;
        }
        if(p[i]->isInt()&&params[i]->getType()==TypeSystem::boolType)
        {
            auto dst = new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));
            new ZextInstruction(dst, params[i], insert_bb);
            params[i] = dst;
        }
    }
    for(const auto& node : params)
    {
        node->addUse(this);
    }
    operands = params;
    prev = next = this;
    opcode = -1;
    this->instType = Instruction::CALL;
    if (insert_bb != nullptr)
    {
        insert_bb->insertBack(this);
        parent = insert_bb;
    }
}

CallInstruction::~CallInstruction()
{
    if(dst!=nullptr)
    {
        dst->setDef(nullptr);
        if(dst->usersNum() == 0)
            delete dst;
    }
}

void CallInstruction::output() const
{
    if(dst==nullptr)
    {
        fprintf(yyout, "  call %s %s(", dynamic_cast<FunctionType*>(se->getType())->getRetType()->toStr().c_str(), se->toStr().c_str());
    }
    else
    {
        fprintf(yyout, "  %s = call %s %s(",dst->toStr().c_str(), dynamic_cast<FunctionType*>(se->getType())->getRetType()->toStr().c_str(), se->toStr().c_str());
    }
    for(auto iter = operands.begin();iter!=operands.end();++iter)
    {
        fprintf(yyout, "%s %s", (*iter)->getType()->toStr().c_str(), (*iter)->toStr().c_str());
        if(operands.end()-iter!=1)
        {
            fprintf(yyout, ", ");
        }
    }
    fprintf(yyout, ")\n");
    for(size_t i = 0;i<operands.size();++i)
    {
        operands[i]->removeUse(const_cast<CallInstruction*>(this));
    }//???????????????????
}

GlobalInstruction::GlobalInstruction(Operand *dst, SymbolEntry *se,InitVal* v, BasicBlock *insert_bb) : Instruction(GLOBAL, insert_bb)
{
    operands.push_back(dst);
    dst->setDef(this);
    this->se = se;
    val = v;
}
GlobalInstruction::~GlobalInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
}
void GlobalInstruction::output() const
{
    std::string dst, type;
    dst = operands[0]->toStr();
    type = se->getType()->toStr();
    if(se->getType()->isArray())
    {
        if(val==nullptr)
        {
            fprintf(yyout, "%s = global %s zeroinitializer, align 16\n", dst.c_str(), type.c_str());
        }
        else
        {
            bool isint = se->getType()->isInt();
            fprintf(yyout, "%s = global ", dst.c_str());
            bool isreturn = false;
            std::vector<int> initiallist{};
            std::vector<float> finitiallist{};
            std::vector<int> dims = dynamic_cast<ArrayType*>(se->getType())->getDims();
            std::vector<int> status(dims.size());
            std::vector<int> target(dims.size());
            for(size_t i=0;i<target.size();++i)
            {
                target[i] = dims[i]-1;
            }
            std::function<void(InitVal*, int)> initarr = [&](InitVal* init,int level){
                if(init->getExprNode()!=nullptr)
                {   
                    if(isint)
                        initiallist.emplace_back(init->getExprNode()->getInt());
                    else
                        finitiallist.emplace_back(init->getExprNode()->getFloat());
                    if(status==target)
                    {
                        isreturn = true;
                        return;
                    }
                    for(int i = dims.size()-1;i>=0;--i)
                    {
                        if(status[i]==dims[i]-1)
                        {
                            if(i!=0)
                            {
                                status[i] = 0;
                            }
                            else
                            {
                                break;
                            }
                        }
                        else
                        {
                            ++status[i];
                            break;
                        }
                    }

                }
                else
                {
                    auto childs = init->getChilds();
                    for(auto node : childs)
                    {
                        int st = status[level];
                        initarr(node,level+1);
                        if(node->getExprNode()==nullptr)
                        {
                            if(status==target)
                            {
                                isreturn = true;
                                return;
                            }
                            std::vector<int> temp(status);
                            bool isboundary = true;
                            for(size_t i = level+1;i<status.size();++i)
                            {
                                if(status[i] != 0)
                                {
                                    isboundary=false;
                                    break;
                                }
                            }
                            if(!isboundary||temp[level]==st)
                            {
                                status[level]++;
                                for(size_t i = level+1;i<status.size();++i)
                                {
                                    status[i] = 0;
                                }
                                while(temp!=status)
                                {
                                    if(isint)
                                        initiallist.emplace_back(0);
                                    else
                                        finitiallist.emplace_back(0);        
                                    if(temp==target)
                                    {
                                        isreturn = true;
                                        return;
                                    }
                                    for(int i = dims.size()-1;i>=0;--i)
                                    {
                                        if(temp[i]>=dims[i]-1)
                                        {
                                            if(i!=0)
                                            {
                                                temp[i] = 0;
                                            }
                                            else
                                            {
                                                break;
                                            }
                                        }
                                        else
                                        {
                                            ++temp[i];
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            };
            initarr(val,0);
            if(!isreturn||status<target)
            {
                while(status!=target)
                {
                    if(isint)
                        initiallist.emplace_back(0);
                    else
                        finitiallist.emplace_back(0);
                    for(int i = target.size()-1;i>=0;--i)
                    {
                        if(status[i]==target[i])
                        {
                            status[i] = 0;
                        }
                        else
                        {
                            ++status[i];
                            break;
                        }
                    }
                }
                if(isint)
                    initiallist.emplace_back(0);
                else
                    finitiallist.emplace_back(0);
            }
            int index = 0;
            std::function<void(ArrayType,bool)> printarr = [&](ArrayType arrtype,bool iscomma){
                fprintf(yyout,"%s [",arrtype.toStr().c_str());
                if(arrtype.getDims().size()!=1)
                {
                    int size = arrtype.getDims().front();
                    arrtype.eraseFirst();
                    for(int i = 0;i<size;++i)
                    {
                        if(i==size-1)
                        {
                            printarr(arrtype,false);
                        }
                        else
                        {
                            printarr(arrtype,true);
                        }
                    }
                    if(iscomma)
                    {
                        fprintf(yyout,"], ");
                    }
                    else
                    {
                        fprintf(yyout,"]");
                    }
                }
                else
                {
                    for(int i = 0;i<arrtype.getDims().front();++i)
                    {
                        if(isint)
                        {
                            fprintf(yyout,"i32 %d",initiallist[index++]);
                            if(i!=arrtype.getDims().front()-1)
                            {
                                fprintf(yyout,", ");
                            }
                        }
                        else
                        {
                            if(finitiallist[index]==0)
                            {
                                ++index;
                                fprintf(yyout,"float 0.000000e+00");
                            }
                            else
                            {
                                union {
                                    double val;
                                    uint64_t hex;
                                } u;
                                u.val = finitiallist[index++];
                                std::stringstream ss;
                                std::string str;
                                ss << std::hex << std::showbase << u.hex;
                                ss >> str;
                                fprintf(yyout,"float %s",str.c_str());
                            }
                            if(i!=arrtype.getDims().front()-1)
                            {
                                fprintf(yyout,", ");
                            }
                        }
                    }
                    if(iscomma)
                    {
                        fprintf(yyout,"], ");
                    }
                    else
                    {
                        fprintf(yyout,"]");
                    }
                }
            };
            printarr(*(dynamic_cast<ArrayType*>(se->getType())),true);
            fprintf(yyout,"align 16\n");
        }
    }
    else
    {
        if(val==nullptr)
        {
            
            if(type=="float")
            {
                fprintf(yyout, "%s = global %s 0.000000e+00, align 4\n", dst.c_str(), type.c_str());
            }
            else
            {
                fprintf(yyout, "%s = global %s 0, align 4\n", dst.c_str(), type.c_str());
            }
        }
        else
        {
            if(type=="float")
            {
                union {
                    double value;
                    uint64_t hex;
                } u;
                u.value = val->getExprNode()->getFloat();
                std::stringstream ss;
                std::string str;
                ss << std::hex << std::showbase << u.hex;
                ss >> str;
                fprintf(yyout, "%s = global %s %s, align 4\n", dst.c_str(), type.c_str(),str.c_str());
            }
            else
            {
                fprintf(yyout, "%s = global %s %d, align 4\n", dst.c_str(), type.c_str(),val->getExprNode()->getInt());
            }
        }
    }
}

ConstantInstruction::ConstantInstruction(Operand *dst, SymbolEntry *se,InitVal* v, BasicBlock *insert_bb) : Instruction(CONSTANT, insert_bb)
{
    operands.push_back(dst);
    dst->setDef(this);
    this->se = se;
    val = v;
}
ConstantInstruction::~ConstantInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
}
void ConstantInstruction::output() const
{
    std::string dst, type;
    dst = operands[0]->toStr();
    type = se->getType()->toStr();
    if(se->getType()->isArray())
    {
        bool isint = se->getType()->isInt();
        fprintf(yyout, "%s = constant ", dst.c_str());
        std::vector<int> initiallist{};
        std::vector<float> finitiallist{};
        if(isint)
        {
            initiallist = dynamic_cast<IdentifierSymbolEntry*>(se)->getIntArrayop().value();
        }
        else
        {
            finitiallist = dynamic_cast<IdentifierSymbolEntry*>(se)->getFloatArrayop().value();
        }
        int index = 0;
        std::function<void(ArrayType,bool)> printarr = [&](ArrayType arrtype,bool iscomma){
            fprintf(yyout,"%s [",arrtype.toStr().c_str());
            if(arrtype.getDims().size()!=1)
            {
                int size = arrtype.getDims().front();
                arrtype.eraseFirst();
                for(int i = 0;i<size;++i)
                {
                    if(i==size-1)
                    {
                        printarr(arrtype,false);
                    }
                    else
                    {
                        printarr(arrtype,true);
                    }
                }
                if(iscomma)
                {
                    fprintf(yyout,"], ");
                }
                else
                {
                    fprintf(yyout,"]");
                }
            }
            else
            {
                for(int i = 0;i<arrtype.getDims().front();++i)
                {
                    if(isint)
                    {
                        fprintf(yyout,"i32 %d",initiallist[index++]);
                        if(i!=arrtype.getDims().front()-1)
                        {
                            fprintf(yyout,", ");
                        }
                    }
                    else
                    {
                        if(finitiallist[index]==0)
                        {
                            ++index;
                            fprintf(yyout,"float 0.000000e+00");
                        }
                        else
                        {
                            union {
                                double val;
                                uint64_t hex;
                            } u;
                            u.val = finitiallist[index++];
                            std::stringstream ss;
                            std::string str;
                            ss << std::hex << std::showbase << u.hex;
                            ss >> str;
                            fprintf(yyout,"float %s",str.c_str());
                        }
                        if(i!=arrtype.getDims().front()-1)
                        {
                            fprintf(yyout,", ");
                        }
                    }
                }
                if(iscomma)
                {
                    fprintf(yyout,"], ");
                }
                else
                {
                    fprintf(yyout,"]");
                }
            }
        };
        printarr(*(dynamic_cast<ArrayType*>(se->getType())),true);
        fprintf(yyout,"align 16\n");
    }
    else
    {
        if(val==nullptr)
        {
            
            if(type=="float")
            {
                fprintf(yyout, "%s = global %s 0.000000e+00, align 4\n", dst.c_str(), type.c_str());
            }
            else
            {
                fprintf(yyout, "%s = global %s 0, align 4\n", dst.c_str(), type.c_str());
            }
        }
        else
        {
            if(type=="float")
            {
                union {
                    double value;
                    uint64_t hex;
                } u;
                u.value = val->getExprNode()->getFloat();
                std::stringstream ss;
                std::string str;
                ss << std::hex << std::showbase << u.hex;
                ss >> str;
                fprintf(yyout, "%s = global %s %s, align 4\n", dst.c_str(), type.c_str(),str.c_str());
            }
            else
            {
                fprintf(yyout, "%s = global %s %d, align 4\n", dst.c_str(), type.c_str(),val->getExprNode()->getInt());
            }
        }
    }
}

AllocaInstruction::AllocaInstruction(Operand *dst, SymbolEntry *se, BasicBlock *insert_bb) : Instruction(ALLOCA, insert_bb)
{
    operands.push_back(dst);
    dst->setDef(this);
    this->se = se;
}

AllocaInstruction::~AllocaInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
}

void AllocaInstruction::output() const
{
    std::string dst, type;
    dst = operands[0]->toStr();
    type = se->getType()->toStr();
    if(se->getType()->isArray())
    {
        fprintf(yyout, "  %s = alloca %s, align 16\n", dst.c_str(), type.c_str());
    }
    else
    {
        fprintf(yyout, "  %s = alloca %s, align 4\n", dst.c_str(), type.c_str());
    }
}

LoadInstruction::LoadInstruction(Operand *dst, Operand *src_addr, BasicBlock *insert_bb) : Instruction(LOAD, insert_bb)
{
    operands.push_back(dst);
    operands.push_back(src_addr);
    dst->setDef(this);
    src_addr->addUse(this);
}

LoadInstruction::~LoadInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
}

void LoadInstruction::output() const
{
    std::string dst = operands[0]->toStr();
    std::string src = operands[1]->toStr();
    std::string dst_type = operands[0]->getType()->toStr();
    std::string src_type = operands[1]->getType()->toStr();
    fprintf(yyout, "  %s = load %s, %s %s, align 4\n", dst.c_str(), dst_type.c_str(), src_type.c_str(), src.c_str());
}

StoreInstruction::StoreInstruction(Operand *dst_addr, Operand *src, BasicBlock *insert_bb)
{
    if(src->getType()->isInt()&&dst_addr->getType()->isFloat())
    {
        auto dst = new Operand(new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel()));
        new SitofpInstruction(dst, src, insert_bb);
        operands.push_back(dst_addr);
        operands.push_back(dst);
        dst_addr->addUse(this);
        dst->addUse(this);
    }
    else if(src->getType()->isFloat()&&dst_addr->getType()->isInt())
    {
        auto dst = new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));
        new FptosiInstruction(dst, src, insert_bb);
        operands.push_back(dst_addr);
        operands.push_back(dst);
        dst_addr->addUse(this);
        dst->addUse(this);
    }
    else
    {
        operands.push_back(dst_addr);
        operands.push_back(src);
        dst_addr->addUse(this);
        src->addUse(this);
    }
    prev = next = this;
    opcode = -1;
    this->instType = Instruction::STORE;
    if (insert_bb != nullptr)
    {
        insert_bb->insertBack(this);
        parent = insert_bb;
    }
}

StoreInstruction::~StoreInstruction()
{
    operands[0]->removeUse(this);
    operands[1]->removeUse(this);
}

void StoreInstruction::output() const
{
    std::string dst = operands[0]->toStr();
    std::string src = operands[1]->toStr();
    std::string dst_type = operands[0]->getType()->toStr();
    std::string src_type = operands[1]->getType()->toStr();

    fprintf(yyout, "  store %s %s, %s %s, align 4\n", src_type.c_str(), src.c_str(), dst_type.c_str(), dst.c_str());
    
}

GEPInstruction::GEPInstruction(Operand *dst, Operand *src, int i, BasicBlock *insert_bb,bool sp) : Instruction(GEP, insert_bb)
{
    operands.push_back(dst);
    operands.push_back(src);
    dst->setDef(this);
    src->addUse(this);
    index.emplace_back(i);
    issp = sp;
}

GEPInstruction::GEPInstruction(Operand *dst, Operand *src, Operand *i, BasicBlock *insert_bb,bool sp) : Instruction(GEP, insert_bb)
{
    operands.push_back(dst);
    operands.push_back(src);
    operands.push_back(i);
    dst->setDef(this);
    src->addUse(this);
    i->addUse(this);
    issp = sp;
}

GEPInstruction::GEPInstruction(Operand *dst, Operand *src, std::vector<int> i, BasicBlock *insert_bb): Instruction(GEP, insert_bb)
{
    operands.push_back(dst);
    operands.push_back(src);
    dst->setDef(this);
    src->addUse(this);
    index = i;
}

GEPInstruction::~GEPInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
    if(operands.size()==3)
    {
        operands[2]->removeUse(this);
    }
}

void GEPInstruction::output() const
{
    std::string dst = operands[0]->toStr();
    std::string src = operands[1]->toStr();
    if(issp)
    {
        if(index.empty())
        {
            fprintf(yyout,"  %s = getelementptr inbounds %s, %s %s, i32 %s\n"
            ,dst.c_str(),dynamic_cast<PointerType*>(operands[1]->getType())->getValue()->toStr().c_str(),operands[1]->getType()->toStr().c_str(),src.c_str(),operands[2]->toStr().c_str());
        }
        else
        {
            fprintf(yyout,"  %s = getelementptr inbounds %s, %s %s, i32 %d\n"
            ,dst.c_str(),dynamic_cast<PointerType*>(operands[1]->getType())->getValue()->toStr().c_str(),operands[1]->getType()->toStr().c_str(),src.c_str(),index[0]);
        }
    }
    else
    {
        if(index.empty())
        {
            fprintf(yyout,"  %s = getelementptr inbounds %s, %s %s, i32 0, i32 %s\n"
            ,dst.c_str(),dynamic_cast<PointerType*>(operands[1]->getType())->getValue()->toStr().c_str(),operands[1]->getType()->toStr().c_str(),src.c_str(),operands[2]->toStr().c_str());
        }
        else
        {
            fprintf(yyout,"  %s = getelementptr inbounds %s, %s %s, i32 0"
            ,dst.c_str(),dynamic_cast<PointerType*>(operands[1]->getType())->getValue()->toStr().c_str(),operands[1]->getType()->toStr().c_str(),src.c_str());
            for(auto iter = index.begin();iter!=index.end();++iter)
            {
                fprintf(yyout,", i32 %d",*iter);
            }
            fprintf(yyout,"\n");
        }
    }
    
}

SitofpInstruction::SitofpInstruction(Operand *dst, Operand *src, BasicBlock *insert_bb) : Instruction(SITOFP, insert_bb)
{
    operands.push_back(dst);
    operands.push_back(src);
    dst->setDef(this);
    src->addUse(this);
}

SitofpInstruction::~SitofpInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
}

void SitofpInstruction::output() const
{
    std::string dst = operands[0]->toStr();
    std::string src = operands[1]->toStr();
    fprintf(yyout, "  %s = sitofp i32 %s to float\n", dst.c_str(), src.c_str());
}

ZextInstruction::ZextInstruction(Operand *dst, Operand *src, BasicBlock *insert_bb) : Instruction(ZEXT, insert_bb)
{
    operands.push_back(dst);
    operands.push_back(src);
    dst->setDef(this);
    src->addUse(this);
}

ZextInstruction::~ZextInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
}

void ZextInstruction::output() const
{
    std::string dst = operands[0]->toStr();
    std::string src = operands[1]->toStr();
    fprintf(yyout, "  %s = zext %s %s to %s\n", dst.c_str(),operands[1]->getType()->toStr().c_str() ,src.c_str(),operands[0]->getType()->toStr().c_str());
}

FptosiInstruction::FptosiInstruction(Operand *dst, Operand *src, BasicBlock *insert_bb) : Instruction(FPTOSI, insert_bb)
{
    operands.push_back(dst);
    operands.push_back(src);
    dst->setDef(this);
    src->addUse(this);
}

FptosiInstruction::~FptosiInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
}

void FptosiInstruction::output() const
{
    std::string dst = operands[0]->toStr();
    std::string src = operands[1]->toStr();
    fprintf(yyout, "  %s = fptosi float %s to i32\n", dst.c_str(), src.c_str());
}