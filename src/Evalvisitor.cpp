#include "Evalvisitor.h"
#include <algorithm>

// Value comparison operators
bool Value::operator==(const Value& other) const {
    // Try to convert to same type for comparison
    if (type_ == other.type_) {
        switch (type_) {
            case NONE: return true;
            case BOOL: return bool_val_ == other.bool_val_;
            case INT: return int_val_ == other.int_val_;
            case FLOAT: return float_val_ == other.float_val_;
            case STRING: return string_val_ == other.string_val_;
            case TUPLE: return tuple_val_ == other.tuple_val_;
        }
    }

    // Try implicit conversion (but not to string)
    if (type_ == INT && other.type_ == FLOAT) {
        return static_cast<double>(int_val_) == other.float_val_;
    }
    if (type_ == FLOAT && other.type_ == INT) {
        return float_val_ == static_cast<double>(other.int_val_);
    }
    if (type_ == BOOL && (other.type_ == INT || other.type_ == FLOAT)) {
        return asInt() == other.asInt();
    }
    if (other.type_ == BOOL && (type_ == INT || type_ == FLOAT)) {
        return int_val_ == other.asInt();
    }

    return false;
}

bool Value::operator!=(const Value& other) const {
    return !(*this == other);
}

bool Value::operator<(const Value& other) const {
    if (type_ == STRING && other.type_ == STRING) {
        return string_val_ < other.string_val_;
    }
    if (type_ == INT && other.type_ == INT) {
        return int_val_ < other.int_val_;
    }
    if (type_ == FLOAT && other.type_ == FLOAT) {
        return float_val_ < other.float_val_;
    }
    if (type_ == INT && other.type_ == FLOAT) {
        return static_cast<double>(int_val_) < other.float_val_;
    }
    if (type_ == FLOAT && other.type_ == INT) {
        return float_val_ < static_cast<double>(other.int_val_);
    }
    if (type_ == BOOL) {
        return asInt() < other.asInt();
    }
    if (other.type_ == BOOL) {
        return asInt() < other.asInt();
    }
    throw std::runtime_error("Cannot compare these types with <");
}

bool Value::operator<=(const Value& other) const {
    return *this < other || *this == other;
}

bool Value::operator>(const Value& other) const {
    return !(*this <= other);
}

bool Value::operator>=(const Value& other) const {
    return !(*this < other);
}

// Arithmetic operators
Value Value::operator+(const Value& other) const {
    if (type_ == STRING && other.type_ == STRING) {
        return Value(string_val_ + other.string_val_);
    }
    if (type_ == INT && other.type_ == INT) {
        return Value(int_val_ + other.int_val_);
    }
    if (type_ == FLOAT && other.type_ == FLOAT) {
        return Value(float_val_ + other.float_val_);
    }
    if (type_ == INT && other.type_ == FLOAT) {
        return Value(static_cast<double>(int_val_) + other.float_val_);
    }
    if (type_ == FLOAT && other.type_ == INT) {
        return Value(float_val_ + static_cast<double>(other.int_val_));
    }
    if (type_ == STRING && other.type_ == INT) {
        std::string result;
        for (long long i = 0; i < other.int_val_; ++i) {
            result += string_val_;
        }
        return Value(result);
    }
    if (type_ == INT && other.type_ == STRING) {
        std::string result;
        for (long long i = 0; i < int_val_; ++i) {
            result += other.string_val_;
        }
        return Value(result);
    }
    throw std::runtime_error("Unsupported operands for +");
}

Value Value::operator-(const Value& other) const {
    if (type_ == INT && other.type_ == INT) {
        return Value(int_val_ - other.int_val_);
    }
    if (type_ == FLOAT && other.type_ == FLOAT) {
        return Value(float_val_ - other.float_val_);
    }
    if (type_ == INT && other.type_ == FLOAT) {
        return Value(static_cast<double>(int_val_) - other.float_val_);
    }
    if (type_ == FLOAT && other.type_ == INT) {
        return Value(float_val_ - static_cast<double>(other.int_val_));
    }
    throw std::runtime_error("Unsupported operands for -");
}

