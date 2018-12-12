#ifndef SYSTEMSOFTWARE1_COMMANDS_H
#define SYSTEMSOFTWARE1_COMMANDS_H

#include <iostream>
#include "Type.h"
#include <functional>

class Command {
protected:
    address GetAddress(const Registers &regs) const noexcept
    {
        return regs.ExecuteCommand.b==1?regs.ExecuteCommand.addr + regs.AddrReg: regs.ExecuteCommand.addr;
    }
public:
    ~Command() = default;
    virtual void operator()(Memory &mem, Registers &regs) const noexcept = 0;
};

class MemoryModCommand: public Command
{
protected:
    data GetData(const Memory &mem, address a) const noexcept
    {
        MemoryUnion result{};
        for (int i = a, k = 0; i < a + sizeof(result); ++i, ++k)
            result.bytes[k] = mem[i];
        return result.dat;
    }

    void LoadData(Memory &mem, address a, data dat) const noexcept
    {
        MemoryUnion memUnion{dat};
        for (int i = a, k = 0; i < a + sizeof(dat); ++i, ++k)
            mem[i] = memUnion.bytes[k];
    }
};

class LoadString: public  MemoryModCommand
{
    void operator()(Memory &mem, Registers &regs) const noexcept override {
        std::string tmp{};
        std::cout << std::endl;
        std::cin >> tmp;
        data sz_str{};
        sz_str.integer = static_cast<int>(tmp.size());
        address  loadStrStart = GetAddress(regs);
        LoadData(mem, GetAddress(regs), sz_str); //Запишем в 4 байта количество символов
        //Вычислим адрес  записи символов
        loadStrStart += sizeof(data);
        for (int i = 0; i < sz_str.integer; ++i) {
            mem[i + loadStrStart] = static_cast<unsigned char>(tmp[i]);
        }
    }
};

class PrintString: public MemoryModCommand
{
    void operator()(Memory &mem, Registers &regs) const noexcept override {
        data sz_str{};
        address  printStrStart = GetAddress(regs);
        sz_str = GetData(mem, printStrStart);   //Получим 4 байта(размер)
        //Вычислим адрес для чтения
        printStrStart += sizeof(data);
        std::cout << std::endl;
        for (int i = 0; i < sz_str.integer; ++i)
            std::cout << mem[i + printStrStart];
    }
};

class  Arithmetic: public MemoryModCommand
{
protected:
    void operator()(Memory &mem, Registers &regs) const noexcept override {
        data tmp = GetData(mem, GetAddress(regs));
        Operation(regs, regs.SummatorReg, tmp);
        PostOperation(regs);
    }
    virtual void Operation(Registers &erorr, data &save, data value) const noexcept = 0;
    virtual void PostOperation(Registers &save) const noexcept = 0;
};

class RealArithmetic:  public Arithmetic
{
protected:
    void FlagR(Registers &regs) const noexcept
    {
        if(fabsf(regs.SummatorReg.real) < eps)
            regs.PSW.CF = 0;
        else if(regs.SummatorReg.real < 0.0f)
            regs.PSW.CF = -1;
        else
            regs.PSW.CF = 1;
    }

public:
    void PostOperation(Registers &save) const noexcept override
    {
        FlagR(save);
    }
};

class IntegerArithmetic: public Arithmetic
{
protected:
    void FlagI(Registers &regs) const noexcept
    {
        if(regs.SummatorReg.integer == 0)
            regs.PSW.CF = 0;
        else if(regs.SummatorReg.integer < 0)
            regs.PSW.CF = -1;
        else
            regs.PSW.CF = 1;
    }
public:
    void PostOperation(Registers &save) const noexcept override
    {
        FlagI(save);
    }
};

class Load : public MemoryModCommand
{
public:
    void operator()(Memory &mem, Registers &regs) const noexcept override
    {
        regs.SummatorReg = GetData(mem,  GetAddress(regs));
    }
};

class Store : public MemoryModCommand
{
public:
    void operator()(Memory &mem, Registers &regs) const noexcept override
    {
        LoadData(mem, GetAddress(regs), regs.SummatorReg);
    }
};

class GO : public Command
{
public:
    void operator()(Memory &mem, Registers &regs) const noexcept override {
        regs.PSW.IP = GetAddress(regs);
        regs.PSW.JP = 1;
    }
};

