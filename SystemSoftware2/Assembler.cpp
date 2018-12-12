#include <exception>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include "Assembler.h"
#include "StrToDate.h"
#include "Listing.h"

using namespace std;

//====Перечисления для конечного автомата
//Состояния
enum States: uint8_t//9  состояний
{
    Start,                // Стартовое состояние
    WaitingOperations,     // Ожидание кода операции
    Label,                // Ввод метки
    NoColon,              // Отсутствие символа :
    CodeOperations,        // Ввод кода операции
    WaitingAruments,      // Ожидание аргументов
    Arg,                  // Ввод аргумента
    End,                  // Конечное состояние
    ErrSt                 // Состояние ошибки
};
//Сигналы
enum TypeChar: uint8_t//7 сигналов
{
    Blank,      // Пробелы
    Colon,      // Двоеточия
    Comment,    // Комментарии
    Digit,      // Цифры
    Id,         // Идентификаторы
    Underline,  // Подчеркивания
    Arguments,  // Аргументы
};

TypeChar toTypeChar(char ch)
{
    if(ch == ':') return TypeChar::Colon;
    if(ch == ';') return TypeChar::Comment;
    if(ch == '_') return TypeChar::Underline;
    if(isdigit(ch)) return TypeChar::Digit;
    if(isspace(ch)) return TypeChar::Blank;
    if(isalpha(ch)) return TypeChar::Id;
    return TypeChar::Arguments;
}
//====Перечисления для конечного автомата!

Assembler::Assembler(const std::string &iFile)
{
    init();                 //Иницциализация таблиц
    //Ассемблерный код
    ifstream asmFileStream(iFile);

    int LC = 0;
    bool ifEnd = false;
    int line = 0;
    if(asmFileStream.is_open())
    {
        Operator curOperator;
        while (!asmFileStream.eof())
        {
            string curLine; //Ассемблер построчный читаем  строку
            getline(asmFileStream,  curLine);

            curOperator = getOperator(curLine); //Преобразуем строку в оператор

            if(curOperator.nError != Error::noError || (curOperator.Code.empty() && curOperator.Label.empty())) curOperator.work = false;

            firstPass(curOperator, LC);

            if (curOperator.Code == "end" && curOperator.nError == Error::noError)
                ifEnd = true;   //Если нашли конец
            curOperator.number = ++line;
            _program.push_back(curOperator);

        }

        if(!ifEnd)
        {
            curOperator.nError = Error::noEnd;
        }

        if (!isExistError())
            secondPass();


    } else throw NoFileException();
}

template<>
void Assembler::defDat<string>(Operator &op, Context &ctx)     //Обработчик команд определения данных(Строки спец. шаблон)
{

    Error err; // Текущее значение и наличие ошибки на текущем обрабатываемом аргументе директивы

    auto prefix = PrefixByType<std::string>(); // Префикс = начальный символ

    err = Error::noError;

    string curStr = parseSimpleStr(op.Arguments[0], err);


    if (err == Error::noError) // Если не было ошибки перевода чисел
    {
        op.Binary.push_back(prefix);
        data sizeStr{};
        sizeStr.integer = curStr.size();
        for (const byte &var : sizeStr.bytes) {
            op.Binary.push_back(var);
        }
        for (const auto &i : curStr) {
            op.Binary.push_back((uint8_t) i);
        }
        ctx[LcSimbol] += curStr.size() + sizeof(int);
    }
    else op.nError = err;
}


