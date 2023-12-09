#include "Ast.h"
#include <stack>
#include <string>
#include "IRBuilder.h"
#include "Instruction.h"
#include "SymbolTable.h"
#include "Type.h"
#include "Unit.h"
extern Unit unit;

#include <iostream>

extern FILE* yyout;
int Node::counter = 0;
IRBuilder* Node::builder;

Node::Node() {
    seq = counter++;
    next = nullptr;
}

void Node::setNext(Node* node) {
    Node* n = this;
    while (n->getNext()) {
        n = n->getNext();
    }
    if (n == this) {
        this->next = node;
    } else {
        n->setNext(node);
    }
}

void Node::backPatch(std::vector<Instruction*>& list, BasicBlock* bb) {
    for (auto& inst : list) {
        if (inst->isCond())
            dynamic_cast<CondBrInstruction*>(inst)->setTrueBranch(bb);
        else if (inst->isUncond())
            dynamic_cast<UncondBrInstruction*>(inst)->setBranch(bb);
    }
}

std::vector<Instruction*> Node::merge(std::vector<Instruction*>& list1,
                                      std::vector<Instruction*>& list2) {
    std::vector<Instruction*> res(list1);
    res.insert(res.end(), list2.begin(), list2.end());
    return res;
}

BinaryExpr::BinaryExpr(SymbolEntry* se,
                       int op,
                       ExprNode* expr1,
                       ExprNode* expr2)
    : ExprNode(se), op(op), expr1(expr1), expr2(expr2) {
    dst = new Operand(se);
    std::string op_str;
    switch (op) {
        case ADD:
            op_str = "+";
            break;
        case SUB:
            op_str = "-";
            break;
        case MUL:
            op_str = "*";
            break;
        case DIV:
            op_str = "/";
            break;
        case MOD:
            op_str = "%";
            break;
        case AND:
            op_str = "&&";
            break;
        case OR:
            op_str = "||";
            break;
        case LESS:
            op_str = "<";
            break;
        case LESSEQUAL:
            op_str = "<=";
            break;
        case MORE:
            op_str = ">";
            break;
        case MOREEQUAL:
            op_str = ">=";
            break;
        case EQUAL:
            op_str = "==";
            break;
        case NOTEQUAL:
            op_str = "!=";
            break;
    }
    if (expr1->getType()->isVoid() || expr2->getType()->isVoid()) {
        fprintf(stderr,
                "invalid operand of type \'void\' to binary \'opeartor%s\'\n",
                op_str.c_str());
    }
    if (op >= BinaryExpr::AND && op <= BinaryExpr::NOTEQUAL) {
        //确认是针对逻辑符号的运算
        type = TypeSystem::boolType;
        if (op == BinaryExpr::AND || op == BinaryExpr::OR) {
            if (expr1->getType()->isInt() &&
                expr1->getType()->getSize() == 32) {
                ImplictCastExpr* temp = new ImplictCastExpr(expr1);
                this->expr1 = temp;
            }
            if (expr2->getType()->isInt() &&
                expr2->getType()->getSize() == 32) {
                ImplictCastExpr* temp = new ImplictCastExpr(expr2);
                this->expr2 = temp;
            }
        }
    } else
        type = TypeSystem::intType;
};


void Ast::genCode(Unit* unit) {
    IRBuilder* builder = new IRBuilder(unit);
    Node::setIRBuilder(builder);
    root->genCode();
}

void FunctionDef::genCode() {
    Unit* unit = builder->getUnit();
    Function* func = new Function(unit, se);
    //我们生成代码都必须注意，我们在宏观上是在为函数生成
    BasicBlock* entry = func->getEntry();
    // set the insert point to the entry basicblock of this function.
    // 直接取得入口块的地址

    builder->setInsertBB(entry);
    if (decl) decl->genCode();
    // function中的stmt节点是用compoundstmt进行初始化的
    if (stmt) stmt->genCode();

    /*
     * Construct control flow graph. You need do set successors and predecessors
     * for each basic block. Todo
     */
    for (auto block = func->begin(); block != func->end(); block++) {
        //函数由基本块组成，获取该块的最后一条指令
        Instruction* i = (*block)->begin();
        Instruction* last = (*block)->rbegin();

        while (i != last) {
            if (i->isCond() || i->isUncond()) {
                (*block)->remove(i);
            }
            i = i->getNext();
        }
        

        if (last->isCond()) {
            BasicBlock *truebranch, *falsebranch;
            truebranch = dynamic_cast<CondBrInstruction*>(last)->getTrueBranch();
            falsebranch = dynamic_cast<CondBrInstruction*>(last)->getFalseBranch();
            if (truebranch->empty()) {
                new RetInstruction(nullptr, truebranch);

            } else if (falsebranch->empty()) {
                new RetInstruction(nullptr, falsebranch);
                
            }
            (*block)->addSucc(truebranch);
            (*block)->addSucc(falsebranch);
            truebranch->addPred(*block);
            falsebranch->addPred(*block);
            //设置继承关系
        } else if (last->isUncond()) {
            //无条件跳转指令可获取跳转的目标块
            BasicBlock* next_flow = dynamic_cast<UncondBrInstruction*>(last)->getBranch();

            (*block)->addSucc(next_flow);
            next_flow->addPred(*block);
            if (next_flow->empty()) {
                if (((FunctionType*)(se->getType()))->getRetType() == TypeSystem::intType) 
                    new RetInstruction(new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0)), next_flow);
                else if (((FunctionType*)(se->getType()))->getRetType() == TypeSystem::voidType) 
                    new RetInstruction(nullptr, next_flow);
            }

        }
        //最后一条语句不是返回以及跳转
        else if (!last->isRet()) {
            if (((FunctionType*)(se->getType()))->getRetType() == TypeSystem::voidType) {
                new RetInstruction(nullptr, *block);
            }
        }
    }
}