class JZ : public Command
{
public:
    void operator()(Memory &mem, Registers &regs) const noexcept override {
        if(regs.PSW.CF == 0){
            regs.PSW.IP = GetAddress(regs);
            regs.PSW.JP = 1;
        }
    }
};

class JG : public Command
{
public:
    void operator()(Memory &mem, Registers &regs) const noexcept override {
        if(regs.PSW.CF == 1)
        {
            regs.PSW.IP = GetAddress(regs);
            regs.PSW.JP = 1;
        }
    }
};

class JL : public Command
{
public:
    void operator()(Memory &mem, Registers &regs) const noexcept override {
        if(regs.PSW.CF == -1){
            regs.PSW.IP = GetAddress(regs);
            regs.PSW.JP = 1;
        }
    }
};

class ShiftR : public MemoryModCommand
{
public:
    void operator()(Memory &mem, Registers &regs) const noexcept override {
        data tmp = GetData(mem, GetAddress(regs));
        regs.SummatorReg.integer >>= tmp.integer;
    }
};

class ShiftL : public MemoryModCommand
{
public:
    void operator()(Memory &mem, Registers &regs) const noexcept override {
        data tmp = GetData(mem, GetAddress(regs));
        regs.SummatorReg.integer <<= tmp.integer;
    }
};

class And: public MemoryModCommand
{
public:
    void operator()(Memory &mem, Registers &regs) const noexcept override {
        data tmp = GetData(mem, GetAddress(regs));
        regs.SummatorReg.integer &= tmp.integer;
    }
};

class Or: public MemoryModCommand
{
public:
    void operator()(Memory &mem, Registers &regs) const noexcept override {
        data tmp = GetData(mem, GetAddress(regs));
        regs.SummatorReg.integer |= tmp.integer;
    }
};

class Xor: public MemoryModCommand
{
public:
    void operator()(Memory &mem, Registers &regs) const noexcept override {
        data tmp = GetData(mem, GetAddress(regs));
        regs.SummatorReg.integer ^= tmp.integer;
    }
};

class Not: public MemoryModCommand
{
public:
    void operator()(Memory &mem, Registers &regs) const noexcept override {
        regs.SummatorReg.integer = ~regs.SummatorReg.integer;
    }
};

class Inki : public IntegerArithmetic
{
public:
    void Operation(Registers &erorr, data &save, data value) const noexcept override
    {
        ++save.integer;
    }
};

class Ret : public Command
{
public:
    void operator()(Memory &mem, Registers &regs) const noexcept override {
        if(!regs.addrStack.empty()) {
            regs.PSW.IP = regs.addrStack.top();
            regs.addrStack.pop();
        }
        else
        {
            regs.PSW.ER = 1;    //Вовзращать нечего
        }
    }
};


class Call : public Command
{
public:
    void operator()(Memory &mem, Registers &regs) const noexcept override {
        regs.addrStack.push(regs.PSW.IP);
        regs.PSW.IP = GetAddress(regs);
        regs.PSW.JP = 1;
    }
};


class Deci : public IntegerArithmetic
{
public:
    void Operation(Registers &erorr, data &save, data value) const noexcept override
    {
        --save.integer;
    }
};

class Laddr : public MemoryModCommand
{
public:
    void operator()(Memory &mem, Registers &regs) const noexcept override {
        regs.AddrReg = GetAddress(regs);
    }
};

class StIR : public MemoryModCommand
{
public:
    void operator()(Memory &mem, Registers &regs) const noexcept override {
        data tmpReal{};
        tmpReal.real = regs.SummatorReg.integer;
        LoadData(mem, GetAddress(regs), tmpReal);
    }
};

class StRI : public MemoryModCommand
{
public:
    void operator()(Memory &mem, Registers &regs) const noexcept override {
        data tmpInteger{};
        tmpInteger.integer = static_cast<int>(regs.SummatorReg.real);
        LoadData(mem, GetAddress(regs), tmpInteger);
    }
};


class LdIR : public MemoryModCommand
{
public:
    void operator()(Memory &mem, Registers &regs) const noexcept override {
        data tmp = GetData(mem, GetAddress(regs));
        regs.SummatorReg.real = tmp.integer;
    }
};

