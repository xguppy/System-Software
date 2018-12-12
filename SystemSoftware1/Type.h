#ifndef SYSTEMSOFTWARE1_TYPE_H
#define SYSTEMSOFTWARE1_TYPE_H
#include <vector>
#include <math.h>
#include <limits>
#include <stack>

constexpr float eps = std::numeric_limits<float>::epsilon();

using address = unsigned short;
using byte = unsigned char;
using Memory = std::vector<byte>;

#pragma pack(push,1)
struct command
{
    byte opcode: 7; //Код операции
    byte b: 1;  // Бит смещения
    address addr: 16;    //Адрес
};
//Для размещения данных
union data
{
    int integer;
    float real;
};

//Для размещения и считывания из памяти данных и команд
union MemoryUnion
{
    data dat;
    command cmd;    //В случае с коммандой читать 3 байта
    byte bytes[4];
};

struct Registers
{
    struct {
        address IP;                 //Счётчик комманд
        short CF: 2;                //Флаг сравнения
        unsigned short TF: 1;       //Флаг трассировки
        unsigned short EF: 1;       //Флаг окончания работы процессора
        unsigned short ER: 1;       //Флаг ошибки
        unsigned short JP: 1;       //Флаг перехода, чтобы не увеличивать счётчик
        unsigned short: 10;         //Резерв
    } PSW{};
    address AddrReg{};              //Адресный регистр
    data SummatorReg{};             //Сумматор
    command ExecuteCommand{};       //Собранная комманда
    //Стек вызовов в регистровом файле как в микроконтроллере Intel 8051
    std::stack<address> addrStack;
};

#pragma pack(pop)
#endif //SYSTEMSOFTWARE1_TYPE_H