void Assembler::init() noexcept {
    //Инициализация переменных
    _namesTable.Assign(new  VariableExp("SINT"), sizeof(int));
    _namesTable.Assign(new  VariableExp("SREAL"), sizeof(float));
    _namesTable.Assign(new VariableExp("SCMD"), CmdSize);
    /*Счетчик адреса — специфический вид операнда. Он обозначается знаком $.
     Специфика этого операнда в том, что когда транслятор ассемблера встречает в исходной программе этот символ,
     то он подставляет вместо него текущее значение счетчика адреса. Значение счетчика адреса, или, как его иногда называют,
     счетчика размещения, представляет собой смещение текущей машинной команды относительно начала сегмента кода.
     В формате листинга счетчику адреса соответствует
     вторая или третья колонка (в зависимости от того, присутствует или нет в листинге колонка с уровнем вложенности).*/
    _namesTable.Assign(new VariableExp(LcSimbol), 0);

    //====Define Data
    _asmOperators["int"] = {defDat<int>, 0, 0};
    _asmOperators["real"] = {defDat<float>, 0, 0};
    _asmOperators["str"] = {defDat<string>, 0, 0};

    _asmOperators["memint"] = {memDat<int>, 0, 0};
    _asmOperators["memreal"] = {memDat<float>, 0, 0};
    _asmOperators["memstr"] = {memDat<uint8_t>, 0, 0};
    //====Define Data!

    //====Directives
    _asmOperators["equ"] = {equ, 0, 0};
    _asmOperators["proc"] = {proc, 0, 0};
    _asmOperators["endproc"] = {endproc, 0, 0};

    _asmOperators["org"] = {chAddr<Prefix::loadAddr>, 0, 0};
    _asmOperators["end"] = {chAddr<Prefix::start>, 0, 0};
    //====Directives!

    //====Transmisson
    _asmOperators["load"] = {CmdValid<1>, operations::load, CmdSize};
    _asmOperators["load!"] = {CmdValid<1, 1>, operations::load, CmdSize};
    _asmOperators["loadir"] = {CmdValid<1>, operations::ldIR, CmdSize};
    _asmOperators["loadir!"] = {CmdValid<1, 1>, operations::ldIR, CmdSize};
    _asmOperators["loadri"] = {CmdValid<1>, operations::ldRI, CmdSize};
    _asmOperators["loadri!"] = {CmdValid<1, 1>, operations::ldRI, CmdSize};

    _asmOperators["store"] = {CmdValid<1>, operations::stores, CmdSize};
    _asmOperators["store!"] = {CmdValid<1, 1>, operations::stores, CmdSize};
    _asmOperators["storeir"] = {CmdValid<1>, operations::stIR, CmdSize};
    _asmOperators["storeir!"] = {CmdValid<1, 1>, operations::stIR, CmdSize};
    _asmOperators["storeri"] = {CmdValid<1>, operations::stRI, CmdSize};
    _asmOperators["storeri!"] = {CmdValid<1, 1>, operations::stRI, CmdSize};
    //====Transmisson!

    //====Math
    _asmOperators["iadd"] = {CmdValid<1>, operations::iadd, CmdSize};
    _asmOperators["isub"] = {CmdValid<1>, operations::isub, CmdSize};
    _asmOperators["imul"] = {CmdValid<1>, operations::imul, CmdSize};
    _asmOperators["idiv"] = {CmdValid<1>, operations::idiv, CmdSize};
    _asmOperators["imod"] = {CmdValid<1>, operations::imod, CmdSize};
    _asmOperators["lshift"] = {CmdValid<1>, operations::shiftL, CmdSize};
    _asmOperators["rshift"] = {CmdValid<1>, operations::shiftR, CmdSize};
    _asmOperators["ink"] = {CmdValid<0>, operations::inki, CmdSize};
    _asmOperators["dec"] = {CmdValid<0>, operations::deci, CmdSize};

    _asmOperators["or"] = {CmdValid<1>, operations::ior, CmdSize};
    _asmOperators["xor"] = {CmdValid<1>, operations::ixor, CmdSize};
    _asmOperators["and"] = {CmdValid<1>, operations::iand, CmdSize};
    _asmOperators["not"] = {CmdValid<1>, operations::inot, CmdSize};

    _asmOperators["radd"] = {CmdValid<1>, operations::radd, CmdSize};
    _asmOperators["rsub"] = {CmdValid<1>, operations::rsub, CmdSize};
    _asmOperators["rmul"] = {CmdValid<1>, operations::rmul, CmdSize};
    _asmOperators["rdiv"] = {CmdValid<1>, operations::rdiv, CmdSize};
    //====Math!

    //====IO
    _asmOperators["iin"] = {CmdValid<1>, operations::iin, CmdSize};
    _asmOperators["rin"] = {CmdValid<1>, operations::rin, CmdSize};

    _asmOperators["iout"] = {CmdValid<1>, operations::iout, CmdSize};
    _asmOperators["rout"] = {CmdValid<1>, operations::rout, CmdSize};
    //====IO!

    //====Jump
    _asmOperators["go"] = {CmdValid<1>, operations::go, CmdSize};
    _asmOperators["go!"] = {CmdValid<1,1>, operations::go, CmdSize};

    _asmOperators["jz"] = {CmdValid<1>, operations::jz, CmdSize};
    _asmOperators["jz!"] = {CmdValid<1, 1>, operations::jz, CmdSize};
    _asmOperators["jl"] = {CmdValid<1>, operations::jl, CmdSize};
    _asmOperators["jl!"] = {CmdValid<1, 1>, operations::jl, CmdSize};
    _asmOperators["jg"] = {CmdValid<1>, operations::jg, CmdSize};
    _asmOperators["jg!"] = {CmdValid<1, 1>, operations::jg, CmdSize};

    _asmOperators["call"] = {CmdValid<1>, operations::call, CmdSize};
    _asmOperators["ret"] = {CmdValid<0>, operations::ret, CmdSize};
    _asmOperators["stop"] =  {CmdValid<0>, operations::stop, CmdSize};
    //====Jump!

    //====Compare
    _asmOperators["icmp"] = {CmdValid<1>, operations::icmp, CmdSize};
    _asmOperators["rcmp"] = {CmdValid<1>, operations::rcmp, CmdSize};
    //====Compare!

    //====String
    _asmOperators["loadstr"] = {CmdValid<1>, operations::ldStr, CmdSize};
    _asmOperators["printstr"] = {CmdValid<1>, operations::prStr, CmdSize};
    //====String!

}

