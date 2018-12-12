#ifndef SYSTEMSOFTWARE2_MCLIENT_H
#define SYSTEMSOFTWARE2_MCLIENT_H


#include <string>
#include <memory>
#include <vector>
#include "MParser.h"
#include "Errors.h"

class MClient final {
    std::unique_ptr<IExpression> _treeExp;   //Клиент занимается построением дерева
    std::shared_ptr<Context> _ctx;    //Указатель на контекст(Не является владельцем)

    void buildExpTree(const std::vector<std::string> &rpnStrExp);                          //Построение дерева в _treeExp
    IExpression* getVar(char ch, IExpression* op1, IExpression* op2);                      //Возвращает указатель с операцией
    std::vector<std::string> lexAnalyze(const std::string &expression);                    //Разбиение строки на лексемы
    std::vector<std::string> getRPN(const std::vector<std::string> &lexStrings);           //Получение обратной польской нотации
    static int8_t getPriorityOperatior(char ch) noexcept;                                  //Получение приоритета, если не является оператором возвращает -1
public:
    MClient(std::string expString, Context &ctx, Error &e);
    int Execute();  //Вычислить

    class WrongLexException: public std::exception
    {
    public:
        const char *what() const noexcept {
            return "Wrong Lexem!!!";
        }
    };

    class WrongRPNConvertException: public std::exception
    {
    public:
        const char *what() const noexcept {
            return "Wrong loigic!!!";
        }
    };

    class WrongBuildingTreeException: public std::exception
    {
    public:
        const char *what() const noexcept {
            return "Something went wrong in building tree!!!";
        }
    };

};

#endif //SYSTEMSOFTWARE2_MCLIENT_H
