#include <utility>

#include "MParser.h"


int VariableExp::Evaluate(Context &context) {
    return context[_name];
}

VariableExp::VariableExp(std::string name) {
    _name = std::move(name);
}

std::string VariableExp::GetName() const {
    return _name;
}

void Context::Assign(VariableExp *exp, int value) {
    _variables[exp->GetName()] = value;
}

bool Context::isExist(const std::string &str) {
    return _variables.find(str) != _variables.end();
}

int AddExpression::Evaluate(Context &context) {
    return _operand1->Evaluate(context) + _operand2->Evaluate(context);
}

AddExpression::AddExpression(IExpression *operand1, IExpression *operand2) {
    _operand1 = operand1;
    _operand2 = operand2;
}

SubExpression::SubExpression(IExpression *operand1, IExpression *operand2) {
    _operand1 = operand1;
    _operand2 = operand2;
}

int SubExpression::Evaluate(Context &context) {
    return _operand1->Evaluate(context) - _operand2->Evaluate(context);
}

MulExpression::MulExpression(IExpression *operand1, IExpression *operand2) {
    _operand1 = operand1;
    _operand2 = operand2;
}

int MulExpression::Evaluate(Context &context) {
    return _operand1->Evaluate(context) * _operand2->Evaluate(context);
}

DivExpression::DivExpression(IExpression *operand1, IExpression *operand2) {
    _operand1 = operand1;
    _operand2 = operand2;
}

int DivExpression::Evaluate(Context &context) {
    return _operand1->Evaluate(context) / _operand2->Evaluate(context);
}

ModExpression::ModExpression(IExpression *operand1, IExpression *operand2) {
    _operand1 = operand1;
    _operand2 = operand2;
}

int ModExpression::Evaluate(Context &context) {
    return _operand1->Evaluate(context) % _operand2->Evaluate(context);
}