Operator Assembler::getOperator(const string &parsString) {
    //Будем использовать перечисления и матрицу состояний
    vector<vector<States>> statesMatrix
    {
        //В начале у нас состояние Start
        {States::WaitingOperations, States::ErrSt, States::End, States::ErrSt, States::Label, States::Label, States::ErrSt},
        //Пробел->ждём,Двоеточие->ошибка,Комментарий->конец,Цифра->ошибка,Идентификатор->метка,Подчёркивание->метка,Любой->ошибка
        //Ждём операции
        {States::WaitingOperations, States::ErrSt, States::End, States::ErrSt, States::CodeOperations, States::CodeOperations, States::ErrSt},
        //Пробел->ждём,Двоеточие->ошибка,Комментарий->конец,Цифра->ошибка,Идентификатор->метка,Подчёркивание->метка,Любой->ошибка
        //Метка
        {States::NoColon, States::WaitingOperations, States::End, States::Label, States::Label, States::Label, States::ErrSt},
        //Пробел->Нет :,Двоеточие->Ждём операторов,Комментарий->конец,Цифра->метка,Идентификатор->метка,Подчёркивание->метка,Любой->ошибка
        //Нет ":"
        {States::ErrSt, States::ErrSt, States::ErrSt, States::ErrSt, States::ErrSt, States::ErrSt, States::ErrSt},
        //Из ошибочного состояния отсутсвия ':' попадём только в ошибку
        //Код операции
        {States::WaitingAruments, States::ErrSt, States::End, States::ErrSt, States::CodeOperations, States::ErrSt, States::ErrSt},
        //Пробел->ждём аргументы, Двоеточие->ошибка, Комментарий->конец, Цифра->ошибка, Идентификатор->код операции, Подчёркивание->ошибка , Любой->ошибка
        //Ожидание аргументов
        {States::WaitingAruments, States::ErrSt, States::End,  States::Arg, States::Arg, States::Arg, States::Arg},
        //Пробел->ждём, Двоеточие->ошибка, Комментарий->конец, Цифра->аргумент, Идентификатор->аргумент, Подчёркивание->аргумент , Любой->аргумент
        //Ввод аргументов
        {States::Arg, States::ErrSt, States::End,  States::Arg, States::Arg, States::Arg, States::Arg},
        //Пробел->ждём, Двоеточие->ошибка, Комментарий->конец, Цифра->аргумент, Идентификатор->аргумент, Подчёркивание->аргумент , Любой->аргумент
        //Конечное состояние
        {States::End, States::End, States::End, States::End, States::End, States::End, States::End},
        //Пробел->конец, Двоеточие->конец, Комментарий->конец, Цифра->конец, Идентификатор->конец, Подчёркивание->конец , Любой->конец
        //Состояние ошибки
        {States::End, States::End, States::End, States::End, States::End, States::End, States::End}
        //Конец(для наполнения, не обязательно)
    };

    int i = 0, sz = parsString.size();
    States curState = States::Start;
    Operator assemblyOperator{};
    string curStrParse{};
    TypeChar chType;
    assemblyOperator.work = true;
    while(i < sz)
    {
        chType = toTypeChar(parsString[i]);
        States prev = curState;
        curState = statesMatrix[curState][chType];
        switch(prev)
        {
            case States::Start:
                if(curState == States::Label)
                {
                    curStrParse += parsString[i];
                }
                else if(curState == States::End)
                {
                    assemblyOperator.Comment = parsString;
                    assemblyOperator.work = false;
                }
                break;
            case States::WaitingOperations:
                if(curState == States::CodeOperations)
                {
                    curStrParse += parsString[i];
                }
                else if(curState == States::End)
                {
                    assemblyOperator.Comment = curStrParse;
                    assemblyOperator.work = !assemblyOperator.Label.empty();
                }
                break;
            case States::Label:
                if(curState == States::Label)
                {
                    curStrParse += parsString[i];
                }
                else if(curState == States::WaitingOperations || curState == States::NoColon)
                {
                    assemblyOperator.Label  = curStrParse;
                    curStrParse = {};
                }
                break;
            case States::CodeOperations:
                if(curState == States::CodeOperations)
                {
                    curStrParse += parsString[i];
                }
                else if(curState == States::WaitingAruments)
                {
                    assemblyOperator.Code = curStrParse;
                    curStrParse = {};
                }
                else if(curState == States::End)
                {
                    assemblyOperator.Code = curStrParse;
                    assemblyOperator.Comment = parsString.substr(i);
                    curStrParse = {};
                }
                else if(curState == States::Arg)
                {
                    curStrParse = parsString[i];
                }
                break;
            case States::WaitingAruments:
                if(curState == States::End)
                {
                    assemblyOperator.Comment = parsString.substr(i);
                }
                else if(curState == States::Arg)
                {
                    curStrParse = parsString[i];
                }
                break;
            case States::Arg:
                if(curState == States::End)
                {
                    assemblyOperator.Arguments = split(curStrParse, ',');
                    curStrParse = {};
                    assemblyOperator.Comment = parsString.substr(i);
                }
                else if(curState == States::Arg)
                {
                    curStrParse += parsString[i];
                }
                break;
            case States::End:
                return assemblyOperator;
            case States::ErrSt:
                assemblyOperator.nError = Error::illSymbol;
                assemblyOperator.work = false;
                assemblyOperator.Comment = curStrParse;
                return assemblyOperator;
            case States::NoColon:
                assemblyOperator.nError = Error::noColon;
                break;
        }
        ++i;
    }

    if (curState == States::CodeOperations)  assemblyOperator.Code = curStrParse;
    else if (curState == States::Arg) assemblyOperator.Arguments = split(curStrParse, ',');
    else if (curState == States::Label)
    {
        assemblyOperator.nError = Error::illOperator;
        assemblyOperator.work = false;
        assemblyOperator.Comment = curStrParse;
    }

    if (!assemblyOperator.Arguments.empty())    //Если есть аргументы
    {
        if(assemblyOperator.Code != "str")
            for_each(assemblyOperator.Arguments.begin(), assemblyOperator.Arguments.end(), deleteSpaces);
        else
            for_each(assemblyOperator.Arguments.begin(), assemblyOperator.Arguments.end(), trim);
        for (const auto &elem: assemblyOperator.Arguments) {    //роверим ччто аргументы не пусты
            if(elem.empty())  assemblyOperator.nError = Error::illCountArgument;
        }
    }

    return assemblyOperator;
}

