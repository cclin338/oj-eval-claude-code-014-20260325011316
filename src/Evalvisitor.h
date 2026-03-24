#pragma once
#ifndef PYTHON_INTERPRETER_EVALVISITOR_H
#define PYTHON_INTERPRETER_EVALVISITOR_H

#include "Python3ParserBaseVisitor.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <variant>
#include <stdexcept>
#include <cmath>

// Value types
class Value {
public:
    enum Type { NONE, BOOL, INT, FLOAT, STRING, TUPLE };

    Value() : type_(NONE) {}
    explicit Value(bool b) : type_(BOOL), bool_val_(b) {}
    explicit Value(long long i) : type_(INT), int_val_(i) {}
    explicit Value(const std::string& s) : type_(STRING), string_val_(s) {}
    explicit Value(double f) : type_(FLOAT), float_val_(f) {}
    explicit Value(const std::vector<Value>& t) : type_(TUPLE), tuple_val_(t) {}

    Type type() const { return type_; }

    bool isNone() const { return type_ == NONE; }
    bool isBool() const { return type_ == BOOL; }
    bool isInt() const { return type_ == INT; }
    bool isFloat() const { return type_ == FLOAT; }
    bool isString() const { return type_ == STRING; }
    bool isTuple() const { return type_ == TUPLE; }
    bool isNumber() const { return type_ == INT || type_ == FLOAT; }

    bool asBool() const {
        if (type_ == BOOL) return bool_val_;
        if (type_ == INT) return int_val_ != 0;
        if (type_ == FLOAT) return float_val_ != 0.0;
        if (type_ == STRING) return !string_val_.empty();
        if (type_ == NONE) return false;
        if (type_ == TUPLE) return !tuple_val_.empty();
        throw std::runtime_error("Cannot convert to bool");
    }

    long long asInt() const {
        if (type_ == BOOL) return bool_val_ ? 1 : 0;
        if (type_ == INT) return int_val_;
        if (type_ == FLOAT) return static_cast<long long>(float_val_);
        if (type_ == STRING) {
            try {
                size_t pos;
                long long result = std::stoll(string_val_, &pos);
                if (pos != string_val_.length()) {
                    throw std::runtime_error("Invalid integer string");
                }
                return result;
            } catch (...) {
                throw std::runtime_error("Invalid integer string");
            }
        }
        throw std::runtime_error("Cannot convert to int");
    }

    double asFloat() const {
        if (type_ == BOOL) return bool_val_ ? 1.0 : 0.0;
        if (type_ == INT) return static_cast<double>(int_val_);
        if (type_ == FLOAT) return float_val_;
        if (type_ == STRING) {
            try {
                return std::stod(string_val_);
            } catch (...) {
                throw std::runtime_error("Invalid float string");
            }
        }
        throw std::runtime_error("Cannot convert to float");
    }

    std::string asString() const {
        if (type_ == STRING) return string_val_;
        if (type_ == BOOL) return bool_val_ ? "True" : "False";
        if (type_ == INT) return std::to_string(int_val_);
        if (type_ == FLOAT) {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(6) << float_val_;
            return oss.str();
        }
        if (type_ == NONE) return "None";
        if (type_ == TUPLE) {
            std::string result = "(";
            for (size_t i = 0; i < tuple_val_.size(); ++i) {
                if (i > 0) result += ", ";
                result += tuple_val_[i].asString();
            }
            result += ")";
            return result;
        }
        throw std::runtime_error("Cannot convert to string");
    }

    std::vector<Value> asTuple() const {
        if (type_ == TUPLE) return tuple_val_;
        throw std::runtime_error("Not a tuple");
    }

    bool operator==(const Value& other) const;
    bool operator!=(const Value& other) const;
    bool operator<(const Value& other) const;
    bool operator<=(const Value& other) const;
    bool operator>(const Value& other) const;
    bool operator>=(const Value& other) const;

    Value operator+(const Value& other) const;
    Value operator-(const Value& other) const;
    Value operator*(const Value& other) const;
    Value operator/(const Value& other) const;
    Value operator%(const Value& other) const;
    Value operator-() const;
    Value operator!() const;

    Value floorDiv(const Value& other) const;

private:
    Type type_;
    bool bool_val_;
    long long int_val_;
    double float_val_;
    std::string string_val_;
    std::vector<Value> tuple_val_;
};

// Function definition
class Function {
public:
    std::string name;
    std::vector<std::string> params;
    std::map<std::string, Value> defaults;
    Python3Parser::SuiteContext* suite;

    Function() : suite(nullptr) {}
    Function(const std::string& n, const std::vector<std::string>& p,
             const std::map<std::string, Value>& d, Python3Parser::SuiteContext* s)
        : name(n), params(p), defaults(d), suite(s) {}
};

// Scope management
class Scope {
public:
    std::map<std::string, Value> variables;
    std::shared_ptr<Scope> parent;

    Scope(std::shared_ptr<Scope> p = nullptr) : parent(p) {}

    Value& get(const std::string& name) {
        auto it = variables.find(name);
        if (it != variables.end()) {
            return it->second;
        }
        if (parent) {
            return parent->get(name);
        }
        throw std::runtime_error("Undefined variable: " + name);
    }

    void set(const std::string& name, const Value& value) {
        variables[name] = value;
    }

