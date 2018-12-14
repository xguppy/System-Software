#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "Processor.h"
#include "OpCodes.h"
#include "Type.h"

using namespace std;
Processor::~Processor() noexcept {
    for (auto &i: commands_)
        delete i;
}

Processor &Processor::Instance(uint8_t debuging) noexcept {
    static Processor processor;
    processor.Reset();
    processor.InitCmd();
    processor.memory_.resize(N);
    processor.regs_.PSW.TF = debuging;
    return processor;
}

void Processor::Reset() noexcept {
    regs_.PSW.CF = 0;
    regs_.PSW.EF = 0;
    regs_.PSW.ER = 0;
    regs_.PSW.JP = 0;
    regs_.PSW.BF = 0;
    regs_.AddrReg = 0;
    regs_.SummatorReg.integer = 0;
    for(auto &var: regs_.BreakPoints)
    {
        var.addr = 0;
        var.isOn = 0;
        var.mode = BreakPointsMode::Read;
    }
    memory_.clear();
}

void Processor::Run() noexcept {
    bool isContinue = true;
    while(regs_.PSW.EF != 1 && regs_.PSW.ER != 1 && isContinue) // Если конец, ошибка или точки остановы не прервали выполнения, то выходим
    {
        regs_.PSW.JP = 0;
        if(regs_.PSW.TF != 0)
        {
            cout << Trace().str();  // До выпонения комманды
            logT_ += "\n" + Trace().str();
        }
        regs_.ExecuteCommand = GetCommand(memory_, regs_.PSW.IP);
        if(commands_[regs_.ExecuteCommand.opcode])
            commands_[regs_.ExecuteCommand.opcode]->operator()(memory_, regs_);
        else
            regs_.PSW.ER = 1;
        if(regs_.PSW.TF != 0)
        {
            cout << Trace().str();  // После выполнения комманды
            logT_ += "\n" + Trace().str();
        }
        if(regs_.PSW.JP == 0) regs_.PSW.IP += sizeof(command);
        command nextCmdOpCode = GetCommand(memory_, regs_.PSW.IP);
        for(const auto &var: regs_.BreakPoints)
        {
            if(var.isOn && (var.mode == BreakPointsMode::Read) && (var.addr == nextCmdOpCode.addr) &&
            (cmdMode_[nextCmdOpCode.opcode] & BreakPointsMode::Read) == BreakPointsMode::Read)
                isContinue = false;
            else if(var.isOn && (var.mode == BreakPointsMode::Execute) && (var.addr == regs_.PSW.IP))
                isContinue = false;
            else if(var.isOn && (var.mode == BreakPointsMode::Write) && (var.addr == nextCmdOpCode.addr) &&
                    (cmdMode_[nextCmdOpCode.opcode] & BreakPointsMode::Write) == BreakPointsMode::Write)
                isContinue = false;
        }
    }
}

stringstream Processor::Trace() const noexcept {
    stringstream res;
    res << "Summator: " << std::setw(10) << std::setfill('0') << regs_.SummatorReg.integer << "; "
              << std::setw(12) << regs_.SummatorReg.real << std::endl;
    res << "ExecutingCommand: " << std::setw(10) << std::setfill('0') << regs_.ExecuteCommand.opcode << "; "
              << std::setw(12) << regs_.ExecuteCommand.b << std::setw(12) << regs_.ExecuteCommand.addr << std::endl;
    res << "CF: "<< regs_.PSW.CF << std::endl << "; "
             << "ER: " << regs_.PSW.ER << "; "
            << "EF: "  << regs_.PSW.EF << "; "
            << "JP: "  << regs_.PSW.JP << "; ";
    res << "AdresReg: " << std::setw(4) << std::setfill('0') << regs_.AddrReg << " - next address";
    res << "IP: " << std::setw(4) << std::setfill('0') << regs_.PSW.IP << " - next address";
    return res;
}