Value Value::operator*(const Value& other) const {
    if (type_ == INT && other.type_ == INT) {
        return Value(int_val_ * other.int_val_);
    }
    if (type_ == FLOAT && other.type_ == FLOAT) {
        return Value(float_val_ * other.float_val_);
    }
    if (type_ == INT && other.type_ == FLOAT) {
        return Value(static_cast<double>(int_val_) * other.float_val_);
    }
    if (type_ == FLOAT && other.type_ == INT) {
        return Value(float_val_ * static_cast<double>(other.int_val_));
    }
    if (type_ == STRING && other.type_ == INT) {
        std::string result;
        for (long long i = 0; i < other.int_val_; ++i) {
            result += string_val_;
        }
        return Value(result);
    }
    if (type_ == INT && other.type_ == STRING) {
        std::string result;
        for (long long i = 0; i < int_val_; ++i) {
            result += other.string_val_;
        }
        return Value(result);
    }
    throw std::runtime_error("Unsupported operands for *");
}

Value Value::operator/(const Value& other) const {
    if (type_ == INT && other.type_ == INT) {
        if (other.int_val_ == 0) throw std::runtime_error("Division by zero");
        return Value(static_cast<double>(int_val_) / static_cast<double>(other.int_val_));
    }
    if (type_ == FLOAT && other.type_ == FLOAT) {
        if (other.float_val_ == 0.0) throw std::runtime_error("Division by zero");
        return Value(float_val_ / other.float_val_);
    }
    if (type_ == INT && other.type_ == FLOAT) {
        if (other.float_val_ == 0.0) throw std::runtime_error("Division by zero");
        return Value(static_cast<double>(int_val_) / other.float_val_);
    }
    if (type_ == FLOAT && other.type_ == INT) {
        if (other.int_val_ == 0) throw std::runtime_error("Division by zero");
        return Value(float_val_ / static_cast<double>(other.int_val_));
    }
    throw std::runtime_error("Unsupported operands for /");
}

Value Value::operator%(const Value& other) const {
    if (type_ == INT && other.type_ == INT) {
        if (other.int_val_ == 0) throw std::runtime_error("Modulo by zero");
        long long a = int_val_;
        long long b = other.int_val_;
        long long result = a - (a / b) * b;
        return Value(result);
    }
    if (type_ == FLOAT && other.type_ == FLOAT) {
        if (other.float_val_ == 0.0) throw std::runtime_error("Modulo by zero");
        double a = float_val_;
        double b = other.float_val_;
        double result = std::fmod(a, b);
        return Value(result);
    }
    if (type_ == INT && other.type_ == FLOAT) {
        if (other.float_val_ == 0.0) throw std::runtime_error("Modulo by zero");
        double a = static_cast<double>(int_val_);
        double b = other.float_val_;
        double result = std::fmod(a, b);
        return Value(result);
    }
    if (type_ == FLOAT && other.type_ == INT) {
        if (other.int_val_ == 0) throw std::runtime_error("Modulo by zero");
        double a = float_val_;
        double b = static_cast<double>(other.int_val_);
        double result = std::fmod(a, b);
        return Value(result);
    }
    throw std::runtime_error("Unsupported operands for %");
}

Value Value::operator-() const {
    if (type_ == INT) return Value(-int_val_);
    if (type_ == FLOAT) return Value(-float_val_);
    throw std::runtime_error("Unary - not supported for this type");
}

Value Value::operator!() const {
    return Value(!asBool());
}

