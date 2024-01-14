%code top{
    #include <iostream>
    #include <assert.h>
    #include <functional>
    #include "parser.h"
    #include <cstring>
    extern Ast ast;
    int yylex();
    int yyerror( char const * );
    Type* vtype = nullptr;
    std::string name{};
    bool issysinit = false;
    int isCalculable (ExprNode* node1,ExprNode* node2=nullptr)
    {  
        if(node1->hasInt())
        {
            if(node2==nullptr)
            {
                return 1;
            }
            else if(node2->hasInt())
            {
                return 1;
            }
            else if(node2->hasFloat())
            {
                return 2;
            }
            return 0;
        }
        if(node1->hasFloat())
        {
            if(node2==nullptr)
            {
                return 2;
            }
            else if(node2->hasInt())
            {
                return 3;
            }
            else if(node2->hasFloat())
            {
                return 4;
            }
            return 0;
        }
        return 0;
    }
}

%code requires {
    #include "Ast.h"
    #include "SymbolTable.h"
    #include "Type.h"
}

%union {
    int itype;
    char* strtype;
    StmtNode* stmttype;
    ExprNode* exprtype;
    Type* type;
    VarIds* vistype;
    VarId* vitype;
    DeclStmt* dectype;
    TDeclStmt* tdectype;
    ArrayList* altype;
    ArrayNode* artype;
    InitVal* ivtype;
    InitVals* vivtype;
    SeqNode* seqtype;
    FArrayList* faltype;
    FArrayNode* fartype;
    FuncParam* fptype;
    FuncParams* fpstype;
    CallParams* cptype;
}

%start Program
%token <strtype> ID FLT INTEGER OCTAL HEX
%token IF ELSE WHILE CONTINUE BREAK
%token INT VOID CONST FLOAT
%token LPAREN RPAREN LBRACE RBRACE SEMICOLON COMMA LBRACKET RBRACKET
%token ADD SUB MUL DIV MOD LESS ASSIGN GREATER NGREATER NLESS NEQUAL EQUAL NOT LOR LAND 
%token RETURN

%nterm <cptype> CallParams
%nterm <vistype> VarIds ConstVarIds
%nterm <fptype> FuncParam
%nterm <fpstype> FuncParams
%nterm <vitype> VarId TempVarId ConstVarId
%nterm <ivtype> InitVal ConstInitVal
%nterm <vivtype> InitVals ConstInitVals
%nterm <altype> ArrayList 
%nterm <artype> ArrayNode
%nterm <faltype> FArrayList
%nterm <fartype> FArrayNode
%nterm <seqtype> Stmts
%nterm <dectype> ConstDeclStmt
%nterm <tdectype> DeclStmt TempDecl
%nterm <stmttype> Stmt AssignStmt BlockStmt IfStmt ReturnStmt WhileStmt BreakStmt ContinueStmt PlainStmt
%nterm <exprtype> Exp AddExp Cond LOrExp PrimaryExp LVal RelExp LAndExp MulExp EqExp ConstExp UnaryExp
%nterm <type> BType

%precedence THEN
%precedence ELSE
%%
Program
    : Stmts {
        ast.setRoot($1);
    }
    ;
Stmts
    : Stmt {$$ = new SeqNode($1);}
    | Stmts Stmt{
        $1->emplace_back($2);
        $$ = $1;
    }
    ;
Stmt
    : AssignStmt {$$=$1;}
    | BlockStmt {$$=$1;}
    | IfStmt {$$=$1;}
    | WhileStmt {$$=$1;}
    | ReturnStmt {$$=$1;}
    | DeclStmt {$$=$1;}
    | ConstDeclStmt {$$=$1;}
    | BreakStmt {$$=$1;}
    | ContinueStmt {$$=$1;}
    | PlainStmt {$$=$1;}
    ;