std::vector<std::string> Assembler::split(const std::string &s, char delimiter) noexcept {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
        tokens.push_back(token);
    return tokens;
}

void Assembler::deleteSpaces(std::string &s) noexcept {
    size_t i;
    while((i=s.find(' ')) != std::string::npos)
        s.erase(i, 1);
}

void Assembler::equ(Operator &op, Context &ctx) {
   if(op.Label.empty())
        op.nError = Error::noLabel;
    else
    {
        Error isError = Error::noError;
        MClient mcLient(op.Arguments[0], ctx, isError);
        if (isError == Error::noError)
            ctx[op.Label] =  mcLient.Execute();
        else
            op.nError = isError;
    }
}

void Assembler::proc(Operator &op, Context &ctx) {
    if (op.Label.empty())
        op.nError = Error::undLabel;
    else if (!op.Arguments.empty())
        op.nError = Error::illCountArgument;
}

void Assembler::endproc(Operator &op, Context &ctx) {
    string name = op.Arguments[0];
    deleteSpaces(name);
    if (op.Arguments.size() != 1)
        op.nError = Error::illCountArgument;
    else if (!ctx.isExist(name))
        op.nError = Error::undLabel;
}

template<size_t countArguments, byte OffsetBit>
void Assembler::CmdValid(Operator &op, Context &ctx) {
    if(op.Arguments.size() == countArguments)
    {
        if(op.Arguments.size() == 0)
        {
            op.Binary.push_back(OffsetBit);

            for (int i = 0; i < sizeof(address); ++i) {
                op.Binary.push_back(0);
            }
        } else
        {
            for (const auto &var: op.Arguments) {
                Error err = Error::noError;
                MClient calc(var, ctx, err);
                data transit{};
                if (err == Error::noError)
                {
                    transit.integer = calc.Execute();
                    op.Binary.push_back(OffsetBit); //Бит смещения

                    for (int i = 0; i < sizeof(address); ++i) {
                        op.Binary.push_back(transit.bytes[i]);  //Запишем адрес
                    }
                }
                else
                {
                    op.nError = Error::illExpression;
                }
            }
        }
    }
    else
    {
        op.nError = Error::illCountArgument;
    }
}

