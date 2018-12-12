#ifndef SYSTEMSOFTWARE2_OPERATOR_H
#define SYSTEMSOFTWARE2_OPERATOR_H

#include <string>
#include <vector>
#include <functional>
#include "Errors.h"
#include "MParser.h"

typedef unsigned char byte;		// для сокращения записи
typedef unsigned short address;		// для сокращения записи
static constexpr char LcSimbol[] = "$";   //Символьное отображение счётчика размещения
                                        //(возможно использовать string_view вместо char[] нужен C++17)
//Оператор как описанно в книге
struct Operator
{
    bool work;				    // подлежит ли обработке
    int LC;				        // смещение - счетчик размещения
    std::vector<byte> Binary;	// двоичная команда
    int number;				    // номер исходной строки
    std::string Label;			// метка оператора
    std::string Code;			// код операции или директивы
    std::vector<std::string> Arguments;		// поле аргументов
    std::string Comment;		// комметарий (для листинга)
    Error nError = Error::noError;		        // номер ошибки
};

//Операция как описано в книге(Только убрал указатель на функцию)
struct operation
{
    std::function<void(Operator&, Context&)> function;           // транслирующая функция
    byte Code;                                                   // у директив = 0
    byte Length;                                                 // у директив = 0
};

//Для перевода данных в исполняемый(бинарный) файл
union data
{
    int integer;
    float real;
    byte bytes[4];
};

#endif //SYSTEMSOFTWARE2_OPERATOR_H