Value Value::floorDiv(const Value& other) const {
    if (type_ == INT && other.type_ == INT) {
        if (other.int_val_ == 0) throw std::runtime_error("Division by zero");
        long long a = int_val_;
        long long b = other.int_val_;
        long long result = std::floor(static_cast<double>(a) / static_cast<double>(b));
        return Value(result);
    }
    if (type_ == FLOAT && other.type_ == FLOAT) {
        if (other.float_val_ == 0.0) throw std::runtime_error("Division by zero");
        double result = std::floor(float_val_ / other.float_val_);
        return Value(result);
    }
    if (type_ == INT && other.type_ == FLOAT) {
        if (other.float_val_ == 0.0) throw std::runtime_error("Division by zero");
        double result = std::floor(static_cast<double>(int_val_) / other.float_val_);
        return Value(result);
    }
    if (type_ == FLOAT && other.type_ == INT) {
        if (other.int_val_ == 0) throw std::runtime_error("Division by zero");
        double result = std::floor(float_val_ / static_cast<double>(other.int_val_));
        return Value(result);
    }
    throw std::runtime_error("Unsupported operands for //");
}

// EvalVisitor implementation
EvalVisitor::EvalVisitor() {
    global_scope = std::make_shared<Scope>();
    current_scope = global_scope;
    has_return = false;
    should_break = false;
    should_continue = false;
}

void EvalVisitor::enterScope() {
    current_scope = std::make_shared<Scope>(current_scope);
}

void EvalVisitor::exitScope() {
    if (current_scope->parent) {
        current_scope = current_scope->parent;
    }
}

std::string EvalVisitor::getStringLiteral(antlr4::tree::TerminalNode* node) {
    if (!node) return "";
    std::string s = node->toString();
    if (s.length() >= 2 && (s[0] == '"' || s[0] == '\'')) {
        return s.substr(1, s.length() - 2);
    }
    return s;
}

// Visitor methods
std::any EvalVisitor::visitFile_input(Python3Parser::File_inputContext* ctx) {
    for (auto stmt : ctx->stmt()) {
        executeStatement(stmt);
        if (should_break || should_continue) {
            break;
        }
    }
    std::cout << output_buffer.str();
    return nullptr;
}

std::any EvalVisitor::visitFuncdef(Python3Parser::FuncdefContext* ctx) {
    std::string name = ctx->NAME()->toString();
    std::vector<std::string> params;
    std::map<std::string, Value> defaults;

    auto typedargslist = ctx->parameters()->typedargslist();
    if (typedargslist) {
        for (size_t i = 0; i < typedargslist->tfpdef().size(); ++i) {
            params.push_back(typedargslist->tfpdef(i)->NAME()->toString());
        }
        for (size_t i = 0; i < typedargslist->test().size(); ++i) {
            auto test_val = std::any_cast<Value>(visit(typedargslist->test(i)));
            defaults[params[params.size() - typedargslist->test().size() + i]] = test_val;
        }
    }

    functions[name] = Function(name, params, defaults, ctx->suite());
    return nullptr;
}

std::any EvalVisitor::visitParameters(Python3Parser::ParametersContext* ctx) {
    return visitChildren(ctx);
}

std::any EvalVisitor::visitTypedargslist(Python3Parser::TypedargslistContext* ctx) {
    return visitChildren(ctx);
}

std::any EvalVisitor::visitTfpdef(Python3Parser::TfpdefContext* ctx) {
    return visitChildren(ctx);
}

std::any EvalVisitor::visitStmt(Python3Parser::StmtContext* ctx) {
    return visitChildren(ctx);
}

std::any EvalVisitor::visitSimple_stmt(Python3Parser::Simple_stmtContext* ctx) {
    return visitChildren(ctx);
}

std::any EvalVisitor::visitSmall_stmt(Python3Parser::Small_stmtContext* ctx) {
    return visitChildren(ctx);
}

std::any EvalVisitor::visitExpr_stmt(Python3Parser::Expr_stmtContext* ctx) {
    executeExprStmt(ctx);
    return nullptr;
}

std::any EvalVisitor::visitAugassign(Python3Parser::AugassignContext* ctx) {
    return visitChildren(ctx);
}

std::any EvalVisitor::visitFlow_stmt(Python3Parser::Flow_stmtContext* ctx) {
    return visitChildren(ctx);
}

std::any EvalVisitor::visitBreak_stmt(Python3Parser::Break_stmtContext* ctx) {
    executeBreakStmt();
    return nullptr;
}

std::any EvalVisitor::visitContinue_stmt(Python3Parser::Continue_stmtContext* ctx) {
    executeContinueStmt();
    return nullptr;
}

