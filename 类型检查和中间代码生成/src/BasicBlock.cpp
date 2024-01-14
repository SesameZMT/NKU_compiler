#include "BasicBlock.h"
#include "Function.h"
#include <algorithm>

extern FILE* yyout;

int BasicBlock::deadinstelim()
{
    int count = 0;
    for (auto inst = head->getNext(); inst != head; inst = inst->getNext())
    {
        if(inst->getInstType()!=15)
        {
            auto op = inst->getDef();
            if(op!=nullptr&&op->getUse().empty())
            {
                auto next = inst->getNext();
                auto prev = inst->getPrev();
                (inst->getPrev())->setNext(next);
                (inst->getNext())->setPrev(prev);
                inst->setParent(nullptr);
                ++count;
                delete inst;
            }
        }
    }
    return count;
}
// insert the instruction to the front of the basicblock.
void BasicBlock::insertFront(Instruction *inst)
{
    insertBefore(inst, head->getNext());
}

// insert the instruction to the back of the basicblock.
void BasicBlock::insertBack(Instruction *inst) 
{
    insertBefore(inst, head);
}

// insert the instruction dst before src.
void BasicBlock::insertBefore(Instruction *dst, Instruction *src)
{
    // Todo
    dst->setNext(src);
    dst->setPrev(src->getPrev());
    src->setPrev(dst);
    (dst->getPrev())->setNext(dst);
    dst->setParent(this);
}
// remove the instruction from intruction list.
void BasicBlock::remove(Instruction *inst)
{
    inst->getPrev()->setNext(inst->getNext());
    inst->getNext()->setPrev(inst->getPrev());
}

int BasicBlock::output() const
{
    fprintf(yyout, "B%d:", no);
    int res = 0;
    if (!pred.empty())
    {
        fprintf(yyout, "%*c; preds = %%B%d", 32, '\t', pred[0]->getNo());
        for (auto i = pred.begin() + 1; i != pred.end(); i++)
            fprintf(yyout, ", %%B%d", (*i)->getNo());
    }
    fprintf(yyout, "\n");
    for (auto i = head->getNext(); i != head; i = i->getNext())
    {
        i->output();
        if(i->getInstType()==3)
            res = 1;
    }
    return res;
}

void BasicBlock::addSucc(BasicBlock *bb)
{
    succ.push_back(bb);
}

// remove the successor basicclock bb.
void BasicBlock::removeSucc(BasicBlock *bb)
{
    succ.erase(std::find(succ.begin(), succ.end(), bb));
}

void BasicBlock::addPred(BasicBlock *bb)
{
    pred.push_back(bb);
}

// remove the predecessor basicblock bb.
void BasicBlock::removePred(BasicBlock *bb)
{
    pred.erase(std::find(pred.begin(), pred.end(), bb));
}

BasicBlock::BasicBlock(Function *f)
{
    this->no = SymbolTable::getLabel();
    f->insertBlock(this);
    parent = f;
    head = new DummyInstruction();
    head->setParent(this);
}

BasicBlock::~BasicBlock()
{
    Instruction *inst;
    inst = head->getNext();
    while (inst != head)
    {
        Instruction *t;
        t = inst;
        inst = inst->getNext();
        delete t;
    }
    for(auto &bb:pred)
        bb->removeSucc(this);
    for(auto &bb:succ)
        bb->removePred(this);
    if(parent!=nullptr)
        parent->remove(this);
}
