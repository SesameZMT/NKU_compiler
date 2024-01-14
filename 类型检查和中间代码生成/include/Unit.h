#ifndef __UNIT_H__
#define __UNIT_H__

#include <vector>
#include "Ast.h"
#include "Function.h"

class Unit
{
    typedef std::vector<Function *>::iterator iterator;
    typedef std::vector<Function *>::reverse_iterator reverse_iterator;

private:
    std::vector<Function *> func_list;
    Instruction* globalbb{};
public:
    Unit() = default;
    ~Unit() ;
    void insertFunc(Function *);
    void removeFunc(Function *);
    void deadinstelim();
    void output() const;
    Instruction*& getbb(){return globalbb;}
    iterator begin() { return func_list.begin(); };
    iterator end() { return func_list.end(); };
    reverse_iterator rbegin() { return func_list.rbegin(); };
    reverse_iterator rend() { return func_list.rend(); };
};

#endif