std::any EvalVisitor::visitReturn_stmt(Python3Parser::Return_stmtContext* ctx) {
    executeReturnStmt(ctx);
    return nullptr;
}

std::any EvalVisitor::visitCompound_stmt(Python3Parser::Compound_stmtContext* ctx) {
    if (ctx->if_stmt()) {
        executeIfStmt(ctx->if_stmt());
    } else if (ctx->while_stmt()) {
        executeWhileStmt(ctx->while_stmt());
    } else if (ctx->funcdef()) {
        executeFuncDef(ctx->funcdef());
    }
    return nullptr;
}

std::any EvalVisitor::visitIf_stmt(Python3Parser::If_stmtContext* ctx) {
    return visitChildren(ctx);
}

std::any EvalVisitor::visitWhile_stmt(Python3Parser::While_stmtContext* ctx) {
    return visitChildren(ctx);
}

std::any EvalVisitor::visitSuite(Python3Parser::SuiteContext* ctx) {
    return visitChildren(ctx);
}

std::any EvalVisitor::visitTest(Python3Parser::TestContext* ctx) {
    return evaluateExpression(ctx);
}

std::any EvalVisitor::visitOr_test(Python3Parser::Or_testContext* ctx) {
    return evaluateOrTest(ctx);
}

std::any EvalVisitor::visitAnd_test(Python3Parser::And_testContext* ctx) {
    return evaluateAndTest(ctx);
}

std::any EvalVisitor::visitNot_test(Python3Parser::Not_testContext* ctx) {
    return evaluateNotTest(ctx);
}

std::any EvalVisitor::visitComparison(Python3Parser::ComparisonContext* ctx) {
    return evaluateComparison(ctx);
}

std::any EvalVisitor::visitComp_op(Python3Parser::Comp_opContext* ctx) {
    return visitChildren(ctx);
}

std::any EvalVisitor::visitArith_expr(Python3Parser::Arith_exprContext* ctx) {
    return evaluateArithExpr(ctx);
}

std::any EvalVisitor::visitAddorsub_op(Python3Parser::Addorsub_opContext* ctx) {
    return visitChildren(ctx);
}

std::any EvalVisitor::visitTerm(Python3Parser::TermContext* ctx) {
    return evaluateTerm(ctx);
}

std::any EvalVisitor::visitMuldivmod_op(Python3Parser::Muldivmod_opContext* ctx) {
    return visitChildren(ctx);
}

std::any EvalVisitor::visitFactor(Python3Parser::FactorContext* ctx) {
    return evaluateFactor(ctx);
}

std::any EvalVisitor::visitAtom_expr(Python3Parser::Atom_exprContext* ctx) {
    return evaluateAtomExpr(ctx);
}

std::any EvalVisitor::visitTrailer(Python3Parser::TrailerContext* ctx) {
    return visitChildren(ctx);
}

std::any EvalVisitor::visitAtom(Python3Parser::AtomContext* ctx) {
    return evaluateAtom(ctx);
}

std::any EvalVisitor::visitFormat_string(Python3Parser::Format_stringContext* ctx) {
    return evaluateFormatString(ctx);
}

std::any EvalVisitor::visitTestlist(Python3Parser::TestlistContext* ctx) {
    std::vector<Value> values;
    for (auto test : ctx->test()) {
        values.push_back(std::any_cast<Value>(visit(test)));
    }
    // If there's only one value and no comma, return it directly instead of a tuple
    if (values.size() == 1 && ctx->COMMA().size() == 0) {
        return values[0];
    }
    return Value(values);
}

std::any EvalVisitor::visitArglist(Python3Parser::ArglistContext* ctx) {
    return visitChildren(ctx);
}

std::any EvalVisitor::visitArgument(Python3Parser::ArgumentContext* ctx) {
    return visitChildren(ctx);
}

// Expression evaluation
Value EvalVisitor::evaluateExpression(Python3Parser::TestContext* ctx) {
    return std::any_cast<Value>(visit(ctx->or_test()));
}

