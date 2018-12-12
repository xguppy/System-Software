#include <fstream>
#include <iomanip>
#include "Listing.h"

using namespace std;

std::string Listing::getErrorStr(Error error) noexcept {
    switch(error)
    {
        case Error::noError:
            return "Нет ошибок";
        case Error::illSymbol:
            return "Недопустимый символ в одном из полей";
        case Error::noColon:
            return "Нет двоеточия после метки";
        case Error::illOperator:
            return "Неправильный формат оператора";
        case Error::illString:
            return "Ошибка в строке";
        case Error::illInteger:
            return "Ошибка в целом числе";
        case Error::illExpression:
            return "Неверный синтаксис выражения";
        case Error::undLabel:
            return "Неизвестное имя в выражении";
        case Error::reuLabel:
            return "Повторно определенная метка";
        case Error::illCountArgument:
            return "Неправильное количество аргументов";
        case Error::illRealValue:
            return "Неверный формат вещественного числа";
        case Error::noLabel:
            return "Нет метки в equ, proc, data";
        case Error::noEnd:
            return "Нет директивы end";
    }
}

void Listing::WriteListing() const noexcept {
    ofstream list(_listFile);
    list <<  setw(5) << setfill (' ') << "Строка:" << "  " << setw(5) << setfill (' ') <<  "LC:" << "\t";


    list << setw(10) << setfill(' ') << "Метка:" << "\t\t" << setw(6) << setfill(' ') << "КО:" << "\t\t\t" << setw(15) << setfill(' ') << "Аргументы:" << "\t";
    list << setw(15) << setfill(' ') << "Комментарии:\t\t" << "Ошибки:\n";
    for (const auto& elem: _program)
    {
        list <<  setw(5) << setfill (' ') << elem.number << "  " << setw(5) << setfill ('0') <<  elem.LC << "\t";


        list << setw(10) << setfill(' ') << elem.Label << "\t" << setw(6) << setfill(' ') << elem.Code << "\t\t\t";
        for (size_t i = 0; i < elem.Arguments.size(); ++i)
        {
            list << setw(5) << elem.Arguments[i];
            if (i != elem.Arguments.size() - 1)
                list << ',';
            list << ' ';
        }

        list << "\t\t\t" << setw(15) << setfill(' ') << elem.Comment << "\t\t";

        if(elem.nError != Error::noError)
            list << getErrorStr(elem.nError);

        list << endl;
    }
    list << "Таблица имён:\n";
    for (const auto& elem: _ctx)
            list << setw(10) << "[" << elem.first << "] => {" << elem.second << "}" << endl;
    list.close();
}
