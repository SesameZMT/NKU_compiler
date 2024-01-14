#ifndef __SYMBOLTABLE_H__
#define __SYMBOLTABLE_H__

#include <optional>
#include <string>
#include <map>
#include "Type.h"

class Type;
class Operand;

inline bool issysruntime(char* str)
{
    std::string t = str;
    if(t=="getint"||t=="getch"||t=="getfloat"||t=="getarray"||t=="getfarray"
    ||t=="putint"||t=="putch"||t=="putfloat"||t=="putarray"||t=="putfarray"
    ||t=="putf"||t=="starttime"||t=="stoptime")
    {
        return true;
    }
    return false;
}
class SymbolEntry
{
private:
    int kind;
protected:
    enum {CONSTANT, VARIABLE, TEMPORARY};
    Type *type;

public:
    SymbolEntry(Type *type, int kind);
    virtual ~SymbolEntry() {};
    bool isConstant() const {return kind == CONSTANT;};
    bool isTemporary() const {return kind == TEMPORARY;};
    bool isVariable() const {return kind == VARIABLE;};
    Type* getType() {return type;};
    void setType(Type *type) {this->type = type;};
    virtual std::string toStr() = 0;
    // You can add any function you need here.
};


/*  
    Symbol entry for literal constant. Example:

    int a = 1;

    Compiler should create constant symbol entry for literal constant '1'.
*/
class IntSymbolEntry : public SymbolEntry
{
private:
    int value;

public:
    IntSymbolEntry(Type *type, int value);
    virtual ~IntSymbolEntry() {};
    int getValue() const {return value;};
    std::string toStr();
    // You can add any function you need here.
};

class FloatSymbolEntry : public SymbolEntry
{
private:
    float value;

public:
    FloatSymbolEntry(Type *type, float value);
    virtual ~FloatSymbolEntry() {};
    int getValue() const {return value;};
    std::string toStr();
    // You can add any function you need here.
};

/* 
    Symbol entry for identifier. Example:

    int a;
    int b;
    void f(int c)
    {
        int d;
        {
            int e;
        }
    }

    Compiler should create identifier symbol entries for variables a, b, c, d and e:

    | variable | scope    |
    | a        | GLOBAL   |
    | b        | GLOBAL   |
    | c        | PARAM    |
    | d        | LOCAL    |
    | e        | LOCAL +1 |
*/
class IdentifierSymbolEntry : public SymbolEntry
{
private:
    enum {GLOBAL, PARAM, LOCAL};
    std::string name;
    int scope;
    Operand *addr;
    std::optional<float> fval = std::nullopt;
    std::optional<int> ival = std::nullopt;
    std::optional<std::vector<float>> farray = std::nullopt;
    std::optional<std::vector<int>> iarray = std::nullopt;

public:
    IdentifierSymbolEntry(Type *type, std::string name, int scope);
    virtual ~IdentifierSymbolEntry() {};
    std::string toStr();
    bool isGlobal() const {return scope == GLOBAL;};
    bool isParam() const {return scope == PARAM;};
    bool isLocal() const {return scope >= LOCAL;};
    int getScope() const {return scope;};
    void setAddr(Operand *addr) {this->addr = addr;};
    void changeFloat(std::optional<float> f){fval = f;}
    void changeInt(std::optional<int> i){ival = i;}
    void changeFloatArray(std::optional<std::vector<float>> f){farray = f;}
    void changeIntArray(std::optional<std::vector<int>> i){iarray = i;}
    bool hasInt(){return ival!=std::nullopt;}
    bool hasFloat(){return fval!=std::nullopt;}
    bool hasIntArray(){return iarray!=std::nullopt;}
    bool hasFloatArray(){return farray!=std::nullopt;}
    bool hasValue(){return fval!=std::nullopt||ival!=std::nullopt;}
    bool hasArray(){return farray!=std::nullopt||iarray!=std::nullopt;}
    std::optional<int> getIntop(){return ival;}
    std::optional<std::vector<int>> getIntArrayop(){return iarray;}
    std::optional<float> getFloatop(){return fval;}
    std::optional<std::vector<float>> getFloatArrayop(){return farray;}
    int getInt(std::vector<int>);
    float getFloat(std::vector<int>);
    int getInt(){if(fval!=std::nullopt)return static_cast<int>(fval.value());else return ival.value();}
    float getFloat(){if(ival!=std::nullopt)return static_cast<float>(ival.value());else return fval.value();}
    Operand* getAddr() {return addr;};
    // You can add any function you need here.
};


