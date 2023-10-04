//
// Copyright 2021 Patrick Flynn
// This file is part of the Espresso compiler.
// Espresso is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>
#include <memory>

#include <parser/Parser.hpp>

Parser::Parser(std::string input) : BaseParser(input) {
    scanner = std::make_unique<Scanner>(input);
}

bool Parser::parse() {
    Token token;
    do {
        token = scanner->getNext();
        bool code = true;
        
        switch (token.type) {
            case Public:
            case Protected:
            case Private:
            case Routine:
            case Func: {
                code = buildFunction(tree->block, token);
            } break;
            
            case Const: code = buildConst(tree->block, true); break;
            case Enum: code = buildEnum(); break;
            
            case Eof:
            case Nl: break;
            
            default: {
                syntax->addError(scanner->getLine(), "Invalid token in global scope.");
                token.print();
                code = false;
            }
        }
        
        if (!code) break;
    } while (token.type != Eof);
    
    // Check for errors, and print if so
    if (syntax->errorsPresent()) {
        syntax->printErrors();
        return false;
    }
    
    syntax->printWarnings();
    return true;
}

// Builds a statement block
bool Parser::buildBlock(std::shared_ptr<AstBlock> block, std::shared_ptr<AstNode> parent) {
    Token token = scanner->getNext();
    while (token.type != Eof) {
        bool code = true;
        bool end = false;
        
        switch (token.type) {
            case VarD: code = buildVariableDec(block); break;
            case Const: code = buildConst(block, false); break;
            
            case Id: {
                Token idToken = token;
                token = scanner->getNext();
                
                if (token.type == Assign) {
                    code = buildVariableAssign(block, idToken);
                } else if (token.type == LBracket) {
                    code = buildArrayAssign(block, idToken);
                } else if (token.type == LParen) {
                    Token varToken;
                    varToken.type = EmptyToken;
                    code = buildFunctionCallStmt(block, idToken, varToken);
                } else if (token.type == Dot) {
                    Token memberToken = scanner->getNext();
                    if (memberToken.type != Id) {
                        syntax->addError(scanner->getLine(), "Expected member name.");
                    }
                    
                    token = scanner->getNext();
                    if (token.type == LParen) {
                        code = buildFunctionCallStmt(block, memberToken, idToken);
                    }
                    // TODO: Catch others
                } else {
                    syntax->addError(scanner->getLine(), "Invalid use of identifier.");
                    token.print();
                    return false;
                }
            } break;
            
            case Return: code = buildReturn(block); break;
            
            // Handle conditionals
            // Yes, ELIF and ElSE are similar, but if you look closely, there is a subtle
            // difference (one very much needed)
            case If: code = buildConditional(block); break;
            case Elif: {
                std::shared_ptr<AstIfStmt> condParent = std::static_pointer_cast<AstIfStmt>(parent);
                code = buildConditional(condParent->false_block);
                end = true;
            } break;
            case Else: {
                std::shared_ptr<AstIfStmt> condParent = std::static_pointer_cast<AstIfStmt>(parent);
                buildBlock(condParent->false_block);
                end = true;
            } break;
            
            // Handle loops
            case While: code = buildWhile(block); break;
            //case Repeat: code = buildRepeat(block); break;
            //case For: code = buildFor(block); break;
            //case ForAll: code = buildForAll(block); break;
            
            case Break: code = buildLoopCtrl(block, true); break;
            case Continue: code = buildLoopCtrl(block, false); break;
            
            // Handle the END keyword
            case End: {
                end = true;
            } break;
            
            case Nl: break;
            
            default: {
                syntax->addError(scanner->getLine(), "Invalid token in expression.");
                token.print();
                return false;
            }
        }
        
        if (end) break;
        if (!code) return false;
        token = scanner->getNext();
    }
    
    return true;
}