PlainStmt
    : SEMICOLON {$$ = new PlainStmt();}
    | Exp SEMICOLON {$$ = new PlainStmt($1);}
    ;
LVal
    : ID FArrayList {
        if(!issysinit)
        {
            initsys();
            issysinit = true;
        }
        SymbolEntry *se;
        se = identifiers->lookup($1);
        if(se == nullptr)
        {
            fprintf(stderr, "identifier \"%s\" is undefined\n", (char*)$1);
            delete [](char*)$1;
            assert(se != nullptr);
        }
        $$ = new Id(se,$2);
        if($2==nullptr||$2->empty())
        {
            if(dynamic_cast<IdentifierSymbolEntry*>(se)->hasValue())
            {
                if(dynamic_cast<IdentifierSymbolEntry*>(se)->hasInt())
                {
                    $$->changeInt(std::make_optional<int>(dynamic_cast<IdentifierSymbolEntry*>(se)->getInt()));
                }
                else
                {
                    $$->changeFloat(std::make_optional<float>(dynamic_cast<IdentifierSymbolEntry*>(se)->getFloat()));
                }
            }
        }
        else
        {
            if(dynamic_cast<IdentifierSymbolEntry*>(se)->hasArray())
            {
                std::vector<int> d{};
                bool doable = true;
                for(auto& node : $2->getVector())
                {
                    if(node->getExprNode()->hasInt())
                        d.emplace_back(node->getExprNode()->getIntop().value());
                    else
                    {
                        doable = false;
                        break;
                    }
                }
                if(doable)
                {
                    if(dynamic_cast<IdentifierSymbolEntry*>(se)->hasIntArray())
                    {
                        $$->changeInt(std::make_optional<int>(dynamic_cast<IdentifierSymbolEntry*>(se)->getInt(d)));
                    }
                    else
                    {
                        $$->changeFloat(std::make_optional<float>(dynamic_cast<IdentifierSymbolEntry*>(se)->getFloat(d)));
                    }
                }
            }
        }
        delete []$1;
    }
    ;
ContinueStmt
    : CONTINUE SEMICOLON{
        $$ = new ContinueStmt();
    }
    ;
BreakStmt
    : BREAK SEMICOLON{
        $$ = new BreakStmt();
    }
    ;
AssignStmt
    :
    LVal ASSIGN Exp SEMICOLON {
        $$ = new AssignStmt($1, $3);
    }
    ;
BlockStmt
    :   LBRACE 
        {identifiers = new SymbolTable(identifiers);} 
        Stmts RBRACE 
        {
            $$ = new CompoundStmt($3);
            SymbolTable *top = identifiers;
            identifiers = identifiers->getPrev();
            delete top;
        }
    |   LBRACE RBRACE 
        {
            $$ = new CompoundStmt(nullptr);
        }
    ;
IfStmt
    : IF LPAREN Cond RPAREN Stmt %prec THEN {
        $$ = new IfStmt($3, $5);
    }
    | IF LPAREN Cond RPAREN Stmt ELSE Stmt {
        $$ = new IfElseStmt($3, $5, $7);
    }
    ;
ReturnStmt
    :
    RETURN Exp SEMICOLON{
        $$ = new ReturnStmt($2);
    }
    | RETURN SEMICOLON{
        $$ = new ReturnStmt(nullptr);
    }
    ;
Cond
    :
    LOrExp {$$ = $1;}
    ;
