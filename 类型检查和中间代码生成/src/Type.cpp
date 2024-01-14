#include "Type.h"
#include <sstream>
#include <functional>

IntType TypeSystem::commonInt = IntType(32);
IntType TypeSystem::commonBool = IntType(1);
FloatType TypeSystem::commonFloat = FloatType();
VoidType TypeSystem::commonVoid = VoidType();

Type* TypeSystem::intType = &commonInt;
Type* TypeSystem::voidType = &commonVoid;
Type* TypeSystem::boolType = &commonBool;
Type* TypeSystem::floatType = &commonFloat;
bool PointerType::isInt() const {return valueType->isInt();}
bool PointerType::isFloat() const {return valueType->isFloat();}
bool ArrayType::isInt() const {return valueType->isInt();}
bool ArrayType::isFloat() const {return valueType->isFloat();}
bool Type::isInt() const {return kind == INT;}
bool Type::isVoid() const {return kind == VOID;}
bool Type::isFunc() const {return kind == FUNC;}
bool Type::isFloat() const {return kind == FLOAT;}
bool Type::isPointer() const {return kind == PTR;}
bool Type::isArray() const {return kind == ARRAY;}

std::string IntType::toStr()
{
    std::ostringstream buffer;
    buffer << "i" << size;
    return buffer.str();
}

std::string FloatType::toStr()
{
    return "float";
}

std::string VoidType::toStr()
{
    return "void";
}

std::string FunctionType::toStr()
{
    std::ostringstream buffer;
    buffer << returnType->toStr() << "()";
    return buffer.str();
}

std::string PointerType::toStr()
{
    std::ostringstream buffer;
    buffer << valueType->toStr() << "*";
    return buffer.str();
}

std::string ArrayType::toStr()
{
    std::ostringstream buffer;
    size_t index = 0;
    std::function<void()> printarr = [&](){
        buffer<<"[" << dims[index] << " x ";
        if(index == dims.size()-1)
        {
            if(isInt())
            {
                buffer<<"i32";
            }
            else
            {
                buffer<<"float";
            }
        }
        else
        {
            ++index;
            printarr();
        }
        buffer<<"]";
    };
	printarr();
    return buffer.str();
}