class LdRI : public MemoryModCommand
{
public:
    void operator()(Memory &mem, Registers &regs) const noexcept override {
        data tmp = GetData(mem, GetAddress(regs));
        regs.SummatorReg.integer = static_cast<int>(tmp.real);
    }
};

class ROut : public MemoryModCommand
{
public:
    void operator()(Memory &mem, Registers &regs) const noexcept override {
        data tmp = GetData(mem, GetAddress(regs));
        std::cout << std::endl << tmp.real;
    }
};


class RIn : public MemoryModCommand
{
public:
    void operator()(Memory &mem, Registers &regs) const noexcept override {
        data tmp{};
        std::cout << std::endl;
        std::cin >> tmp.real;
        LoadData(mem, GetAddress(regs), tmp);
    }

};

class RCmp : public MemoryModCommand
{
public:
    void operator()(Memory &mem, Registers &regs) const noexcept override {
        data val = GetData(mem, GetAddress(regs));

        if(fabsf(regs.SummatorReg.real - val.real) < eps)
            regs.PSW.CF = 0;
        else if(regs.SummatorReg.real < val.real)
            regs.PSW.CF = -1;
        else
            regs.PSW.CF = 1;
    }
};

class RDiv : public RealArithmetic
{
public:
    void Operation(Registers &erorr, data &save, data value) const noexcept override
    {
        if(fabsf(value.real) < eps) erorr.PSW.ER = 1;
        else save.real /= value.real;
    }
};

class RMul : public RealArithmetic
{
public:
    void Operation(Registers &erorr, data &save, data value) const noexcept override
    {
        save.real *= value.real;
    }
};

class RSub : public RealArithmetic
{
public:
    void Operation(Registers &erorr, data &save, data value) const noexcept override
    {
        save.real -= value.real;
    }
};

class RAdd : public RealArithmetic
{
public:
    void Operation(Registers &erorr, data &save, data value) const noexcept override
    {
        save.real += value.real;
    }
};

class IOut : public MemoryModCommand
{
public:
    void operator()(Memory &mem, Registers &regs) const noexcept override {
        data tmp = GetData(mem, GetAddress(regs));
        std::cout << std::endl << tmp.integer;
    }
};

class IIn : public MemoryModCommand
{
public:
    void operator()(Memory &mem, Registers &regs) const noexcept override {
        data tmp{};
        std::cout << std::endl;
        std::cin >> tmp.integer;
        LoadData(mem, GetAddress(regs), tmp);
    }
};

class ICmp : public MemoryModCommand
{
public:
    void operator()(Memory &mem, Registers &regs) const noexcept override {
        data val = GetData(mem, GetAddress(regs));
        if(regs.SummatorReg.integer == val.integer)
            regs.PSW.CF = 0;
        else if(regs.SummatorReg.integer < val.integer)
            regs.PSW.CF = -1;
        else
            regs.PSW.CF = 1;
    }
};

class IMod : public IntegerArithmetic
{
public:
    void Operation(Registers &erorr, data &save, data value) const noexcept override
    {
        if(value.integer == 0) erorr.PSW.ER = 1;
        else save.integer %= value.integer;
    }
};

class IDiv : public IntegerArithmetic
{
public:
    void Operation(Registers &erorr, data &save, data value) const noexcept override
    {
        if(value.integer == 0) erorr.PSW.ER = 1;
        else save.integer /= value.integer;
    }
};

class IMul : public IntegerArithmetic
{
public:
    void Operation(Registers &erorr, data &save, data value) const noexcept override
    {
        save.integer *= value.integer;
    }
};

class ISub : public IntegerArithmetic
{
public:
    void Operation(Registers &erorr, data &save, data value) const noexcept override
    {
        save.integer -= value.integer;
    }
};

class IAdd : public IntegerArithmetic
{
public:
    void Operation(Registers &erorr, data &save, data value) const noexcept override
    {
        save.integer += value.integer;
    }
};

class Stop : public Command
{
public:
    void operator()(Memory &mem, Registers &regs) const noexcept override {
        regs.PSW.EF = 1;
    }

};

#endif //SYSTEMSOFTWARE1_COMMANDS_H
