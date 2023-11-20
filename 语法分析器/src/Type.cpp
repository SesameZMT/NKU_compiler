#include "Type.h"
#include <sstream>

ConstIntType TypeSystem::commonConstInt =ConstIntType(4);
IntType TypeSystem::commonInt = IntType(4);
VoidType TypeSystem::commonVoid = VoidType();

Type* TypeSystem::constintType=&commonConstInt;
Type* TypeSystem::intType = &commonInt;
Type* TypeSystem::voidType = &commonVoid;

std::string IntType::toStr()
{
    return "int";
}

std::string VoidType::toStr()
{
    return "void";
}

std::string ConstIntType::toStr()
{
    return "const int";
}

std::string FunctionType::toStr()
{
    std::ostringstream buffer;
    buffer << returnType->toStr() << "()";
    return buffer.str();
}