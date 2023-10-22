//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#include <iostream>
#include <memory>

#include <parser/Parser.hpp>
#include <ast/ast.hpp>
#include <ast/ast_builder.cpp>

// Returns the function arguments
bool Parser::getFunctionArgs(std::vector<Var> &args, std::shared_ptr<AstBlock> block) {
    Token token = scanner->getNext();
    if (token.type == LParen) {
        token = scanner->getNext();
        while (token.type != Eof && token.type != RParen) {
            Token t1 = token;
            Token t2 = scanner->getNext();
            Token t3 = scanner->getNext();
            Var v;
            
            if (t1.type != Id) {
                syntax->addError(scanner->getLine(), "Invalid function argument: Expected name.");
                t1.print();
                return false;
            }
            
            if (t2.type != Colon) {
                syntax->addError(scanner->getLine(), "Invalid function argument: Expected \':\'.");
                return false;
            }
            
            switch (t3.type) {
                case Bool: v.type = AstBuilder::buildVoidType(); break;
                case Char: v.type = AstBuilder::buildCharType(); break;
                case Byte: v.type = AstBuilder::buildInt8Type(); break;
                case UByte: v.type = AstBuilder::buildInt8Type(true); break;
                case Short: v.type = AstBuilder::buildInt16Type(); break;
                case UShort: v.type = AstBuilder::buildInt16Type(true); break;
                case Int: v.type = AstBuilder::buildInt32Type(); break;
                case UInt: v.type = AstBuilder::buildInt32Type(true); break;
                case Int64: v.type = AstBuilder::buildInt64Type(); break;
                case UInt64: v.type = AstBuilder::buildInt64Type(true); break;
                case Str: v.type = AstBuilder::buildStringType(); break;
                
                default: {
                    syntax->addError(scanner->getLine(), "Invalid function argument: Unknown type.");
                    return false;
                }
            }
            
            v.name = t1.id_val;
            
            token = scanner->getNext();
            if (token.type == Comma) {
                token = scanner->getNext();
            } else if (token.type == LBracket) {
                Token token1 = scanner->getNext();
                Token token2 = scanner->getNext();
                
                if (token1.type != RBracket) {
                    syntax->addError(scanner->getLine(), "Invalid type syntax.");
                    return false;
                }
                
                if (token2.type == Comma) token = scanner->getNext();
                else token = token2;
                
                v.type = AstBuilder::buildPointerType(v.type);
            }
            
            args.push_back(v);
            block->symbolTable[v.name] = v.type;
        }
    } else {
        scanner->rewind(token);
    }
    
    return true;
}

// Builds a function
bool Parser::buildFunction(std::shared_ptr<AstBlock> block, Token startToken) {
    bool isRoutine = false;
    Attr visible = Attr::Public;
    bool attrMod = false;
    
    // Check the tokens
    switch (startToken.type) {
        case Routine: isRoutine = true; break;
        
        case Public: visible = Attr::Public; attrMod = true; break;
        case Protected: visible = Attr::Protected; attrMod = true; break;
        case Private: visible = Attr::Private; attrMod = true; break;
        
        default: {}
    }
    
    Token token;
    
    if (attrMod) {
        token = scanner->getNext();
        
        if (token.type == Routine) {
            isRoutine = true;
        } else if (token.type != Func) {
            syntax->addError(scanner->getLine(), "Expected \"func\" or \"routine\".");
            return false;
        }
    }

    // Make sure we have a function name
    token = scanner->getNext();
    std::string funcName = token.id_val;
    
    if (token.type != Id) {
        syntax->addError(scanner->getLine(), "Expected function name.");
        return false;
    }
    
    // Get arguments
    std::vector<Var> args;
    if (!getFunctionArgs(args, block)) return false;

    // Check to see if there's any return type
    token = scanner->getNext();
    std::shared_ptr<AstDataType> data_type = AstBuilder::buildVoidType();
    
    if (token.type == Arrow) {
        token = scanner->getNext();
        switch (token.type) {
            case Int: data_type = AstBuilder::buildInt32Type(); break;
            default: {}
        }
    }
    
    // Do syntax error check
    if (token.type != Is) {
        syntax->addError(scanner->getLine(), "Expected \'is\' keyword.");
        return false;
    }

    // Create the function object
    std::shared_ptr<AstFunction> func = std::make_shared<AstFunction>(funcName);
    func->data_type = data_type;
    func->args = args;
    func->routine = isRoutine;
    func->attr = visible;
    tree->block->addStatement(func);
    
    // Build the body
    if (!buildBlock(func->block)) return false;
    
    // Make sure we end with a return statement
    V_AstType lastType = func->block->block.back()->type;
    if (lastType == V_AstType::Return) {
        std::shared_ptr<AstStatement> ret = func->block->block.back();
        if (func->data_type->type == V_AstType::Void && ret->hasExpression()) {
            syntax->addError(scanner->getLine(), "Cannot return from void function.");
            return false;
        } else if (!ret->hasExpression()) {
            syntax->addError(scanner->getLine(), "Expected return value.");
            return false;
        }
    } else {
        if (func->data_type->type == V_AstType::Void) {
            std::shared_ptr<AstReturnStmt> ret = std::make_shared<AstReturnStmt>();
            func->addStatement(ret);
        } else {
            syntax->addError(scanner->getLine(), "Expected return statement.");
            return false;
        }
    }
    
    return true;
}

// Builds a function call
bool Parser::buildFunctionCallStmt(std::shared_ptr<AstBlock> block, Token idToken, Token varToken) {
    std::shared_ptr<AstFuncCallStmt> fc = std::make_shared<AstFuncCallStmt>(idToken.id_val);
    block->addStatement(fc);
    
    if (varToken.type == Id) {
        fc->object_name = varToken.id_val;
    }
    
    std::shared_ptr<AstExpression> expr = buildExpression(block, nullptr, RParen, Comma);
    if (expr) fc->expression = expr;
    
    Token token = scanner->getNext();
    if (token.type != SemiColon) {
        syntax->addError(scanner->getLine(), "Expected \';\'.");
        token.print();
        return false;
    }
    
    return true;
}

// Builds a return statement
bool Parser::buildReturn(std::shared_ptr<AstBlock> block) {
    std::shared_ptr<AstReturnStmt> stmt = std::make_shared<AstReturnStmt>();
    block->addStatement(stmt);
    
    stmt->expression = buildExpression(block, nullptr);
    if (!stmt->expression) return false;
    
    return true;
}