void BinaryExpr::genCode() {
    BasicBlock* bb = builder->getInsertBB();
    Function* func = bb->getParent();
    if (op == AND) {
        BasicBlock* trueBB = new BasicBlock(func);  
        // if the result of lhs is true, jump to the trueBB.
        expr1->genCode();
        backPatch(expr1->trueList(), trueBB);
        builder->setInsertBB(trueBB);  
        // set the insert point to the trueBB so that intructions
        // generated by expr2 will be inserted into it.
        expr2->genCode();
        true_list = expr2->trueList();
        false_list = merge(expr1->falseList(), expr2->falseList());
    } else if (op == OR) {
        // Todo
        BasicBlock* trueBB = new BasicBlock(func);
        expr1->genCode();
        backPatch(expr1->falseList(), trueBB);
        builder->setInsertBB(trueBB);
        expr2->genCode();
        true_list = merge(expr1->trueList(), expr2->trueList());
        false_list = expr2->falseList();
        
    } else if (op >= LESS && op <= NOTEQUAL) {
        // Todo
        expr1->genCode();
        expr2->genCode();
        Operand* src1 = expr1->getOperand();
        Operand* src2 = expr2->getOperand();
        if (src1->getType()->getSize() == 1) {
            Operand* dst = new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));
            new ZextInstruction(dst, src1, bb);
            src1 = dst;
        }
        if (src2->getType()->getSize() == 1) {
            Operand* dst = new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));
            new ZextInstruction(dst, src2, bb);
            src2 = dst;
        }
        int op_kind;
        switch (op) {
            case LESS:
                op_kind = CmpInstruction::L;
                break;
            case LESSEQUAL:
                op_kind = CmpInstruction::LE;
                break;
            case MORE:
                op_kind = CmpInstruction::G;
                break;
            case MOREEQUAL:
                op_kind = CmpInstruction::GE;
                break;
            case EQUAL:
                op_kind = CmpInstruction::E;
                break;
            case NOTEQUAL:
                op_kind = CmpInstruction::NE;
                break;
        }
        new CmpInstruction(op_kind, dst, src1, src2, bb);
        BasicBlock *truebb, *falsebb, *tempbb;
        //临时假块
        truebb = new BasicBlock(func);
        falsebb = new BasicBlock(func);
        tempbb = new BasicBlock(func);

        true_list.push_back(new CondBrInstruction(truebb, tempbb, dst, bb));

        false_list.push_back(new UncondBrInstruction(falsebb, tempbb));

    } else if (op >= ADD && op <= MOD) {
        expr1->genCode();
        expr2->genCode();
        Operand* src1 = expr1->getOperand();
        Operand* src2 = expr2->getOperand();
        int opcode;
        switch (op) {
            case ADD:
                opcode = BinaryInstruction::ADD;
                break;
            case SUB:
                opcode = BinaryInstruction::SUB;
                break;
            case MUL:
                opcode = BinaryInstruction::MUL;
                break;
            case DIV:
                opcode = BinaryInstruction::DIV;
                break;
            case MOD:
                opcode = BinaryInstruction::MOD;
                break;
        }
        new BinaryInstruction(opcode, dst, src1, src2, bb);
    }
}

void Constant::genCode() {
    // we don't need to generate code.
    // we input the value in the elder state
}