Value EvalVisitor::evaluateOrTest(Python3Parser::Or_testContext* ctx) {
    Value result = std::any_cast<Value>(visit(ctx->and_test(0)));
    for (size_t i = 1; i < ctx->and_test().size(); ++i) {
        if (result.asBool()) {
            return result;  // Short-circuit
        }
        result = std::any_cast<Value>(visit(ctx->and_test(i)));
    }
    return result;
}

Value EvalVisitor::evaluateAndTest(Python3Parser::And_testContext* ctx) {
    Value result = std::any_cast<Value>(visit(ctx->not_test(0)));
    for (size_t i = 1; i < ctx->not_test().size(); ++i) {
        if (!result.asBool()) {
            return result;  // Short-circuit
        }
        result = std::any_cast<Value>(visit(ctx->not_test(i)));
    }
    return result;
}

Value EvalVisitor::evaluateNotTest(Python3Parser::Not_testContext* ctx) {
    if (ctx->NOT()) {
        Value val = std::any_cast<Value>(visit(ctx->not_test()));
        return Value(!val.asBool());
    }
    return std::any_cast<Value>(visit(ctx->comparison()));
}

Value EvalVisitor::evaluateComparison(Python3Parser::ComparisonContext* ctx) {
    std::vector<Value> values;
    for (auto arith : ctx->arith_expr()) {
        values.push_back(std::any_cast<Value>(visit(arith)));
    }

    if (values.size() == 1) {
        return values[0];
    }

    for (size_t i = 0; i < ctx->comp_op().size(); ++i) {
        auto op = ctx->comp_op(i);
        Value left = values[i];
        Value right = values[i + 1];

        bool result = false;
        if (op->LESS_THAN()) {
            result = left < right;
        } else if (op->GREATER_THAN()) {
            result = left > right;
        } else if (op->EQUALS()) {
            result = left == right;
        } else if (op->GT_EQ()) {
            result = left >= right;
        } else if (op->LT_EQ()) {
            result = left <= right;
        } else if (op->NOT_EQ_2()) {
            result = left != right;
        }

        if (!result) {
            return Value(false);
        }
    }
    return Value(true);
}

Value EvalVisitor::evaluateArithExpr(Python3Parser::Arith_exprContext* ctx) {
    Value result = std::any_cast<Value>(visit(ctx->term(0)));
    for (size_t i = 0; i < ctx->addorsub_op().size(); ++i) {
        auto op = ctx->addorsub_op(i);
        Value right = std::any_cast<Value>(visit(ctx->term(i + 1)));
        if (op->ADD()) {
            result = result + right;
        } else if (op->MINUS()) {
            result = result - right;
        }
    }
    return result;
}

Value EvalVisitor::evaluateTerm(Python3Parser::TermContext* ctx) {
    Value result = std::any_cast<Value>(visit(ctx->factor(0)));
    for (size_t i = 0; i < ctx->muldivmod_op().size(); ++i) {
        auto op = ctx->muldivmod_op(i);
        Value right = std::any_cast<Value>(visit(ctx->factor(i + 1)));
        if (op->STAR()) {
            result = result * right;
        } else if (op->DIV()) {
            result = result / right;
        } else if (op->IDIV()) {
            result = result.floorDiv(right);
        } else if (op->MOD()) {
            result = result % right;
        }
    }
    return result;
}

Value EvalVisitor::evaluateFactor(Python3Parser::FactorContext* ctx) {
    if (ctx->ADD()) {
        return std::any_cast<Value>(visit(ctx->factor()));
    } else if (ctx->MINUS()) {
        Value val = std::any_cast<Value>(visit(ctx->factor()));
        return -val;
    }
    return std::any_cast<Value>(visit(ctx->atom_expr()));
}

