#include <algorithm>
#include <stack>
#include "MClient.h"
#include "Operator.h"
#include "StrToDate.h"

using namespace std;



enum priority: int8_t
{
    High = 2,    //Высокий
    Mid =  1,       //Средний
    Low = 0,        //Низкий
    NAO = -1        //Не операция
};

int8_t MClient::getPriorityOperatior(char ch) noexcept {
    switch(ch)
    {
        case '*':
        case '%':
        case '/':
            return priority::High;
        case '+':
        case '-':
            return priority::Mid;
        case '(':
        case ')':
            return priority::Low;
        default:
            return priority::NAO;
    }
}

IExpression *MClient::getVar(char ch, IExpression *op1, IExpression *op2) {
    switch(ch)
    {
        case '+' :
            return (VariableExp*) new AddExpression(op1, op2);
        case '-' :
            return (VariableExp*) new SubExpression(op1, op2);
        case '*' :
            return (VariableExp*) new MulExpression(op1, op2);
        case '/' :
            return (VariableExp*) new DivExpression(op1, op2);
        case '%' :
            return (VariableExp*) new ModExpression(op1, op2);
        default:
            return nullptr;
    }
}

MClient::MClient(std::string expString, Context &ctx, Error &e)  {
    try
    {
        _ctx = make_shared<Context>(ctx);
        size_t i;
        while((i=expString.find(' ')) != std::string::npos)
            expString.erase(i, 1);
        try {
            vector<string> tokens = lexAnalyze(expString);

            vector<string> RPN = getRPN(tokens);

            buildExpTree(RPN);
        }
        catch(...)
        {
            e = Error::illExpression;
        }

    }
    catch (exception&)
    {
        e = Error::illExpression;
    }
}

std::vector<std::string> MClient::lexAnalyze(const std::string &expression) {
    vector<string> res;
    int i = 0, n = expression.size();
    string curLexem;

    while (i < n)
    {
        char curChar = expression[i];
        if (isalpha(curChar) || curChar == '_')
        {
            do
            {
                curLexem += curChar; ++i; curChar = expression[i];
            }
            while ((i < n) && (isalpha(curChar) || isdigit(curChar) || (curChar == '_')));
            res.push_back(curLexem); curLexem = ""; //Если идентификатор
        }
        else if (getPriorityOperatior(curChar) != priority::NAO || curChar == LcSimbol[0])
        {
            res.emplace_back(1, curChar); ++i; //Если операция
        }
        else if (isdigit(curChar))
        {
            do
            {
                curLexem += curChar; ++i; curChar = expression[i];
            }
            while ((i < n) && (isalpha(curChar) || isdigit(curChar)));
            res.push_back(curLexem); curLexem = "";   //Если число
        }
        else throw WrongLexException(); //Всё остальное неверная лексема
    }

    return res;
}

std::vector<std::string> MClient::getRPN(const std::vector<std::string> &lexStrings) {
    /*
    *   Пока есть ещё символы для чтения:
        Читаем очередной символ.
        Если символ является числом или постфиксной функцией (например, ! — факториал), добавляем его к выходной строке.
        Если символ является открывающей скобкой, помещаем его в стек.
        Если символ является закрывающей скобкой:
            До тех пор, пока верхним элементом стека не станет открывающая скобка, выталкиваем элементы из стека в выходную строку. При этом открывающая скобка удаляется из стека, но в выходную строку не добавляется. Если стек закончился раньше, чем мы встретили открывающую скобку, это означает, что в выражении либо неверно поставлен разделитель, либо не согласованы скобки.
            Если существуют разные виды скобок, появление непарной скобки также свидетельствует об ошибке. Если какие-то скобки одновременно являются функциями (например, [x] — целая часть), добавляем к выходной строке символ этой функции.
        Если символ является бинарной операцией о1, тогда:
        1) пока на вершине стека префиксная функция…
            … ИЛИ операция на вершине стека приоритетнее o1
            … ИЛИ операция на вершине стека левоассоциативная с приоритетом как у o1
            … выталкиваем верхний элемент стека в выходную строку;
        2) помещаем операцию o1 в стек.
     *  Когда входная строка закончилась, выталкиваем все символы из стека в выходную строку. В стеке должны были остаться только символы операций; если это не так, значит в выражении не согласованы скобки.
     * */

    vector<string> res;
    stack<string> opStack;

    for (const auto& elem: lexStrings)
    {
        if (elem == "(")
            opStack.push(elem); //Открывающуюся строку в стек
        else if (elem == ")")
        {
            while (!opStack.empty() && opStack.top() != "(")
            {
                res.push_back(opStack.top());   //Операции в строку пока не встретили открывающуюся скобку
                opStack.pop();
            }
            if (!opStack.empty())
                opStack.pop();
            else throw WrongRPNConvertException();  //появление непарной скобки также свидетельствует об ошибке
        }
        else if (elem.size() == 1 && getPriorityOperatior(elem[0]) != priority::NAO) //Если размер строки 1 и этот элемент операция
        {
            string op = elem;
            while (!opStack.empty() && (getPriorityOperatior(opStack.top()[0]) >= getPriorityOperatior(op[0])))
            {
                res.push_back(opStack.top());
                opStack.pop();
            }
            opStack.push(op);
        }
        else
        {
            res.push_back(elem);    //Число
        }
    }

    while (!opStack.empty())
    {
        res.push_back(opStack.top());
        opStack.pop();
    }

    return res;
}

void MClient::buildExpTree(const std::vector<std::string> &rpnStrExp) {
    stack<IExpression*> calcStack;

    for(const auto& elem: rpnStrExp)
    {
        if (elem.size() == 1 && getPriorityOperatior(elem[0]) != priority::NAO)
        {
            if (calcStack.size() >= 2)
            {
                IExpression* op1 = calcStack.top(); calcStack.pop();
                IExpression* op2 = calcStack.top(); calcStack.pop();

                IExpression* expr = getVar(elem[0], op1, op2);
                calcStack.push(expr);
            }
            else throw WrongBuildingTreeException();
        }
        else if (isalpha(elem[0]) || elem == "$")
        {
            if (_ctx->isExist(elem))
                calcStack.push(new ConstantExp(_ctx->operator[](elem)));
            else throw WrongBuildingTreeException();
        }
        else if (isdigit(elem[0]))
        {
            Error e = Error::noError;
            int value = StrToInteger(elem, e); // Парсим число
            if (e == Error::noError)
                calcStack.push(new ConstantExp(value));
            else throw WrongBuildingTreeException();
        }
    }
    if (calcStack.size() == 1)
    {
        unique_ptr<IExpression> tmp_ptr(calcStack.top());
        _treeExp = move(tmp_ptr);
    }
    else throw WrongBuildingTreeException();
}

int MClient::Execute() {
    return _treeExp->Evaluate(*_ctx);
}