void Id::genCode() {
    BasicBlock* bb = builder->getInsertBB();
    Operand* addr = dynamic_cast<IdentifierSymbolEntry*>(symbolEntry)->getAddr();
    if (type->isInt()) new LoadInstruction(dst, addr, bb);
    //直接载入运算即可
    else if (type->isArray()) {
        if (arrIdx) {
            Type* type = ((ArrayType*)(this->type))->getElementType();
            //成员的类型
            Type* myself = this->type;
            Operand* tempSrc = addr;
            Operand* tempDst = dst;
            ExprNode* idx = arrIdx;
            //直接取表达式
            bool flag = false;
            bool pointer = false;
            while (true) {
                if (((ArrayType*)myself)->getLength() == -1) {
                    Operand* dst1 = new Operand(new TemporarySymbolEntry(new PointerType(type), SymbolTable::getLabel()));
                    tempSrc = dst1;
                    new LoadInstruction(dst1, addr, bb);
                    //已经确实找到真正需要载入变量的位置了
                    flag = true;
                }
                if (!idx) {
                    Operand* dst1 = new Operand(new TemporarySymbolEntry(new PointerType(type), SymbolTable::getLabel()));
                    Operand* idx = new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0));
                    new GepInstruction(dst1, tempSrc, idx, bb);
                    //进行指针的变换操作
                    tempDst = dst1;
                    pointer = true;
                    break;
                }

                idx->genCode();
                new GepInstruction(tempDst, tempSrc, idx->getOperand(), bb, flag);
                if (flag)
                    flag = false;
                if (type == TypeSystem::intType || type == TypeSystem::constIntType)
                    break;
                type = ((ArrayType*)type)->getElementType();
                myself = ((ArrayType*)myself)->getElementType();
                tempSrc = tempDst;
                tempDst = new Operand(new TemporarySymbolEntry(new PointerType(type), SymbolTable::getLabel()));
                idx = (ExprNode*)(idx->getNext());
            }
            dst = tempDst;
            // 如果是右值还需要一条load
            if (!left && !pointer) {
                Operand* dst1 = new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));
                new LoadInstruction(dst1, dst, bb);
                dst = dst1;
            }

        } else {
            // 这里先这么办 后续有问题再改
            // 代表数组没有传入初始化的参数
            if (((ArrayType*)(this->type))->getLength() == -1) {
                Operand* dst1 = new Operand(new TemporarySymbolEntry(new PointerType(((ArrayType*)(this->type))->getElementType()), SymbolTable::getLabel()));
                new LoadInstruction(dst1, addr, bb);
                dst = dst1;

            } else {
                Operand* idx = new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0));
                new GepInstruction(dst, addr, idx, bb);
                //相同的数组偏移的工作
            }
        }
    }
}

void IfStmt::genCode() {  
    Function* func;
    BasicBlock *then_bb, *end_bb;

    func = builder->getInsertBB()->getParent();
    then_bb = new BasicBlock(func);
    end_bb = new BasicBlock(func);

    cond->genCode();

    backPatch(cond->trueList(), then_bb);
    backPatch(cond->falseList(), end_bb);

    builder->setInsertBB(then_bb);
    thenStmt->genCode();
    then_bb = builder->getInsertBB();
    new UncondBrInstruction(end_bb, then_bb);

    builder->setInsertBB(end_bb);

}

void IfElseStmt::genCode() {
    Function* func;
    BasicBlock *then_bb, *else_bb, *end_bb /*, *bb*/;
    // bb = builder->getInsertBB();
    func = builder->getInsertBB()->getParent();
    then_bb = new BasicBlock(func);
    else_bb = new BasicBlock(func);
    end_bb = new BasicBlock(func);

    cond->genCode();
    // Operand* IfElsecond = cond->getOperand();
    backPatch(cond->trueList(), then_bb);
    backPatch(cond->falseList(), else_bb);

    // new CondBrInstruction(then_bb,else_bb,IfElsecond,bb);

    builder->setInsertBB(then_bb);
    thenStmt->genCode();
    then_bb = builder->getInsertBB();
    new UncondBrInstruction(end_bb, then_bb);

    builder->setInsertBB(else_bb);
    elseStmt->genCode();
    else_bb = builder->getInsertBB();
    new UncondBrInstruction(end_bb, else_bb);

    builder->setInsertBB(end_bb);
}

void CompoundStmt::genCode() {
    // Todo
    if (stmt)
        stmt->genCode();
}

void SeqNode::genCode() {
    // Todo
    stmt1->genCode();
    stmt2->genCode();
}