Value EvalVisitor::evaluateAtomExpr(Python3Parser::Atom_exprContext* ctx) {
    Value atom_val = std::any_cast<Value>(visit(ctx->atom()));

    if (ctx->trailer()) {
        // Function call
        std::string func_name;
        if (atom_val.type() == Value::STRING) {
            func_name = atom_val.asString();
        } else {
            // Get the function name from the atom directly
            auto atom = ctx->atom();
            if (atom->NAME()) {
                func_name = atom->NAME()->toString();
            }
        }

        if (!func_name.empty()) {
            std::vector<Value> args;
            std::map<std::string, Value> kwargs;

            auto arglist = ctx->trailer()->arglist();
            if (arglist) {
                for (auto arg : arglist->argument()) {
                    if (arg->ASSIGN()) {
                        // Keyword argument
                        std::string key = arg->test(0)->getText();
                        Value val = std::any_cast<Value>(visit(arg->test(1)));
                        kwargs[key] = val;
                    } else {
                        // Positional argument
                        args.push_back(std::any_cast<Value>(visit(arg->test(0))));
                    }
                }
            }

            return callFunction(func_name, args, kwargs);
        }
    }

    return atom_val;
}

Value EvalVisitor::evaluateAtom(Python3Parser::AtomContext* ctx) {
    if (ctx->NAME()) {
        std::string name = ctx->NAME()->toString();
        // Try to look up the variable in the scope
        try {
            return current_scope->get(name);
        } catch (...) {
            // Variable not defined, return the name as a string
            return Value(name);
        }
    } else if (ctx->NUMBER()) {
        std::string num_str = ctx->NUMBER()->toString();
        if (num_str.find('.') != std::string::npos) {
            return Value(std::stod(num_str));
        } else {
            return Value(std::stoll(num_str));
        }
    } else if (ctx->NONE()) {
        return Value();
    } else if (ctx->TRUE()) {
        return Value(true);
    } else if (ctx->FALSE()) {
        return Value(false);
    } else if (ctx->format_string()) {
        return std::any_cast<Value>(visit(ctx->format_string()));
    } else if (ctx->OPEN_PAREN()) {
        return std::any_cast<Value>(visit(ctx->test()));
    } else if (ctx->STRING().size() > 0) {
        std::string result;
        for (auto str : ctx->STRING()) {
            result += getStringLiteral(str);
        }
        return Value(result);
    }
    throw std::runtime_error("Unknown atom type");
}

Value EvalVisitor::evaluateFormatString(Python3Parser::Format_stringContext* ctx) {
    std::string result;

    // Process format string parts
    size_t brace_count = 0;
    for (size_t i = 0; i < ctx->FORMAT_STRING_LITERAL().size(); ++i) {
        result += ctx->FORMAT_STRING_LITERAL(i)->toString();
        if (i < ctx->testlist().size()) {
            Value val = std::any_cast<Value>(visit(ctx->testlist(i)));
            result += val.asString();
        }
    }

    return Value(result);
}

// Statement execution
void EvalVisitor::executeStatement(Python3Parser::StmtContext* ctx) {
    if (ctx->simple_stmt()) {
        visit(ctx->simple_stmt());
    } else if (ctx->compound_stmt()) {
        visit(ctx->compound_stmt());
    }
}