PrimaryExp
    :
    LPAREN Exp RPAREN {
        $$ = $2;
    }
    | LVal {
        $$ = $1;
    }
    | INTEGER {
        std::string temp = $1;
        SymbolEntry *se = new IntSymbolEntry(TypeSystem::intType,std::stol(temp, 0, 10));
        $$ = new Constant(se);
        $$->changeInt(std::make_optional<int>(std::stol(temp, 0, 10)));
    }
    | OCTAL {
        std::string temp = $1;
        SymbolEntry *se = new IntSymbolEntry(TypeSystem::intType,std::stol(temp, 0, 8));
        $$ = new Constant(se);
        $$->changeInt(std::make_optional<int>(std::stol(temp, 0, 8)));
    }
    | HEX {
        std::string temp = $1;
        SymbolEntry *se = new IntSymbolEntry(TypeSystem::intType,std::stol(temp.substr(2), 0, 16));
        $$ = new Constant(se);
        $$->changeInt(std::make_optional<int>(std::stol(temp, 0, 16)));
    }
    | FLT {
        std::string temp = $1;
        SymbolEntry *se = new FloatSymbolEntry(TypeSystem::floatType,std::stod(temp));
        $$ = new Constant(se);
        $$->changeFloat(std::make_optional<float>(std::stof(temp)));
    }
    ;
UnaryExp
    :
    PrimaryExp {
        $$ = $1;
    }
    | ID LPAREN RPAREN {
        if(!issysinit)
        {
            initsys();
            issysinit = true;
        }
        if(issysruntime($1))
        {
            char additionalChar = '_';
            size_t originalLen = strlen($1);
            char* newStr = new char[originalLen + 2];
            newStr[0] = additionalChar;
            strcpy(newStr + 1, $1);
            newStr[originalLen + 1] = '\0';
            delete[] $1;
            $1 = newStr;
        }
        auto tse = identifiers->lookup($1);
        assert(tse != nullptr);
        SymbolEntry *se = new TemporarySymbolEntry(dynamic_cast<FunctionType*>(tse->getType())->getRetType(), SymbolTable::getLabel());//todo
        $$ = new UnaryExpr(se,tse,nullptr);
        delete[] $1;
    }
    | ID LPAREN CallParams RPAREN {
        if(!issysinit)
        {
            initsys();
            issysinit = true;
        }
        if(issysruntime($1))
        {
            char additionalChar = '_';
            size_t originalLen = strlen($1);
            char* newStr = new char[originalLen + 2];
            newStr[0] = additionalChar;
            strcpy(newStr + 1, $1);
            newStr[originalLen + 1] = '\0';
            delete[] $1;
            $1 = newStr;
        }
        auto tse = identifiers->lookup($1);
        assert(tse != nullptr);
        SymbolEntry *se = new TemporarySymbolEntry(dynamic_cast<FunctionType*>(tse->getType())->getRetType(), SymbolTable::getLabel());//todo
        $$ = new UnaryExpr(se,tse,$3);
        delete[] $1;
    }
    | ADD UnaryExp {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());//todo
        $$ = new UnaryExpr(se, UnaryExpr::ADD, $2);
        int cond = isCalculable($2);
        if(cond)
        {
            if(cond==1)
            {
                $$->changeInt(std::make_optional<int>($2->getIntop().value()));
            }
            else
            {
                $$->changeFloat(std::make_optional<float>($2->getFloat()));
            }
        }
    }
    | SUB UnaryExp {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());//todo
        $$ = new UnaryExpr(se, UnaryExpr::SUB, $2);
        int cond = isCalculable($2);
        if(cond)
        {
            if(cond==1)
            {
                $$->changeInt(std::make_optional<int>(-$2->getIntop().value()));
            }
            else
            {
                $$->changeFloat(std::make_optional<float>(-$2->getFloat()));
            }
        }
    }
    | NOT UnaryExp {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new UnaryExpr(se, UnaryExpr::NOT, $2);
    }
    ;
