#ifndef SYSTEMSOFTWARE2_MPARSER_H
#define SYSTEMSOFTWARE2_MPARSER_H

//Парсер математичких выражений

#include <unordered_map>
#include <string>
class Context;
//Интерфейс всех классов, которые описывают выражение
class IExpression
{
public:
    //Вычисляет выражение в контексте
    virtual int Evaluate(Context&) = 0;
};
//Именнованная переменная
class VariableExp: public IExpression
{
    std::string _name;
public:
    explicit VariableExp(std::string);
    int Evaluate(Context &context) override;
    std::string GetName() const;
};
//Константа
class ConstantExp: public IExpression
{
    int _value;
public:
    explicit ConstantExp(int value): _value{value}{};
    int Evaluate(Context &context) override
    {return _value;}

};
//Класс Context определяет отображение между перменными и операциями
class Context final
{
    std::unordered_map<std::string, int> _variables;
public:
    void Assign(VariableExp*, int);
    int &operator[](const std::string &name) { return _variables[name]; }
    bool isExist(const std::string&);
    auto begin() const noexcept { return _variables.begin(); }
    auto end() const noexcept { return _variables.end(); }
};
//====Далее рутина по созданию операций
//Выражение полученное в результате сложения
class AddExpression: public IExpression
{
    IExpression *_operand1;
    IExpression *_operand2;
public:
    AddExpression(IExpression *operand1, IExpression *operand2);
    int Evaluate(Context &context) override;
};
//Выражение полученное в результате вычитания
class SubExpression: public IExpression
{
    IExpression *_operand1;
    IExpression *_operand2;
public:
    SubExpression(IExpression *operand1, IExpression *operand2);
    int Evaluate(Context &context) override;
};
//Выражение полученное в результате умножения
class MulExpression: public IExpression
{
    IExpression *_operand1;
    IExpression *_operand2;
public:
    MulExpression(IExpression *operand1, IExpression *operand2);
    int Evaluate(Context &context) override;
};
//Выражение полученное в результате деления
class DivExpression: public IExpression
{
    IExpression *_operand1;
    IExpression *_operand2;
public:
    DivExpression(IExpression *operand1, IExpression *operand2);
    int Evaluate(Context &context) override;
};
//Выражение полученное в результате получения остатка от деления
class ModExpression: public IExpression
{
    IExpression *_operand1;
    IExpression *_operand2;
public:
    ModExpression(IExpression *operand1, IExpression *operand2);
    int Evaluate(Context &context) override;
};
#endif //SYSTEMSOFTWARE2_MPARSER_H