void Processor::InitCmd() noexcept {
    commands_.assign(128, nullptr);
    cmdMode_.assign(128, BreakPointsMode::Execute);
    commands_[stop] = new Stop();
    commands_[go] = new GO();
    cmdMode_[go] |= BreakPointsMode::Read;
    commands_[jl] = new JL();
    cmdMode_[jl] |= BreakPointsMode::Read;
    commands_[jg] = new JG();
    cmdMode_[jg] |= BreakPointsMode::Read;
    commands_[jz] = new JZ();
    cmdMode_[jz] |= BreakPointsMode::Read;
    commands_[rcmp] = new RCmp();
    cmdMode_[rcmp] |= BreakPointsMode::Read;
    commands_[rdiv] = new RDiv();
    cmdMode_[rdiv] |= BreakPointsMode::Read;
    commands_[radd] = new RAdd();
    cmdMode_[radd] |= BreakPointsMode::Read;
    commands_[rout] = new ROut();
    cmdMode_[rout] |= BreakPointsMode::Read;
    commands_[rin] = new RIn();
    cmdMode_[rin] |= BreakPointsMode::Write;
    commands_[isub] = new ISub();
    cmdMode_[isub] |= BreakPointsMode::Read;
    commands_[iadd] = new IAdd();
    cmdMode_[iadd] |= BreakPointsMode::Read;
    commands_[stores] = new Store();
    cmdMode_[stores] |= BreakPointsMode::Write;
    commands_[load] = new Load();
    cmdMode_[load] |= BreakPointsMode::Read;
    commands_[iin] = new IIn();
    cmdMode_[iin] |= BreakPointsMode::Write;
    commands_[iout] = new IOut();
    cmdMode_[iout] |= BreakPointsMode::Read;
    commands_[icmp] = new ICmp();
    cmdMode_[icmp] |= BreakPointsMode::Read;
    commands_[imul] = new IMul();
    cmdMode_[imul] |= BreakPointsMode::Read;
    commands_[idiv] = new IDiv();
    cmdMode_[idiv] |= BreakPointsMode::Read;
    commands_[imod] = new IMod();
    cmdMode_[imod] |= BreakPointsMode::Read;
    commands_[rsub] = new RSub();
    cmdMode_[rsub] |= BreakPointsMode::Read;
    commands_[rmul] = new RMul();
    cmdMode_[rmul] |= BreakPointsMode::Read;
    commands_[stIR] = new StIR();
    cmdMode_[stIR] |= BreakPointsMode::Read;
    commands_[stRI] = new StRI();
    cmdMode_[stRI] |= BreakPointsMode::Read;
    commands_[shiftL] = new ShiftL();
    cmdMode_[shiftL] |= BreakPointsMode::Read;
    commands_[shiftR] = new ShiftR();
    cmdMode_[shiftR] |= BreakPointsMode::Read;
    commands_[iand] = new And();
    cmdMode_[iand] |= BreakPointsMode::Read;
    commands_[ior] = new Or();
    cmdMode_[ior] |= BreakPointsMode::Read;
    commands_[ixor] = new Xor();
    cmdMode_[ixor] |= BreakPointsMode::Read;
    commands_[inot] = new Not();
    cmdMode_[inot] |= BreakPointsMode::Read;
    commands_[ldIR] = new LdIR();
    cmdMode_[ldIR] |= BreakPointsMode::Read;
    commands_[ldRI] = new LdRI();
    cmdMode_[ldRI] |= BreakPointsMode::Read;
    commands_[call] = new Call();
    cmdMode_[call] |= BreakPointsMode::Read;
    commands_[ret] = new Ret();
    commands_[inki] = new Inki();
    commands_[deci] = new Deci();
    commands_[laddr] = new Laddr();
    cmdMode_[laddr] |= BreakPointsMode::Read;
    commands_[ldStr] = new LoadString();
    cmdMode_[ldStr] |= BreakPointsMode::Write;
    commands_[prStr] = new PrintString();
    cmdMode_[prStr] |= BreakPointsMode::Read;
}


command Processor::GetCommand(const Memory &mem, address i) noexcept {
    MemoryUnion dat{};
    for (int j = i, k = 0; j < (i + sizeof(MemoryUnion)); ++j, ++k)
        dat.bytes[k] = mem[j];
    return dat.cmd;
}


void Processor::SetIp(address ip) noexcept {
    regs_.PSW.IP = ip;
}

void Processor::SetBp(address addr, byte isOn, BreakPointsMode mode, address numBP) noexcept {
    regs_.BreakPoints[numBP].addr = addr;
    regs_.BreakPoints[numBP].isOn = isOn;
    regs_.BreakPoints[numBP].mode = mode;
}

void Processor::RemoveBp(address numBP) noexcept {
    regs_.BreakPoints[numBP].isOn = 0;
}