void DeclStmt::genCode() {
    IdentifierSymbolEntry* se =  dynamic_cast<IdentifierSymbolEntry*>(id->getSymbolEntry());
     
    if (se->isGlobal()) {
        Operand* addr;
        SymbolEntry* addr_se;
        addr_se = new IdentifierSymbolEntry(*se);
        addr_se->setType(new PointerType(se->getType()));
        addr = new Operand(addr_se);
        se->setAddr(addr);
        unit.insertGlobal(se);
    } else if (se->isLocal() || se->isParam()) {

        Function* func = builder->getInsertBB()->getParent();
        BasicBlock* entry = func->getEntry();
        Instruction* alloca;
        Operand* addr;
        SymbolEntry* addr_se;
        Type* type = new PointerType(se->getType());
        addr_se = new TemporarySymbolEntry(type, SymbolTable::getLabel());
        addr = new Operand(addr_se);
        alloca = new AllocaInstruction(addr, se);
        // allocate space for local id in function stack.
        entry->insertFront(alloca);  
        // allocate instructions should be inserted
        // into the begin of the entry block.
        Operand* temp;
        if (se->isParam()) temp = se->getAddr();
        se->setAddr(addr);  
        // set the addr operand in symbol entry so that
        // we can use it in subsequent code generation.
        // can use it in subsequent code generation.
        if (expr) {
            if (expr->isInitValueListExpr()) {
                BasicBlock* bb = builder->getInsertBB();
                ExprNode* temp = expr;
                std::stack<ExprNode*> stk;
                std::vector<int> idx;
                idx.push_back(0);
                while (temp) {
                    if (temp->isInitValueListExpr()) {
                        stk.push(temp);
                        idx.push_back(0);
                        temp = ((InitValueListExpr*)temp)->getExpr();
                        continue;
                        //保证还是针对列表生成
                    } else {
                        temp->genCode();
                        Type* type = ((ArrayType*)(se->getType()))->getElementType();
                        Operand* tempSrc = addr;
                        Operand* tempDst;
                        Operand* index;
                        int i = 1;
                        while (1) {
                            tempDst = new Operand(new TemporarySymbolEntry(new PointerType(type), SymbolTable::getLabel()));
                            index = (new Constant(new ConstantSymbolEntry(TypeSystem::intType, idx[i++])))->getOperand();
                            new GepInstruction(tempDst, tempSrc, index, bb);
                            if (type == TypeSystem::intType || type == TypeSystem::constIntType) break;
                            type = ((ArrayType*)type)->getElementType();
                            tempSrc = tempDst;
                        }
                        new StoreInstruction(tempDst, temp->getOperand(), bb);
                    }
                    //已经结束针对列表的工作

                    while (1) {
                        if (temp->getNext()) {
                            temp = (ExprNode*)(temp->getNext());
                            idx[idx.size() - 1]++;
                            break;
                        } else {
                            temp = stk.top();
                            stk.pop();
                            idx.pop_back();
                            if (stk.empty()) break;
                        }
                    }
                    if (stk.empty()) break;
                }
                //保证表达式串存在

            } 
            //保证了我们针对初始化列表的代码生成 



            else {
                BasicBlock* bb = builder->getInsertBB();
                expr->genCode();
                Operand* src = expr->getOperand();
                new StoreInstruction(addr, src, bb);
            }
        }
        //保证了表达式存在

        if (se->isParam()) {
            BasicBlock* bb = builder->getInsertBB();
            new StoreInstruction(addr, temp, bb);
        }
    }
    if (this->getNext()) {
        this->getNext()->genCode();
    }
}

void ReturnStmt::genCode() {
    // Todo
    BasicBlock* bb = builder->getInsertBB();
    Operand* src = nullptr;
    if (retValue) {
        retValue->genCode();
        src = retValue->getOperand();
    }
    new RetInstruction(src, bb);
}

void ExprStmt::genCode() {
    // Todo
    expr->genCode();
}
void ContinueStmt::genCode() {
    // Todo
    Function* func = builder->getInsertBB()->getParent();
    BasicBlock* bb = builder->getInsertBB();
    new UncondBrInstruction(((WhileStmt*)whileStmt)->get_cond_bb(), bb);
    BasicBlock* continue_next_bb = new BasicBlock(func);
    builder->setInsertBB(continue_next_bb);
}

void BreakStmt::genCode() {
    // Todo
    Function* func = builder->getInsertBB()->getParent();
    BasicBlock* bb = builder->getInsertBB();
    new UncondBrInstruction(((WhileStmt*)whileStmt)->get_end_bb(), bb);
    BasicBlock* break_next_bb = new BasicBlock(func);
    builder->setInsertBB(break_next_bb);
}

void WhileStmt::genCode() {
    Function* func;
    BasicBlock *cond_bb, *while_bb, *end_bb, *bb;
    bb = builder->getInsertBB();
    func = builder->getInsertBB()->getParent();
    cond_bb = new BasicBlock(func);
    while_bb = new BasicBlock(func);
    end_bb = new BasicBlock(func);

    this->cond_bb = cond_bb;
    this->end_bb = end_bb;

    new UncondBrInstruction(cond_bb, bb);

    builder->setInsertBB(cond_bb);
    cond->genCode();
    backPatch(cond->trueList(), while_bb);
    backPatch(cond->falseList(), end_bb);
    // Operand* condoperand= cond->getOperand();
    // new CondBrInstruction(while_bb,end_bb,condoperand,cond_bb);

    builder->setInsertBB(while_bb);
    stmt->genCode();

    while_bb = builder->getInsertBB();

    new UncondBrInstruction(cond_bb, while_bb);

    builder->setInsertBB(end_bb);
    
}
void BlankStmt::genCode() {
    // Todo
}
void InitValueListExpr::genCode() {
    // Todo
}
void CallExpr::genCode() {
    std::vector<Operand*> operands;
    ExprNode* temp = param;
    while (temp) {
        temp->genCode();
        operands.push_back(temp->getOperand());
        temp = ((ExprNode*)temp->getNext());
    }
    BasicBlock* bb = builder->getInsertBB();
    new CallInstruction(dst, symbolEntry, operands, bb);
}
void UnaryExpr::genCode() {
    // Todo
    expr->genCode();
    if (op == NOT) {
        BasicBlock* bb = builder->getInsertBB();
        Operand* src = expr->getOperand();
        if (expr->getType()->getSize() == 32) {
            Operand* temp = new Operand(new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel()));
            new CmpInstruction(CmpInstruction::NE, temp, src,new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0)),bb);
            src = temp;
        }
        new XorInstruction(dst, src, bb);
    } else if (op == SUB) {
        Operand* src2;
        BasicBlock* bb = builder->getInsertBB();
        Operand* src1 = new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0));
        //先new出一个操作对象
        if (expr->getType()->getSize() == 1) {
            src2 = new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));
            new ZextInstruction(src2, expr->getOperand(), bb);
            //进行类型转换相关的操作
        } else
            src2 = expr->getOperand();
        new BinaryInstruction(BinaryInstruction::SUB, dst, src1, src2, bb);
    }
}