MulExp
    :
    UnaryExp {$$ = $1;}
    |
    MulExp MUL UnaryExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());//todo
        $$ = new BinaryExpr(se, BinaryExpr::MUL, $1, $3);
        int cond = isCalculable($1,$3);
        if(cond)
        {
            if(cond==1)
            {
                $$->changeInt(std::make_optional<int>($1->getIntop().value()*$3->getIntop().value()));
            }
            else
            {
                $$->changeFloat(std::make_optional<float>($1->getFloat()*$3->getFloat()));
            }
        }
    }
    |
    MulExp DIV UnaryExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());//todo
        $$ = new BinaryExpr(se, BinaryExpr::DIV, $1, $3);
        int cond = isCalculable($1,$3);
        if(cond)
        {
            if(cond==1)
            {
                $$->changeInt(std::make_optional<int>($1->getIntop().value()/$3->getIntop().value()));
            }
            else
            {
                $$->changeFloat(std::make_optional<float>($1->getFloat()/$3->getFloat()));
            }
        }
    }
    |
    MulExp MOD UnaryExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());//todo
        $$ = new BinaryExpr(se, BinaryExpr::MOD, $1, $3);
        int cond = isCalculable($1,$3);
        if(cond==1)
        {
            $$->changeInt(std::make_optional<int>($1->getIntop().value()%$3->getIntop().value()));
        }
    }
    ;
AddExp
    :
    MulExp {$$ = $1;}
    |
    AddExp ADD MulExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());//todo
        $$ = new BinaryExpr(se, BinaryExpr::ADD, $1, $3);
        int cond = isCalculable($1,$3);
        if(cond)
        {
            if(cond==1)
            {
                $$->changeInt(std::make_optional<int>($1->getIntop().value()+$3->getIntop().value()));
            }
            else
            {
                $$->changeFloat(std::make_optional<float>($1->getFloat()+$3->getFloat()));
            }
        }
    }
    |
    AddExp SUB MulExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());//todo
        $$ = new BinaryExpr(se, BinaryExpr::SUB, $1, $3);
        int cond = isCalculable($1,$3);
        if(cond)
        {
            if(cond==1)
            {
                $$->changeInt(std::make_optional<int>($1->getIntop().value()-$3->getIntop().value()));
            }
            else
            {
                $$->changeFloat(std::make_optional<float>($1->getFloat()-$3->getFloat()));
            }
        }
    }
    ;
RelExp
    :
    AddExp {$$ = $1;}
    |
    RelExp LESS AddExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::LESS, $1, $3);
    }
    |
    RelExp GREATER AddExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::GREATER, $1, $3);
    }
    |
    RelExp NGREATER AddExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::NGREATER, $1, $3);
    }
    |
    RelExp NLESS AddExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::NLESS, $1, $3);
    }
    ;
EqExp
    :
    RelExp {$$ = $1;}
    |
    EqExp EQUAL RelExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::EQUAL, $1, $3);
    }
    |
    EqExp NEQUAL RelExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::NEQUAL, $1, $3);
    }
    ;
LAndExp
    :
    EqExp {$$ = $1;}
    |
    LAndExp LAND EqExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::AND, $1, $3);
    }
    ;
LOrExp
    :
    LAndExp {$$ = $1;}
    |
    LOrExp LOR LAndExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::OR, $1, $3);
    }
    ;
BType
    : INT {
        $$ = TypeSystem::intType;
    }
    | FLOAT {
        $$ = TypeSystem::floatType;
    }
    ;
ConstDeclStmt
    :
    CONST BType {
        vtype = $2;
    }
    ConstVarIds SEMICOLON{
        $$ = new DeclStmt($4);
    }
ConstVarIds
    :
    ConstVarId{
        $$ = new VarIds($1);
    }
    | ConstVarIds COMMA ConstVarId{
        $1->push_back($3);
        $$ = $1;
    }
    ;