// Builds an expression
std::shared_ptr<AstExpression> Parser::buildExpression(std::shared_ptr<AstBlock> block, std::shared_ptr<AstDataType> currentType,
                        TokenType stopToken, TokenType separateToken,
                        bool isConst) {
    std::stack<std::shared_ptr<AstExpression>> output;
    std::stack<std::shared_ptr<AstExpression>> opStack;
    
    auto varType = currentType;
    bool lastWasOp = true;

    Token token = scanner->getNext();
    while (token.type != Eof && token.type != stopToken) {
        if (token.type == separateToken && output.size() > 0) {
            std::shared_ptr<AstExpression> expr = output.top();
            output.pop();
            return expr;
            
            /*if (stmt == nullptr) {
                if ((*dest)->getType() == AstType::FuncCallExpr) {
                    AstFuncCallExpr *fc = static_cast<AstFuncCallExpr *>(*dest);
                    fc->addArgument(expr);
                } else {
                    *dest = expr;
                }
            } else {
                stmt->addExpression(expr);
            }
            continue;*/
        }
    
        switch (token.type) {
            case True: {
                lastWasOp = false;
                output.push(std::make_shared<AstI32>(1));
            } break;
            
            case False: {
                lastWasOp = false;
                output.push(std::make_shared<AstI32>(0));
            } break;
            
            case CharL: {
                lastWasOp = false;
                std::shared_ptr<AstChar> c = std::make_shared<AstChar>(token.i8_val);
                output.push(c);
            } break;
            
            case Int32: {
                lastWasOp = false;
                std::shared_ptr<AstI32> i32 = std::make_shared<AstI32>(token.i32_val);
                output.push(i32);
            } break;
            
            case String: {
                lastWasOp = false;
                std::shared_ptr<AstString> str = std::make_shared<AstString>(token.id_val);
                output.push(str);
            } break;
            
            case Id: {
                lastWasOp = false;
                
                if (isConst) {
                    syntax->addError(scanner->getLine(), "Invalid constant value.");
                    return nullptr;
                }
            
                std::string name = token.id_val;
                if (varType->type == V_AstType::Void) {
                    varType = block->symbolTable[name];
                }
                
                token = scanner->getNext();
                if (token.type == LBracket) {
                    /*AstExpression *index = nullptr;
                    buildExpression(nullptr, DataType::Int32, RBracket, EmptyToken, &index);
                    
                    AstArrayAccess *acc = new AstArrayAccess(name);
                    acc->setIndex(index);
                    output.push(acc);*/
                } else if (token.type == LParen) {
                    /*AstFuncCallExpr *fc = new AstFuncCallExpr(name);
                    AstExpression *fcExpr = fc;
                    buildExpression(nullptr, varType, RParen, Comma, &fcExpr);
                    
                    output.push(fc);*/
                } else if (token.type == Scope) {
                    /*if (enums.find(name) == enums.end()) {
                        syntax->addError(scanner->getLine(), "Unknown enum.");
                        return false;
                    }
                    
                    token = scanner->getNext();
                    if (token.type != Id) {
                        syntax->addError(scanner->getLine(), "Expected identifier.");
                        return false;
                    }
                    
                    EnumDec dec = enums[name];
                    AstExpression *val = dec.values[token.id_val];
                    output.push(val);*/
                } else {
                    int constVal = block->isConstant(name);
                    if (constVal > 0) {
                        if (constVal == 1) {
                            std::shared_ptr<AstExpression> expr = block->globalConsts[name].second;
                            output.push(expr);
                        } else if (constVal == 2) {
                            std::shared_ptr<AstExpression> expr = block->localConsts[name].second;
                            output.push(expr);
                        }
                    } else {
                        std::shared_ptr<AstID> id = std::make_shared<AstID>(name);
                        output.push(id);
                    }
                    
                    scanner->rewind(token);
                }
            } break;
            
            case Sizeof: {
                /*lastWasOp = false;
                
                if (isConst) {
                    syntax->addError(scanner->getLine(), "Invalid constant value.");
                    return false;
                }
                
                std::string name = token.id_val;
                
                Token token1 = scanner->getNext();
                Token token2 = scanner->getNext();
                Token token3 = scanner->getNext();
                
                if (token1.type != LParen || token2.type != Id || token3.type != RParen) {
                    syntax->addError(scanner->getLine(), "Invalid token in sizeof.");
                    token.print();
                    return false;
                }
                
                AstID *id = new AstID(token2.id_val);
                AstSizeof *size = new AstSizeof(id);
                output.push(size);*/
            } break;
            
            case Plus: 
            case Minus:
            case And:
            case Or:
            case Xor:
            case Lsh:
            case Rsh: {
                if (opStack.size() > 0) {
                    V_AstType type = opStack.top()->type;
                    if (type == V_AstType::Mul || type == V_AstType::Div) {
                        std::shared_ptr<AstExpression> rval = checkExpression(output.top(), varType);
                        output.pop();
                        
                        std::shared_ptr<AstExpression> lval = checkExpression(output.top(), varType);
                        output.pop();
                        
                        std::shared_ptr<AstBinaryOp> op = std::static_pointer_cast<AstBinaryOp>(opStack.top());
                        opStack.pop();
                        
                        op->lval = lval;
                        op->rval = rval;
                        output.push(op);
                    }
                }
                
                if (token.type == Plus) {
                    std::shared_ptr<AstAddOp> add = std::make_shared<AstAddOp>();
                    opStack.push(add);
                } else if (token.type == And) {
                    opStack.push(std::make_shared<AstAndOp>());
                } else if (token.type == Or) {
                    opStack.push(std::make_shared<AstOrOp>());
                } else if (token.type == Xor) {
                    opStack.push(std::make_shared<AstXorOp>());
                /*} else if (token.type == Lsh) {
                    opStack.push(std::make_shared<AstLshOp>());
                } else if (token.type == Rsh) {
                    opStack.push(std::make_shared<AstRshOp>());*/
                } else {
                    if (lastWasOp) {
                        opStack.push(std::make_shared<AstNegOp>());
                    } else {
                        std::shared_ptr<AstSubOp> sub = std::make_shared<AstSubOp>();
                        opStack.push(sub);
                    }
                }
                
                lastWasOp = true;
            } break;
            
            case Mul: {
                lastWasOp = true;
                std::shared_ptr<AstMulOp> mul = std::make_shared<AstMulOp>();
                opStack.push(mul);
            } break;
            
            case Div: {
                lastWasOp = true;
                std::shared_ptr<AstDivOp> div = std::make_shared<AstDivOp>();
                opStack.push(div);
            } break;
            
            case Mod: {
                lastWasOp = true;
                std::shared_ptr<AstModOp> rem = std::make_shared<AstModOp>();
                opStack.push(rem);
            } break;
            
            case EQ: opStack.push(std::make_shared<AstEQOp>()); lastWasOp = true; break;
            case NEQ: opStack.push(std::make_shared<AstNEQOp>()); lastWasOp = true; break;
            case GT: opStack.push(std::make_shared<AstGTOp>()); lastWasOp = true; break;
            case LT: opStack.push(std::make_shared<AstLTOp>()); lastWasOp = true; break;
            case GTE: opStack.push(std::make_shared<AstGTEOp>()); lastWasOp = true; break;
            case LTE: opStack.push(std::make_shared<AstLTEOp>()); lastWasOp = true; break;
            
            case Step: {
                /*lastWasOp = false;       
                
                if (stmt->getType() != AstType::For) {
                    syntax->addError(scanner->getLine(), "Step is only valid with for loops");
                    return false;
                }
                
                token = scanner->getNext();
                if (token.type != Int32) {
                    syntax->addError(scanner->getLine(), "Expected integer literal with \"step\"");
                    return false;
                }
                
                AstForStmt *forStmt = static_cast<AstForStmt *>(stmt);
                forStmt->setStep(token.i32_val);*/
            } break;
            
            default: {}
        }
        
        if (!lastWasOp && opStack.size() > 0) {
            if (opStack.top()->type == V_AstType::Neg) {
                std::shared_ptr<AstExpression> val = checkExpression(output.top(), varType);
                output.pop();
                
                std::shared_ptr<AstNegOp> op = std::static_pointer_cast<AstNegOp>(opStack.top());
                opStack.pop();
                op->value = val;
                output.push(op);
            }
        }
        
        token = scanner->getNext();
    }
    
    // Build the expression
    while (opStack.size() > 0) {
        std::shared_ptr<AstExpression> rval = checkExpression(output.top(), varType);
        output.pop();
        
        std::shared_ptr<AstExpression> lval = checkExpression(output.top(), varType);
        output.pop();
        
        std::shared_ptr<AstBinaryOp> op = std::static_pointer_cast<AstBinaryOp>(opStack.top());
        opStack.pop();
        
        op->lval = lval;
        op->rval = rval;
        output.push(op);
    }
    
    // Add the expressions back
    if (output.size() == 0) {
        return nullptr;
    }
    
    // Type check the top
    std::shared_ptr<AstExpression> expr = checkExpression(output.top(), varType);
    return expr;
    
    /*if (stmt == nullptr) {
        if ((*dest) == nullptr) {
            *dest = expr;
        } else if ((*dest)->getType() == AstType::FuncCallExpr) {
            AstFuncCallExpr *fc = static_cast<AstFuncCallExpr *>(*dest);
            fc->addArgument(expr);
        } else {
            *dest = expr;
        }
    } else {
        stmt->addExpression(expr);
    }
    
    return true;*/
}

// The debug function for the scanner
void Parser::debugScanner() {
    std::cout << "Debugging scanner..." << std::endl;
    
    Token t;
    do {
        t = scanner->getNext();
        t.print();
    } while (t.type != Eof);
}