/* 
    Symbol entry for temporary variable created by compiler. Example:

    int a;
    a = 1 + 2 + 3;

    The compiler would generate intermediate code like:

    t1 = 1 + 2
    t2 = t1 + 3
    a = t2

    So compiler should create temporary symbol entries for t1 and t2:

    | temporary variable | label |
    | t1                 | 1     |
    | t2                 | 2     |
*/
class TemporarySymbolEntry : public SymbolEntry
{
private:
    int label;
public:
    TemporarySymbolEntry(Type *type, int label);
    virtual ~TemporarySymbolEntry() {};
    std::string toStr();
    int getLabel() const {return label;};
    // You can add any function you need here.
};

// symbol table managing identifier symbol entries
class SymbolTable
{
private:
    std::map<std::string, SymbolEntry*> symbolTable;
    SymbolTable *prev;
    int level;
    static int counter;
public:
    SymbolTable();
    SymbolTable(SymbolTable *prev);
    void install(std::string name, SymbolEntry* entry);
    SymbolEntry* lookup(std::string name);
    SymbolTable* getPrev() {return prev;};
    int getLevel() {return level;};
    static int getLabel() {return counter++;};
    friend void initsys();
};

extern SymbolTable *identifiers;
extern SymbolTable *globals;

inline void initsys()
{
    /*
    std::string strarr[13]{"getint","getch","getfloat","getarray","getfarray"
    ,"putint","putch","putfloat","putarray","putfarray"
    ,"putf","starttime","stoptime"};
    */
    std::string strarr[13]{"_getint","_getch","_getfloat","_getarray","_getfarray"
    ,"_putint","_putch","_putfloat","_putarray","_putfarray"
    ,"_putf","_starttime","_stoptime"};

    SymbolTable * iter = identifiers;
    while(iter->prev!=nullptr)
    {
        iter = iter->prev;
    }
    for(int i = 0;i<13;++i)
    {
        Type *funcType;
        if(i<2)
        {
            funcType = new FunctionType(TypeSystem::intType,{});
        }
        if(i==2)
        {
            funcType = new FunctionType(TypeSystem::floatType,{});
        }
        if(i==3)
        {
            funcType = new FunctionType(TypeSystem::intType,{new PointerType(TypeSystem::intType)});
        }
        if(i==4)
        {
            funcType = new FunctionType(TypeSystem::intType,{new PointerType(TypeSystem::floatType)});
        }
        if(i==5||i==6)
        {
            funcType = new FunctionType(TypeSystem::voidType,{TypeSystem::intType});
        }
        if(i==7)
        {
            funcType = new FunctionType(TypeSystem::voidType,{TypeSystem::floatType});
        }
        if(i==8)
        {
            funcType = new FunctionType(TypeSystem::voidType,{TypeSystem::intType,new PointerType(TypeSystem::intType)});
        }
        if(i==9)
        {
            funcType = new FunctionType(TypeSystem::voidType,{TypeSystem::intType,new PointerType(TypeSystem::floatType)});
        }
        if(i>9)
        {
            funcType = new FunctionType(TypeSystem::voidType,{});
        }//todo
        SymbolEntry *se = new IdentifierSymbolEntry(funcType, strarr[i], identifiers->getLevel());
        iter->install(strarr[i], se);
    }
}

#endif