ConstVarId
    :ID {
        SymbolEntry *se;
        se = new IdentifierSymbolEntry(vtype, $1, identifiers->getLevel());
        identifiers->install($1, se);
    }
    ArrayList ASSIGN ConstInitVal 
    {
        auto se = identifiers->lookup($1);
        assert(se!=nullptr);
        if(!($3->getVector().empty()))
        {
            std::vector<int> d{};
            for(auto& node : $3->getVector())
            {
                d.emplace_back(node->getExprNode()->getIntop().value());
            }
            se->setType(new ArrayType(se->getType(),d));
            bool isint = se->getType()->isInt();
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
            initarr($5,0);
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
            if(isint)
            {
                dynamic_cast<IdentifierSymbolEntry*>(se)->changeIntArray(std::make_optional<std::vector<int>>(initiallist));
            }
            else
            {
                dynamic_cast<IdentifierSymbolEntry*>(se)->changeFloatArray(std::make_optional<std::vector<float>>(finitiallist));
            }
        }
        else
        {
            assert($5->getExprNode()!=nullptr&&$5->getExprNode()->hasValue());
            if(vtype->isInt())
            {
                dynamic_cast<IdentifierSymbolEntry*>(se)->changeInt(std::make_optional<int>($5->getExprNode()->getFloat()));
            }
            else
            {
                dynamic_cast<IdentifierSymbolEntry*>(se)->changeFloat(std::make_optional<float>($5->getExprNode()->getFloat()));
            }
        }
        $$ = new VarId(se,$3,$5);
        $$->changeinit(true);
        $$->changeconst(true);
        delete []$1;
    }
    ;
ConstInitVal
    : ConstExp{
        $$ = new InitVal($1);
    }
    | LBRACE ConstInitVals RBRACE{
        $$ = new InitVal($2->vals);
    }
    | LBRACE RBRACE{
        $$ = new InitVal();
    }
    ;
ConstInitVals
    : ConstInitVal {
        $$ = new InitVals();
        $$->vals.push_back($1);
    }
    | ConstInitVals COMMA ConstInitVal{
        $1->vals.push_back($3);
        $$ = $1;
    }
    ;
CallParams
    :
    Exp {
        $$ = new CallParams($1);
    } 
    | CallParams COMMA Exp{
        $1->push_back($3);
        $$ = $1;
    }
    ;
DeclStmt
    :
    BType {
        vtype = $1;
    }
    TempDecl {
        $$ = $3;
    }
    | VOID{
        vtype = TypeSystem::voidType;
    }
    TempDecl {
        $$ = $3;
    }
    ;
TempDecl
    :
    VarIds SEMICOLON{
        $$ = new TDeclStmt(new DeclStmt($1));
    }
    | ID 
    {
        Type *funcType;
        funcType = new FunctionType(vtype,{});
        SymbolEntry *se = new IdentifierSymbolEntry(funcType, $1, identifiers->getLevel());
        identifiers->install($1, se);
        identifiers = new SymbolTable(identifiers);
    }
    LPAREN FuncParams RPAREN BlockStmt
    {
        SymbolEntry *se;
        se = identifiers->lookup($1);
        std::vector<Type*> t{};
        for(const auto& node : $4->getVector())
        {
            t.emplace_back(node->getType());
        }
        dynamic_cast<FunctionType*>(se->getType())->changeParams(t);
        assert(se != nullptr);
        $$ = new TDeclStmt(new FunctionDef(se, $6,$4));
        SymbolTable *top = identifiers;
        identifiers = identifiers->getPrev(); 
        delete top;
        delete []$1;

    }
VarIds
    :
    VarId{
        $$ = new VarIds($1);
    }
    | VarIds COMMA VarId{
        $1->push_back($3);
        $$ = $1;
    }
    ;
    ;
FuncParams
    :
    %empty {
        $$ = new FuncParams();
    } 
    | FuncParam {
        $$ = new FuncParams($1);
    } 
    | FuncParams COMMA FuncParam{
        $1->push_back($3);
        $$ = $1;
    }
    ;
