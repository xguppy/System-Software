#ifndef SYSTEMSOFTWARE2_ERRORS_H
#define SYSTEMSOFTWARE2_ERRORS_H

#include <exception>
#include <cinttypes>

enum class Error: uint8_t {
    noError = 0,
    //Ошибки  предварительной обработки
    illSymbol,					// недопустимый символ в одном из полей
    noColon,                    // нет двоеточия после метки
    illOperator,                // неправильный формат оператора
                                // ошибки выражений
    illString,                  // ошибка в строке
    illInteger,					// ошибка в целом числе
    illExpression,				// неверный синтаксис выражения
    undLabel,					// неизвестное имя в выражении
                                // общие ошибки оператора
    reuLabel,					// повторно определенная метка
    illCountArgument,		    // неправильное количество аргументов
    illRealValue,               // неверный формат вещественного числа
    //Ошибки  предварительной обработки!

    //====Ошибки директив
    noLabel,					// нет метки в equ, proc, data

    noEnd,						// нет директивы end!
    //====Ошибки директив!

};

#endif
