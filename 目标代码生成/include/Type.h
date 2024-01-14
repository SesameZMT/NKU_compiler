#ifndef __TYPE_H__
#define __TYPE_H__
#include <vector>
#include <string>

class Type
{
private:
    int kind;
protected:
    enum {INT, VOID, FUNC, PTR, FLOAT, ARRAY};
public:
    Type(int kind) : kind(kind) {};
    virtual ~Type() {};
    virtual std::string toStr() = 0;
    virtual bool isInt() const;
    bool isVoid() const;
    bool isFunc() const;
    bool isPointer() const;
    bool isArray() const;
    virtual bool isFloat() const;
};

class IntType : public Type
{
private:
    int size;
public:
    IntType(int size) : Type(Type::INT), size(size){};
    std::string toStr();
};

class FloatType : public Type
{
public:
    FloatType() : Type(Type::FLOAT){};
    std::string toStr();
};

class VoidType : public Type
{
public:
    VoidType() : Type(Type::VOID){};
    std::string toStr();
};

class FunctionType : public Type
{
private:
    Type *returnType;
    std::vector<Type*> paramsType;
public:
    FunctionType(Type* returnType, std::vector<Type*> paramsType) : 
    Type(Type::FUNC), returnType(returnType), paramsType(paramsType){};
    Type* getRetType() {return returnType;};
    std::vector<Type*> getParams() {return paramsType;};
    void changeParams(std::vector<Type*> t){paramsType = t;}
    std::string toStr();
};

class PointerType : public Type
{
private:
    Type *valueType;
public:
    PointerType(Type* valueType) : Type(Type::PTR) {this->valueType = valueType;};
    std::string toStr();
    Type * getValue(){return valueType;}
    virtual bool isInt() const;
    virtual bool isFloat() const;
};

class ArrayType : public Type
{
private:
    std::vector<int> dims;
    Type *valueType;
public:
    ArrayType(Type* valueType,std::vector<int> d) : Type(Type::ARRAY),dims(d) {this->valueType = valueType;};
    ArrayType(ArrayType const &a) : Type(Type::ARRAY) {dims = a.dims;valueType = a.valueType;};
    std::string toStr();
    virtual bool isInt() const;
    virtual bool isFloat() const;
    std::vector<int> getDims() {return dims;}
    void eraseFirst() {dims.erase(dims.begin());}
};

class TypeSystem
{
private:
    static IntType commonInt;
    static IntType commonBool;
    static VoidType commonVoid;
    static FloatType commonFloat;
public:
    static Type *intType;
    static Type *voidType;
    static Type *boolType;
    static Type *floatType;
};

#endif