FuncParam
    : BType ID {
        SymbolEntry *se;
        se = new IdentifierSymbolEntry($1, $2, identifiers->getLevel());
        identifiers->install($2, se);
        $$ = new FuncParam(se);
    }
    | BType ID LBRACKET RBRACKET FArrayList{
        SymbolEntry *se;
        if(!($5->getVector().empty()))
        {
            std::vector<int> d{};
            for(auto& node : $5->getVector())
            {
                d.emplace_back(node->getExprNode()->getIntop().value());
            }
            se = new IdentifierSymbolEntry((new PointerType(new ArrayType($1,d))), $2, identifiers->getLevel());
        }
        else
        {
            se = new IdentifierSymbolEntry((new PointerType($1)), $2, identifiers->getLevel());
        }
        identifiers->install($2, se);
        $$ = new FuncParam(se,$5);
        $$->changearray(true);
    }
    ;
FArrayList
    : %empty { 
        $$ = new FArrayList();
    }
    | FArrayList FArrayNode   {
        $1->push_back($2);
        $$ = $1;
    }
    ;
FArrayNode
    : LBRACKET Exp RBRACKET {
        $$ = new FArrayNode($2);
    }
    ;
VarId
    : ID {
        SymbolEntry *se;
        se = new IdentifierSymbolEntry(vtype, $1, identifiers->getLevel());
        identifiers->install($1, se);
        name = $1;
        delete []$1;
    }
    TempVarId
    {
        $$ = $3;
    }
    ;
TempVarId:
    ArrayList
    {
        auto se = identifiers->lookup(name);
        assert(se!=nullptr);
        if(!($1->getVector().empty()))
        {
            std::vector<int> d{};
            for(auto& node : $1->getVector())
            {
                d.emplace_back(node->getExprNode()->getIntop().value());
            }
            se->setType(new ArrayType(se->getType(),d));
        }
        $$ = new VarId(se,$1);
    }
    | ArrayList ASSIGN InitVal 
    {
        auto se = identifiers->lookup(name);
        assert(se!=nullptr);
        if(!($1->getVector().empty()))
        {
            std::vector<int> d{};
            for(auto& node : $1->getVector())
            {
                d.emplace_back(node->getExprNode()->getIntop().value());
            }
            se->setType(new ArrayType(se->getType(),d));
        }
        $$ = new VarId(se,$1,$3);
        $$->changeinit(true);
    }
ArrayList
    : %empty { 
        $$ = new ArrayList();
    }
    | ArrayList ArrayNode   {
        $1->push_back($2);
        $$ = $1;
    }
    ;
ArrayNode
    : LBRACKET ConstExp RBRACKET {
        $$ = new ArrayNode($2);
    }
    ;
Exp
    :
    AddExp {
        /*
        if($1->hasValue())
        {
            if($1->hasInt())
            {
                SymbolEntry *se = new IntSymbolEntry(TypeSystem::intType,$1->getInt());
                $$ = new Constant(se);
                $$->changeInt(std::make_optional<int>($1->getInt()));
            }
            else
            {
                SymbolEntry *se = new FloatSymbolEntry(TypeSystem::floatType,$1->getFloat());
                $$ = new Constant(se);
                $$->changeFloat(std::make_optional<int>($1->getFloat()));
            }
        }
        */
        $$ = $1;
    }
    ;
ConstExp
    : AddExp {
        assert($1->hasValue());
        $$ = $1;
    }
    ;
InitVal
    : Exp{
        $$ = new InitVal($1);
    }
    | LBRACE InitVals RBRACE{
        $$ = new InitVal($2->vals);
    }
    | LBRACE RBRACE{
        $$ = new InitVal();
    }
    ;
InitVals
    : InitVal {
        $$ = new InitVals();
        $$->vals.push_back($1);
    }
    | InitVals COMMA InitVal{
        $1->vals.push_back($3);
        $$ = $1;
    }
    ;
WhileStmt
    : WHILE LPAREN Cond RPAREN Stmt {
        $$ = new WhileStmt($3, $5);
    }
    ;
%%

int yyerror(char const* message)
{
    std::cerr<<message<<std::endl;
    return -1;
}