void ExprNode::genCode() {
    // Todo
}


void AssignStmt::genCode() {
    BasicBlock* bb = builder->getInsertBB();
    expr->genCode();
    Operand* addr;
    if (lval->getOriginType()->isInt())
        addr = dynamic_cast<IdentifierSymbolEntry*>(lval->getSymbolEntry())->getAddr();
    else if (lval->getOriginType()->isArray()) {
        ((Id*)lval)->setLeft();
        lval->genCode();
        addr = lval->getOperand();
        // Type* type = new PointerType(TypeSystem::intType);
        // SymbolEntry* addr_se = new TemporarySymbolEntry(type,
        // SymbolTable::getLabel()); addr = new Operand(addr_se);
    }
    Operand* src = expr->getOperand();
    /***
     * We haven't implemented array yet, the lval can only be ID. So we just
     * store the result of the `expr` to the addr of the id. If you want to
     * implement array, you have to caculate the address first and then store
     * the result into it.
     */
    new StoreInstruction(addr, src, bb);
}

bool ContinueStmt::typeCheck(Type* retType) {
    return false;
}
bool BreakStmt::typeCheck(Type* retType) {
    return false;
}
bool WhileStmt::typeCheck(Type* retType) {
    if (stmt) return stmt->typeCheck(retType);
    return false;
}
bool BlankStmt::typeCheck(Type* retType) {
    return false;
}
bool InitValueListExpr::typeCheck(Type* retType) {
    return false;
}
bool CallExpr::typeCheck(Type* retType) {
    return false;
}
bool UnaryExpr::typeCheck(Type* retType) {
    return false;
}

bool ExprStmt::typeCheck(Type* retType) {
    return false;
}



bool Ast::typeCheck(Type* retType) {
    if (root != nullptr)
        return root->typeCheck();
    return false;
}

bool FunctionDef::typeCheck(Type* retType) {
    SymbolEntry* se = this->getSymbolEntry();
    Type* ret = ((FunctionType*)(se->getType()))->getRetType();
    StmtNode* stmt = this->stmt;
    if (stmt == nullptr) {
        if (ret != TypeSystem::voidType)
            fprintf(stderr, "non-void function does not return a value\n");
        // 不嵌套函数定义就返回了
        return false;
    }
    if (!stmt->typeCheck(ret)) {
        fprintf(stderr, "function does not have a return statement\n");
        return false;
    }
    return false;
}

bool BinaryExpr::typeCheck(Type* retType) {
    return false;
}

bool Constant::typeCheck(Type* retType) {
    return false;
}

bool Id::typeCheck(Type* retType) {
    return false;
}

bool IfStmt::typeCheck(Type* retType) {
    if (thenStmt)
        return thenStmt->typeCheck(retType);
    return false;
}

bool IfElseStmt::typeCheck(Type* retType) {
    bool flag1 = false, flag2 = false;
    if (thenStmt)
        flag1 = thenStmt->typeCheck(retType);
    if (elseStmt)
        flag2 = elseStmt->typeCheck(retType);
    return flag1 || flag2;
}

bool CompoundStmt::typeCheck(Type* retType) {
    if (stmt)
        return stmt->typeCheck(retType);
    return false;
}

bool SeqNode::typeCheck(Type* retType) {
    bool flag1 = false, flag2 = false;
    if (stmt1)
        flag1 = stmt1->typeCheck(retType);
    if (stmt2)
        flag2 = stmt2->typeCheck(retType);
    return flag1 || flag2;
}

bool DeclStmt::typeCheck(Type* retType) {
    return false;
}

bool ReturnStmt::typeCheck(Type* retType) {
    if (!retType) {
        fprintf(stderr, "expected unqualified-id\n");
        return true;
    }
    if (!retValue && !retType->isVoid()) {
        fprintf(
            stderr,
            "return-statement with no value, in function returning \'%s\'\n",
            retType->toStr().c_str());
        return true;
    }
    if (retValue && retType->isVoid()) {
        fprintf(
            stderr,
            "return-statement with a value, in function returning \'void\'\n");
        return true;
    }
    if (!retValue || !retValue->getSymbolEntry())
        return true;
    Type* type = retValue->getType();
    if (type != retType) {
        fprintf(stderr,
                "cannot initialize return object of type \'%s\' with an rvalue "
                "of type \'%s\'\n",
                retType->toStr().c_str(), type->toStr().c_str());
    }
    return true;
}