template<char prefix>
void Assembler::chAddr(Operator &op, Context &ctx) {
    if (op.Arguments.size() != 1)
        op.nError = Error::illCountArgument;
    else
    {
        Error err = Error::noError;
        MClient calc(op.Arguments[0], ctx, err);
        if (err == Error::noError)
        {
            op.Binary.push_back(prefix);
            data b{}; b.integer = calc.Execute();

            for (address i = 0; i < sizeof(address); ++i) {
                op.Binary.push_back(b.bytes[i]);
            }
            if (prefix == Prefix::loadAddr)
                ctx[LcSimbol] = b.integer;
            else
            {
                op.Binary.push_back(Prefix::cmd);
                op.Binary.push_back(0); //STOP
                op.Binary.push_back(0); //BIT
                for (int i = 0; i < sizeof(short); ++i) {
                    op.Binary.push_back(0); //STOP
                }
                ctx[LcSimbol] += CmdSize;
            }
        }
        else op.nError = err;
    }
}

template<typename T>
void Assembler::memDat(Operator &op, Context &ctx) {
    if (op.Arguments.size() != 1)
    {
        op.nError = Error::illCountArgument;
    }
    else
    {
        Error expression = Error::noError;
        MClient calc(op.Arguments[0], ctx, expression);
        int counter = calc.Execute();
        if (expression == Error::noError)
        {
            Prefix prefix = PrefixByType<T>();
            for (size_t i = 0; i < counter; ++i)
            {
                op.Binary.push_back(prefix);
                for (size_t j = 0; j < sizeof(T); ++j)
                    op.Binary.push_back(0);
            }
        }
        else
        {
            op.nError = expression;
        }
        ctx[LcSimbol] = ctx[LcSimbol] + sizeof(T) * counter;
    }
}