    bool has(const std::string& name) const {
        if (variables.find(name) != variables.end()) {
            return true;
        }
        if (parent) {
            return parent->has(name);
        }
        return false;
    }
};

class EvalVisitor : public Python3ParserBaseVisitor {
private:
    std::shared_ptr<Scope> global_scope;
    std::shared_ptr<Scope> current_scope;
    std::map<std::string, Function> functions;
    Value return_value;
    bool has_return;
    bool should_break;
    bool should_continue;
    std::ostringstream output_buffer;

    void enterScope();
    void exitScope();

    Value evaluateExpression(Python3Parser::TestContext* ctx);
    Value evaluateOrTest(Python3Parser::Or_testContext* ctx);
    Value evaluateAndTest(Python3Parser::And_testContext* ctx);
    Value evaluateNotTest(Python3Parser::Not_testContext* ctx);
    Value evaluateComparison(Python3Parser::ComparisonContext* ctx);
    Value evaluateArithExpr(Python3Parser::Arith_exprContext* ctx);
    Value evaluateTerm(Python3Parser::TermContext* ctx);
    Value evaluateFactor(Python3Parser::FactorContext* ctx);
    Value evaluateAtomExpr(Python3Parser::Atom_exprContext* ctx);
    Value evaluateAtom(Python3Parser::AtomContext* ctx);
    Value evaluateFormatString(Python3Parser::Format_stringContext* ctx);

    void executeStatement(Python3Parser::StmtContext* ctx);
    void executeExprStmt(Python3Parser::Expr_stmtContext* ctx);
    void executeIfStmt(Python3Parser::If_stmtContext* ctx);
    void executeWhileStmt(Python3Parser::While_stmtContext* ctx);
    void executeReturnStmt(Python3Parser::Return_stmtContext* ctx);
    void executeBreakStmt();
    void executeContinueStmt();
    void executeFuncDef(Python3Parser::FuncdefContext* ctx);
    void executeSuite(Python3Parser::SuiteContext* ctx);

    Value callFunction(const std::string& name, const std::vector<Value>& args,
                       const std::map<std::string, Value>& kwargs);
    Value callBuiltin(const std::string& name, const std::vector<Value>& args);

    std::string getStringLiteral(antlr4::tree::TerminalNode* node);

public:
    EvalVisitor();

    std::any visitFile_input(Python3Parser::File_inputContext* ctx) override;
    std::any visitFuncdef(Python3Parser::FuncdefContext* ctx) override;
    std::any visitParameters(Python3Parser::ParametersContext* ctx) override;
    std::any visitTypedargslist(Python3Parser::TypedargslistContext* ctx) override;
    std::any visitTfpdef(Python3Parser::TfpdefContext* ctx) override;
    std::any visitStmt(Python3Parser::StmtContext* ctx) override;
    std::any visitSimple_stmt(Python3Parser::Simple_stmtContext* ctx) override;
    std::any visitSmall_stmt(Python3Parser::Small_stmtContext* ctx) override;
    std::any visitExpr_stmt(Python3Parser::Expr_stmtContext* ctx) override;
    std::any visitAugassign(Python3Parser::AugassignContext* ctx) override;
    std::any visitFlow_stmt(Python3Parser::Flow_stmtContext* ctx) override;
    std::any visitBreak_stmt(Python3Parser::Break_stmtContext* ctx) override;
    std::any visitContinue_stmt(Python3Parser::Continue_stmtContext* ctx) override;
    std::any visitReturn_stmt(Python3Parser::Return_stmtContext* ctx) override;
    std::any visitCompound_stmt(Python3Parser::Compound_stmtContext* ctx) override;
    std::any visitIf_stmt(Python3Parser::If_stmtContext* ctx) override;
    std::any visitWhile_stmt(Python3Parser::While_stmtContext* ctx) override;
    std::any visitSuite(Python3Parser::SuiteContext* ctx) override;
    std::any visitTest(Python3Parser::TestContext* ctx) override;
    std::any visitOr_test(Python3Parser::Or_testContext* ctx) override;
    std::any visitAnd_test(Python3Parser::And_testContext* ctx) override;
    std::any visitNot_test(Python3Parser::Not_testContext* ctx) override;
    std::any visitComparison(Python3Parser::ComparisonContext* ctx) override;
    std::any visitComp_op(Python3Parser::Comp_opContext* ctx) override;
    std::any visitArith_expr(Python3Parser::Arith_exprContext* ctx) override;
    std::any visitAddorsub_op(Python3Parser::Addorsub_opContext* ctx) override;
    std::any visitTerm(Python3Parser::TermContext* ctx) override;
    std::any visitMuldivmod_op(Python3Parser::Muldivmod_opContext* ctx) override;
    std::any visitFactor(Python3Parser::FactorContext* ctx) override;
    std::any visitAtom_expr(Python3Parser::Atom_exprContext* ctx) override;
    std::any visitTrailer(Python3Parser::TrailerContext* ctx) override;
    std::any visitAtom(Python3Parser::AtomContext* ctx) override;
    std::any visitFormat_string(Python3Parser::Format_stringContext* ctx) override;
    std::any visitTestlist(Python3Parser::TestlistContext* ctx) override;
    std::any visitArglist(Python3Parser::ArglistContext* ctx) override;
    std::any visitArgument(Python3Parser::ArgumentContext* ctx) override;
};

#endif // PYTHON_INTERPRETER_EVALVISITOR_H