bool AssignStmt::typeCheck(Type* retType) {
    return false;
}

CallExpr::CallExpr(SymbolEntry* se, ExprNode* param)
    : ExprNode(se), param(param) {
    // 做参数的检查
    dst = nullptr;
    SymbolEntry* s = se;
    int param_num = 0;
    ExprNode* temp = param;
    while (temp) {
        param_num++;
        temp = (ExprNode*)(temp->getNext());
    }
    while (s) {
        Type* type = s->getType();
        std::vector<Type*> params = ((FunctionType*)type)->getParamsType();
        if ((long unsigned int)param_num == params.size()) {
            this->symbolEntry = s;
            break;
        }
        s = s->getNext();
    }
    if (symbolEntry) {
        Type* type = symbolEntry->getType();
        this->type = ((FunctionType*)type)->getRetType();
        if (this->type != TypeSystem::voidType) {
            SymbolEntry* se = new TemporarySymbolEntry(this->type, SymbolTable::getLabel());
            dst = new Operand(se);
        }
        std::vector<Type*> params = ((FunctionType*)type)->getParamsType();
        ExprNode* temp = param;
        for (auto it = params.begin(); it != params.end(); it++) {
            if (temp == nullptr) {
                fprintf(stderr, "lack arguments to function %s %s\n",
                        symbolEntry->toStr().c_str(), type->toStr().c_str());
                break;
            } else if ((*it)->getKind() != temp->getType()->getKind())
                fprintf(stderr, "parameter's type %s can't convert to %s\n",
                        temp->getType()->toStr().c_str(), (*it)->toStr().c_str());
            temp = (ExprNode*)(temp->getNext());
        }
        if (temp != nullptr) {
            fprintf(stderr, "more arguments to function %s %s\n",
                    symbolEntry->toStr().c_str(), type->toStr().c_str());
        }
    }
    if (((IdentifierSymbolEntry*)se)->isSysy()) {
        unit.insertDeclare(se);
    }
}

AssignStmt::AssignStmt(ExprNode* lval, ExprNode* expr)
    : lval(lval), expr(expr) {
    Type* type = ((Id*)lval)->getType();
    SymbolEntry* se = lval->getSymbolEntry();
    bool flag = true;
    if (type->isInt()) {
        if (((IntType*)type)->isConst()) {
            fprintf(stderr,
                    "cannot assign to variable \'%s\' with const-qualified "
                    "type \'%s\'\n",
                    ((IdentifierSymbolEntry*)se)->toStr().c_str(),
                    type->toStr().c_str());
            flag = false;
        }
    } else if (type->isArray()) {
        fprintf(stderr, "array type \'%s\' is not assignable\n",
                type->toStr().c_str());
        flag = false;
    }
    if (flag && !expr->getType()->isInt()) {
        fprintf(stderr,
                "cannot initialize a variable of type \'int\' with an rvalue "
                "of type \'%s\'\n",
                expr->getType()->toStr().c_str());
    }
}

Type* Id::getType() {
    SymbolEntry* se = this->getSymbolEntry();
    if (!se) return TypeSystem::voidType;
    Type* type = se->getType();
    if (!arrIdx) return type;
    else if (!type->isArray()) {
        fprintf(stderr, "1 subscripted value is not an array\n");
        return TypeSystem::voidType;
    } else {
        ArrayType* temp1 = (ArrayType*)type;
        ExprNode* temp2 = arrIdx;
        while (!temp1->getElementType()->isInt()) {
            if (!temp2) {
                return temp1;
            }
            temp2 = (ExprNode*)(temp2->getNext());
            temp1 = (ArrayType*)(temp1->getElementType());
        }
        if (!temp2) {
            fprintf(stderr, "2 subscripted value is not an array\n");
            return temp1;
        } else if (temp2->getNext()) {
            fprintf(stderr, "3 subscripted value is not an array\n");
            return TypeSystem::voidType;
        }
    }
    return TypeSystem::intType;
}

void ExprNode::output(int level) {
    std::string name, type;
    name = symbolEntry->toStr();
    type = symbolEntry->getType()->toStr();
    fprintf(yyout, "%*cconst string\ttype:%s\t%s\n", level, ' ', type.c_str(),
            name.c_str());
}

void Ast::output() {
    fprintf(yyout, "program\n");
    if (root != nullptr)
        root->output(4);
}

void BinaryExpr::output(int level) {
    std::string op_str;
    switch (op) {
        case ADD:
            op_str = "add";
            break;
        case SUB:
            op_str = "sub";
            break;
        case MUL:
            op_str = "mul";
            break;
        case DIV:
            op_str = "div";
            break;
        case MOD:
            op_str = "mod";
            break;
        case AND:
            op_str = "and";
            break;
        case OR:
            op_str = "or";
            break;
        case LESS:
            op_str = "less";
            break;
        case LESSEQUAL:
            op_str = "lessequal";
            break;
        case MORE:
            op_str = "MORE";
            break;
        case MOREEQUAL:
            op_str = "MOREequal";
            break;
        case EQUAL:
            op_str = "equal";
            break;
        case NOTEQUAL:
            op_str = "notequal";
            break;
    }
    fprintf(yyout, "%*cBinaryExpr\top: %s\ttype: %s\n", level, ' ',
            op_str.c_str(), type->toStr().c_str());
    expr1->output(level + 4);
    expr2->output(level + 4);
}

