#ifndef SYSTEMSOFTWARE2_ASSEBLER_H
#define SYSTEMSOFTWARE2_ASSEBLER_H


#include <string>
#include <memory>
#include "Operator.h"
#include "OpCodes.h"
#include "MClient.h"
class Assembler {
private:
//====Данные
    //Размер команды( Нет доступа к сборке как в .NET =( )
    static constexpr int CmdSize = 3;
    //Заполненые опера
    std::vector<Operator> _program;
    //Контекст(Таблица имён)
    Context _namesTable;
    //Таблица операторов и директив
    std::unordered_map<std::string, operation> _asmOperators;
//====Данные!

//====Методы
    //Инициализация данных
    void init() noexcept;
    //Первых проход(пооперационный)
    void firstPass(Operator &op, int &LC);
    //Второй проход
    void secondPass();
    //Получим операцию из строки(Конечный автомат)
    static Operator getOperator(const std::string &parsString);
    //Напишем свой сплит, т.к. подключать Boost ради сплита излишне
    static std::vector<std::string> split(const std::string& s, char delimiter) noexcept;
    //Удаление пробельных символов
    static void deleteSpaces(std::string& s) noexcept;
    //Трим начиная с начала
    static inline void ltrim(std::string &s);
    //Трим с конца
    static inline void rtrim(std::string &s);
    //Трим с начала и с конца
    static inline void trim(std::string &s);
    //Определние префикса по тип
    template <typename T>
    static Prefix PrefixByType();
//====Методы!

//====Функции для операций
    template <size_t countArguments, byte OffsetBit = 0>
    static void CmdValid(Operator &op, Context &ctx);  //Валидация команды
    template <char prefix>
    static void chAddr(Operator &op, Context &ctx);    //Обработичк команд изменяющих счётчик размещения
    template <typename T>
    static void defDat(Operator &op, Context &ctx);    //Обработчик команд определения данных(Все команды кроме строк)
    template <typename T>
    static void memDat(Operator &op, Context &ctx);    //Обработчик команд объявления данных
//====Функции для операций!

//====Директивы
    static void equ(Operator &op, Context &ctx);
    static void proc(Operator &op, Context &ctx);
    static void endproc(Operator &op, Context &ctx);
//====Директивы!



public:
//====Интерфейс ассемблера
    //Входной файл
    explicit Assembler(const std::string &iFile);
    //Исполняеммый модуль
    void createExecute(const std::string &oFile);
    //Листинг
    void createListing(const std::string &lFile);
    //Имеются ли ошибки??? Поможет при завершении первого и второго прохода
    bool isExistError();
//====Интерфейс ассемблера!

//====Исключения
    class NoFileException: public std::exception
    {
    public:
        const char *what() const noexcept {
            return "No Such File!!!";
        }
    };
//====Исключения!

};


#endif //SYSTEMSOFTWARE2_ASSEBLER_H