template<typename T>
Prefix Assembler::PrefixByType() {
    //Почему этот язык не понимает паттерн  матчинг??? =(
    if(typeid(T) == typeid(int))
    {
        return Prefix::integer;
    }
    else if(typeid(T) == typeid(float))
    {
        return Prefix::real;
    }
    else if(typeid(T) == typeid(std::string))
    {
        return Prefix::vmstring;
    }
}

template<typename T>
void Assembler::defDat(Operator &op, Context &ctx) {

    auto prefix = PrefixByType<T>();
    Error err;
    for (const auto& el: op.Arguments)
    {
        err = Error::noError;
        data b{};
        if (typeid(T) == typeid(float))
        {
            string str = el;
            deleteSpaces(str);
            T current = StrToReal(str, err);
            b.real = current;
        }
        else
        {
            MClient calc(el, ctx, err);
            b.integer = calc.Execute();
        }

        if (err == Error::noError)
        {
            op.Binary.push_back(prefix);
            for (int i = 0; i < sizeof(T); ++i)
            {
                op.Binary.push_back(b.bytes[i]);
            }
        }
        else op.nError = err;
        ctx[LcSimbol] = ctx[LcSimbol] + sizeof(T);
    }
}


void Assembler::firstPass(Operator &op, int &LC) {
    if (op.work)
    {
        auto label = op.Label;
        auto code = op.Code;

        if (!label.empty())
        {
            if (_namesTable.isExist(label))
                op.nError = Error::reuLabel;
            else
            {
                VariableExp* var = new VariableExp(label);
                _namesTable.Assign(var, LC);
            }
        }

        if (!code.empty())
        {
            if (_asmOperators.find(code) != end(_asmOperators))
            {
                operation curOp = _asmOperators[code];
                op.LC = LC;
                if (curOp.Length == 0)
                {
                    curOp.function(op, _namesTable);
                    LC = _namesTable[LcSimbol];
                    op.work = false;
                }
                else
                {
                    LC += curOp.Length;
                }
                _namesTable[LcSimbol] =  LC;
            }
            else
            {
                op.nError = Error::illOperator;
            }
        }
        else
        {
            op.LC = LC; op.work = false;
        }
        if (op.nError != Error::noError)
            op.work = false;
    }
}

void Assembler::secondPass() {
    for (auto& var: _program)
    {
        if (var.work)    //Директивы и пр. не обрабатываем
        {
            //Сборка команды
            operation cmd = _asmOperators[var.Code];
            _namesTable[LcSimbol] = var.LC;
            var.Binary.push_back(Prefix::cmd);
            var.Binary.push_back(cmd.Code);

            cmd.function(var, _namesTable);
            var.work = false;
        }
    }
}

bool Assembler::isExistError() {
    for (const auto &var: _program) {
        if(var.nError != Error::noError) return true;
    }
    return false;
}

void Assembler::createExecute(const std::string &oFile) {
    ofstream execute(oFile, ios::binary);
    if(execute.is_open()) {
        for (const auto &var: _program)
        {
            if (!var.Binary.empty())
            {
                for (const auto &byte: var.Binary)
                {
                    execute.write((char *) &byte, sizeof(char));
                }
            }
        }
    }
    else throw NoFileException();
    execute.close();
}

void Assembler::createListing(const std::string &lFile) {
    Listing lst(lFile, _namesTable, _program);
    lst.WriteListing();
}

void Assembler::ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

void Assembler::rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

void Assembler::trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}