int BinaryExpr::getValue() {
    int value;
    switch (op) {
        case ADD:
            value = expr1->getValue() + expr2->getValue();
            break;
        case SUB:
            value = expr1->getValue() - expr2->getValue();
            break;
        case MUL:
            value = expr1->getValue() * expr2->getValue();
            break;
        case DIV:
            value = expr1->getValue() / expr2->getValue();
            break;
        case MOD:
            value = expr1->getValue() % expr2->getValue();
            break;
        case AND:
            value = expr1->getValue() && expr2->getValue();
            break;
        case OR:
            value = expr1->getValue() || expr2->getValue();
            break;
        case LESS:
            value = expr1->getValue() < expr2->getValue();
            break;
        case LESSEQUAL:
            value = expr1->getValue() <= expr2->getValue();
            break;
        case MORE:
            value = expr1->getValue() > expr2->getValue();
            break;
        case MOREEQUAL:
            value = expr1->getValue() >= expr2->getValue();
            break;
        case EQUAL:
            value = expr1->getValue() == expr2->getValue();
            break;
        case NOTEQUAL:
            value = expr1->getValue() != expr2->getValue();
            break;
    }
    return value;
}

UnaryExpr::UnaryExpr(SymbolEntry* se, int op, ExprNode* expr)
    : ExprNode(se, UNARYEXPR), op(op), expr(expr) {
    std::string op_str = op == UnaryExpr::NOT ? "!" : "-";
    if (expr->getType()->isVoid()) {
        fprintf(stderr,
                "invalid operand of type \'void\' to unary \'opeartor%s\'\n",
                op_str.c_str());
    }
    if (op == UnaryExpr::NOT) {
        type = TypeSystem::intType;
        dst = new Operand(se);
        if (expr->isUnaryExpr()) {
            UnaryExpr* ue = (UnaryExpr*)expr;
            if (ue->getOp() == UnaryExpr::NOT) {
                if (ue->getType() == TypeSystem::intType)
                    ue->setType(TypeSystem::boolType);
                // type = TypeSystem::intType;
            }
        }
    } else if (op == UnaryExpr::SUB) {
        type = TypeSystem::intType;
        dst = new Operand(se);
        if (expr->isUnaryExpr()) {
            UnaryExpr* ue = (UnaryExpr*)expr;
            if (ue->getOp() == UnaryExpr::NOT)
                if (ue->getType() == TypeSystem::intType)
                    ue->setType(TypeSystem::boolType);
        }
    }
};

void UnaryExpr::output(int level) {
    std::string op_str;
    switch (op) {
        case NOT:
            op_str = "not";
            break;
        case SUB:
            op_str = "minus";
            break;
    }
    fprintf(yyout, "%*cUnaryExpr\top: %s\ttype: %s\n", level, ' ',
            op_str.c_str(), type->toStr().c_str());
    expr->output(level + 4);
}

int UnaryExpr::getValue() {
    int value;
    switch (op) {
        case NOT:
            value = !(expr->getValue());
            break;
        case SUB:
            value = -(expr->getValue());
            break;
    }
    return value;
}

void CallExpr::output(int level) {
    std::string name, type;
    int scope;
    if (symbolEntry) {
        name = symbolEntry->toStr();
        type = symbolEntry->getType()->toStr();
        scope = dynamic_cast<IdentifierSymbolEntry*>(symbolEntry)->getScope();
        fprintf(yyout, "%*cCallExpr\tfunction name: %s\tscope: %d\ttype: %s\n",
                level, ' ', name.c_str(), scope, type.c_str());
        Node* temp = param;
        while (temp) {
            temp->output(level + 4);
            temp = temp->getNext();
        }
    }
}

void Constant::output(int level) {
    std::string type, value;
    type = symbolEntry->getType()->toStr();
    value = symbolEntry->toStr();
    fprintf(yyout, "%*cIntegerLiteral\tvalue: %s\ttype: %s\n", level, ' ',
            value.c_str(), type.c_str());
}

int Constant::getValue() {
    // assert(symbolEntry->getType()->isInt());
    return ((ConstantSymbolEntry*)symbolEntry)->getValue();
}

int Id::getValue() {
    // assert(symbolEntry->getType()->isInt());
    return ((IdentifierSymbolEntry*)symbolEntry)->getValue();
}

void Id::output(int level) {
    std::string name, type;
    int scope;
    if (symbolEntry) {
        name = symbolEntry->toStr();
        type = symbolEntry->getType()->toStr();
        scope = dynamic_cast<IdentifierSymbolEntry*>(symbolEntry)->getScope();
        fprintf(yyout, "%*cId\tname: %s\tscope: %d\ttype: %s\n", level, ' ',
                name.c_str(), scope, type.c_str());
        if (arrIdx) {
            ExprNode* temp = arrIdx;
            int i = 0;
            while (temp) {
                temp->output(level + 4 + 4 * i++);
                temp = (ExprNode*)(temp->getNext());
            }
        }
    }
}

