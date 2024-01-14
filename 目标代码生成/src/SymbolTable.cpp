#include "SymbolTable.h"
#include <iostream>
#include <sstream>
#include <cassert>

int IdentifierSymbolEntry::getInt(std::vector<int> vec)
{
    int mod = 1;
    int index = 0;
    auto dims = dynamic_cast<ArrayType*>(type)->getDims();
    for(size_t i = 0;i<vec.size();++i)
    {
        mod *= dims[i];
    }
    for(size_t i = 0;i<vec.size();++i)
    {
        mod /= dims[i];
        index += vec[i]*mod;
    }
    return iarray.value()[index];
}

float IdentifierSymbolEntry::getFloat(std::vector<int> vec)
{
    int mod = 1;
    int index = 0;
    auto dims = dynamic_cast<ArrayType*>(type)->getDims();
    for(size_t i = 0;i<vec.size();++i)
    {
        mod *= dims[i];
    }
    for(size_t i = 0;i<vec.size();++i)
    {
        mod /= dims[i];
        index += vec[i]*mod;
    }
    return farray.value()[index];
}

SymbolEntry::SymbolEntry(Type *type, int kind) 
{
    this->type = type;
    this->kind = kind;
}

IntSymbolEntry::IntSymbolEntry(Type *type, int value) : SymbolEntry(type, SymbolEntry::CONSTANT)
{
    this->value = value;
}

std::string IntSymbolEntry::toStr()
{
    std::ostringstream buffer;
    buffer << value;
    return buffer.str();
}

FloatSymbolEntry::FloatSymbolEntry(Type *type, float value) : SymbolEntry(type, SymbolEntry::CONSTANT)
{
    this->value = value;
}

std::string FloatSymbolEntry::toStr()
{
    if(value==0)
        return "0.000000e+00";
    union {
        double val;
        uint64_t hex;
    } u;
    u.val = value;
    std::stringstream ss;
    std::string str;
    ss << std::hex << std::showbase << u.hex;
    ss >> str;
    return str;
}

IdentifierSymbolEntry::IdentifierSymbolEntry(Type *type, std::string name, int scope) : SymbolEntry(type, SymbolEntry::VARIABLE), name(name)
{
    this->scope = scope;
    addr = nullptr;
}

std::string IdentifierSymbolEntry::toStr()
{
    if(scope==1)
        return "%" + name;
    return "@" + name;
}

TemporarySymbolEntry::TemporarySymbolEntry(Type *type, int label) : SymbolEntry(type, SymbolEntry::TEMPORARY)
{
    this->label = label;
}

std::string TemporarySymbolEntry::toStr()
{
    std::ostringstream buffer;
    buffer << "%t" << label;
    return buffer.str();
}

SymbolTable::SymbolTable()
{
    prev = nullptr;
    level = 0;
}

SymbolTable::SymbolTable(SymbolTable *prev)
{
    this->prev = prev;
    this->level = prev->level + 1;
}

SymbolEntry* SymbolTable::lookup(std::string name)
{
    SymbolTable * iter = this;
    while(iter!=nullptr)
    {
        auto res = iter->symbolTable.find(name);
        if(res!=iter->symbolTable.end())
        {
            return res->second;
        }
        iter = iter->prev;
    }
    return nullptr;
}

void SymbolTable::install(std::string name, SymbolEntry* entry)
{
    assert(symbolTable.find(name)==symbolTable.end());
    symbolTable[name] = entry;
}

int SymbolTable::counter = 0;
static SymbolTable t;
SymbolTable *identifiers = &t;
SymbolTable *globals = &t;