void EvalVisitor::executeExprStmt(Python3Parser::Expr_stmtContext* ctx) {
    size_t testlist_count = ctx->testlist().size();
    size_t assign_count = ctx->ASSIGN().size();
    auto augassign = ctx->augassign();

    // Check for augmented assignment
    if (augassign) {
        // Augmented assignment: var op= value
        // testlist(0) is the variable, testlist(1) is the value
        Value lhs_value = std::any_cast<Value>(visit(ctx->testlist(0)));
        Value rhs_value = std::any_cast<Value>(visit(ctx->testlist(1)));

        Value result;
        if (augassign->ADD_ASSIGN()) {
            result = lhs_value + rhs_value;
        } else if (augassign->SUB_ASSIGN()) {
            result = lhs_value - rhs_value;
        } else if (augassign->MULT_ASSIGN()) {
            result = lhs_value * rhs_value;
        } else if (augassign->DIV_ASSIGN()) {
            result = lhs_value / rhs_value;
        } else if (augassign->IDIV_ASSIGN()) {
            result = lhs_value.floorDiv(rhs_value);
        } else if (augassign->MOD_ASSIGN()) {
            result = lhs_value % rhs_value;
        }

        // Extract variable name and assign result
        auto testlist = ctx->testlist(0);
        if (testlist->test().size() == 1) {
            auto test = testlist->test(0);
            auto or_test = test->or_test();
            if (or_test->and_test().size() == 1) {
                auto and_test = or_test->and_test(0);
                if (and_test->not_test().size() == 1) {
                    auto not_test = and_test->not_test(0);
                    if (not_test->comparison()) {
                        auto comparison = not_test->comparison();
                        if (comparison->arith_expr().size() == 1) {
                            auto arith_expr = comparison->arith_expr(0);
                            if (arith_expr->term().size() == 1) {
                                auto term = arith_expr->term(0);
                                if (term->factor().size() == 1) {
                                    auto factor = term->factor(0);
                                    if (factor->atom_expr()) {
                                        auto atom_expr = factor->atom_expr();
                                        if (atom_expr->atom()) {
                                            auto atom = atom_expr->atom();
                                            if (atom->NAME()) {
                                                std::string name = atom->NAME()->toString();
                                                current_scope->set(name, result);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return;
    }

    if (assign_count == 0) {
        // Just an expression
        std::any_cast<Value>(visit(ctx->testlist(0)));
        return;
    }

    // Multiple assignment
    // For `a = 1`, we have testlist_count=2, assign_count=1
    // The last testlist is the value to assign
    // All previous testlists are the targets

    Value rhs_value = std::any_cast<Value>(visit(ctx->testlist(testlist_count - 1)));

    // Assign to all variables on the left side
    for (size_t i = 0; i < testlist_count - 1; ++i) {
        auto testlist = ctx->testlist(i);
        if (testlist->test().size() == 1) {
            // Single variable assignment - extract variable name from parse tree
            auto test = testlist->test(0);
            auto or_test = test->or_test();
            if (or_test->and_test().size() == 1) {
                auto and_test = or_test->and_test(0);
                if (and_test->not_test().size() == 1) {
                    auto not_test = and_test->not_test(0);
                    if (not_test->comparison()) {
                        auto comparison = not_test->comparison();
                        if (comparison->arith_expr().size() == 1) {
                            auto arith_expr = comparison->arith_expr(0);
                            if (arith_expr->term().size() == 1) {
                                auto term = arith_expr->term(0);
                                if (term->factor().size() == 1) {
                                    auto factor = term->factor(0);
                                    if (factor->atom_expr()) {
                                        auto atom_expr = factor->atom_expr();
                                        if (atom_expr->atom()) {
                                            auto atom = atom_expr->atom();
                                            if (atom->NAME()) {
                                                std::string name = atom->NAME()->toString();
                                                current_scope->set(name, rhs_value);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } else {
            // Tuple unpacking
            auto testlist_val = std::any_cast<Value>(visit(testlist));
            if (testlist_val.isTuple()) {
                auto tuple_vals = testlist_val.asTuple();
                for (size_t j = 0; j < tuple_vals.size(); ++j) {
                    auto test = testlist->test(j);
                    auto or_test = test->or_test();
                    if (or_test->and_test().size() == 1) {
                        auto and_test = or_test->and_test(0);
                        if (and_test->not_test().size() == 1) {
                            auto not_test = and_test->not_test(0);
                            if (not_test->comparison()) {
                                auto comparison = not_test->comparison();
                                if (comparison->arith_expr().size() == 1) {
                                    auto arith_expr = comparison->arith_expr(0);
                                    if (arith_expr->term().size() == 1) {
                                        auto term = arith_expr->term(0);
                                        if (term->factor().size() == 1) {
                                            auto factor = term->factor(0);
                                            if (factor->atom_expr()) {
                                                auto atom_expr = factor->atom_expr();
                                                if (atom_expr->atom()) {
                                                    auto atom = atom_expr->atom();
                                                    if (atom->NAME()) {
                                                        std::string name = atom->NAME()->toString();
                                                        current_scope->set(name, tuple_vals[j]);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void EvalVisitor::executeIfStmt(Python3Parser::If_stmtContext* ctx) {
    size_t num_blocks = ctx->test().size();
    for (size_t i = 0; i < num_blocks; ++i) {
        Value condition = std::any_cast<Value>(visit(ctx->test(i)));
        if (condition.asBool()) {
            executeSuite(ctx->suite(i));
            return;
        }
    }
    if (ctx->ELSE()) {
        executeSuite(ctx->suite(num_blocks));
    }
}

void EvalVisitor::executeWhileStmt(Python3Parser::While_stmtContext* ctx) {
    while (true) {
        Value condition = std::any_cast<Value>(visit(ctx->test()));
        if (!condition.asBool()) {
            break;
        }
        executeSuite(ctx->suite());
        if (should_break) {
            should_break = false;
            break;
        }
        if (should_continue) {
            should_continue = false;
        }
    }
}

void EvalVisitor::executeReturnStmt(Python3Parser::Return_stmtContext* ctx) {
    if (ctx->testlist()) {
        return_value = std::any_cast<Value>(visit(ctx->testlist()));
    } else {
        return_value = Value();
    }
    has_return = true;
}

void EvalVisitor::executeBreakStmt() {
    should_break = true;
}

void EvalVisitor::executeContinueStmt() {
    should_continue = true;
}

void EvalVisitor::executeFuncDef(Python3Parser::FuncdefContext* ctx) {
    visit(ctx);
}

void EvalVisitor::executeSuite(Python3Parser::SuiteContext* ctx) {
    if (ctx->simple_stmt()) {
        visit(ctx->simple_stmt());
    } else {
        for (auto stmt : ctx->stmt()) {
            executeStatement(stmt);
            if (should_break || should_continue || has_return) {
                break;
            }
        }
    }
}

// Function calling
Value EvalVisitor::callFunction(const std::string& name, const std::vector<Value>& args,
                                 const std::map<std::string, Value>& kwargs) {
    // Check built-in functions first
    if (name == "print" || name == "int" || name == "float" || name == "str" || name == "bool") {
        return callBuiltin(name, args);
    }

    // User-defined functions
    auto it = functions.find(name);
    if (it == functions.end()) {
        throw std::runtime_error("Undefined function: " + name);
    }

    Function& func = it->second;
    enterScope();

    // Bind arguments
    size_t arg_idx = 0;
    for (size_t i = 0; i < func.params.size(); ++i) {
        std::string param_name = func.params[i];
        auto default_it = func.defaults.find(param_name);

        if (arg_idx < args.size()) {
            current_scope->set(param_name, args[arg_idx++]);
        } else {
            auto kwarg_it = kwargs.find(param_name);
            if (kwarg_it != kwargs.end()) {
                current_scope->set(param_name, kwarg_it->second);
            } else if (default_it != func.defaults.end()) {
                current_scope->set(param_name, default_it->second);
            } else {
                throw std::runtime_error("Missing argument: " + param_name);
            }
        }
    }

    // Execute function body
    has_return = false;
    executeSuite(func.suite);
    Value result = has_return ? return_value : Value();

    exitScope();
    return result;
}

Value EvalVisitor::callBuiltin(const std::string& name, const std::vector<Value>& args) {
    if (name == "print") {
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) output_buffer << " ";
            output_buffer << args[i].asString();
        }
        output_buffer << "\n";
        return Value();
    } else if (name == "int") {
        if (args.empty()) throw std::runtime_error("int() requires an argument");
        return Value(args[0].asInt());
    } else if (name == "float") {
        if (args.empty()) throw std::runtime_error("float() requires an argument");
        return Value(args[0].asFloat());
    } else if (name == "str") {
        if (args.empty()) throw std::runtime_error("str() requires an argument");
        return Value(args[0].asString());
    } else if (name == "bool") {
        if (args.empty()) throw std::runtime_error("bool() requires an argument");
        return Value(args[0].asBool());
    }
    throw std::runtime_error("Unknown builtin function: " + name);
}