void InitValueListExpr::output(int level) {
    std::string type;
    if (symbolEntry->getType())
        type = symbolEntry->getType()->toStr();
    fprintf(yyout, "%*cInitValueListExpr\ttype: %s\n", level, ' ',
            type.c_str());
    Node* temp = expr;
    while (temp) {
        temp->output(level + 4);
        temp = temp->getNext();
    }
}

void InitValueListExpr::addExpr(ExprNode* expr) {
    if (this->expr == nullptr) {
        assert(childCnt == 0);
        childCnt++;
        this->expr = expr;
    } else {
        childCnt++;
        this->expr->setNext(expr);
    }
}

bool InitValueListExpr::isFull() {
    ArrayType* type = (ArrayType*)(this->symbolEntry->getType());
    return childCnt == type->getLength();
}

void InitValueListExpr::fill() {
    Type* type = ((ArrayType*)(this->getType()))->getElementType();
    if (type->isArray()) {
        while (!isFull())
            this->addExpr(new InitValueListExpr(new ConstantSymbolEntry(type)));
        ExprNode* temp = expr;
        while (temp) {
            ((InitValueListExpr*)temp)->fill();
            temp = (ExprNode*)(temp->getNext());
        }
    }
    if (type->isInt()) {
        while (!isFull())
            this->addExpr(new Constant(new ConstantSymbolEntry(type, 0)));
        return;
    }
}

void ImplictCastExpr::output(int level) {
    fprintf(yyout, "%*cImplictCastExpr\ttype: %s to %s\n", level, ' ',
            expr->getType()->toStr().c_str(), type->toStr().c_str());
    this->expr->output(level + 4);
}

void CompoundStmt::output(int level) {
    fprintf(yyout, "%*cCompoundStmt\n", level, ' ');
    if (stmt)
        stmt->output(level + 4);
}

void SeqNode::output(int level) {
    // fprintf(yyout, "%*cSequence\n", level, ' ');
    stmt1->output(level);
    stmt2->output(level);
}

void DeclStmt::output(int level) {
    fprintf(yyout, "%*cDeclStmt\n", level, ' ');
    id->output(level + 4);
    if (expr)
        expr->output(level + 4);
    if (this->getNext()) {
        this->getNext()->output(level);
    }
}

void BlankStmt::output(int level) {
    fprintf(yyout, "%*cBlankStmt\n", level, ' ');
}

void IfStmt::output(int level) {
    fprintf(yyout, "%*cIfStmt\n", level, ' ');
    cond->output(level + 4);
    thenStmt->output(level + 4);
}

void IfElseStmt::output(int level) {
    fprintf(yyout, "%*cIfElseStmt\n", level, ' ');
    cond->output(level + 4);
    thenStmt->output(level + 4);
    elseStmt->output(level + 4);
}

void WhileStmt::output(int level) {
    fprintf(yyout, "%*cWhileStmt\n", level, ' ');
    cond->output(level + 4);
    stmt->output(level + 4);
}
void BreakStmt::output(int level) {
    fprintf(yyout, "%*cBreakStmt\n", level, ' ');
}

void ContinueStmt::output(int level) {
    fprintf(yyout, "%*cContinueStmt\n", level, ' ');
}

void ReturnStmt::output(int level) {
    fprintf(yyout, "%*cReturnStmt\n", level, ' ');
    if (retValue != nullptr)
        retValue->output(level + 4);
}

void AssignStmt::output(int level) {
    fprintf(yyout, "%*cAssignStmt\n", level, ' ');
    lval->output(level + 4);
    expr->output(level + 4);
}

void ExprStmt::output(int level) {
    fprintf(yyout, "%*cExprStmt\n", level, ' ');
    expr->output(level + 4);
}

void FunctionDef::output(int level) {
    std::string name, type;
    name = se->toStr();
    type = se->getType()->toStr();
    fprintf(yyout, "%*cFunctionDefine\tfunction name: %s\ttype: %s\n", level,
            ' ', name.c_str(), type.c_str());
    if (decl) {
        decl->output(level + 4);
    }
    stmt->output(level + 4);
}

void ImplictCastExpr::genCode() {
    expr->genCode();
    BasicBlock* bb = builder->getInsertBB();
    Function* func = bb->getParent();
    BasicBlock* trueBB = new BasicBlock(func);
    BasicBlock* tempbb = new BasicBlock(func);
    BasicBlock* falseBB = new BasicBlock(func);

    new CmpInstruction(
        CmpInstruction::NE, this->dst, this->expr->getOperand(),
        new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0)), bb);
    this->trueList().push_back(
        new CondBrInstruction(trueBB, tempbb, this->dst, bb));
    this->falseList().push_back(new UncondBrInstruction(falseBB, tempbb));
}