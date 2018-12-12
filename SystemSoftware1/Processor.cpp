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
    regs_.AddrReg = 0;
    regs_.SummatorReg.integer = 0;
    memory_.clear();
}

void Processor::Run() noexcept {
    while(regs_.PSW.EF != 1 && regs_.PSW.ER != 1) // Если конец или ошибка, то выходим
    {
        regs_.PSW.JP = 0;
        if(regs_.PSW.TF != 0) Trace();  // До выпонения комманды
        regs_.ExecuteCommand = GetCommand(memory_, regs_.PSW.IP);
        if(commands_[regs_.ExecuteCommand.opcode])
            commands_[regs_.ExecuteCommand.opcode]->operator()(memory_, regs_);
        else
            regs_.PSW.ER = 1;
        if(regs_.PSW.TF != 0) Trace();  // После выполнения комманды
        if(regs_.PSW.JP == 0) regs_.PSW.IP += sizeof(command);
    }
}

void Processor::Trace() const noexcept {
    std::cout << "Summator: " << std::setw(10) << std::setfill('0') << regs_.SummatorReg.integer << "; "
              << std::setw(12) << regs_.SummatorReg.real << std::endl;
    std::cout << "ExecutingCommand: " << std::setw(10) << std::setfill('0') << regs_.ExecuteCommand.opcode << "; "
              << std::setw(12) << regs_.ExecuteCommand.b << std::setw(12) << regs_.ExecuteCommand.addr << std::endl;
    std::cout << "CF: "<< regs_.PSW.CF << std::endl << "; "
             << "ER: " << regs_.PSW.ER << "; "
            << "EF: "  << regs_.PSW.EF << "; "
            << "JP: "  << regs_.PSW.JP << "; ";
    std::cout << "AdresReg: " << std::setw(4) << std::setfill('0') << regs_.AddrReg << " - next address";
    std::cout << "IP: " << std::setw(4) << std::setfill('0') << regs_.PSW.IP << " - next address";
}

void Processor::InitCmd() noexcept {
    commands_.assign(128, nullptr);
    commands_[stop] = new Stop();
    commands_[go] = new GO();
    commands_[jl] = new JL();
    commands_[jg] = new JG();
    commands_[jz] = new JZ();
    commands_[rcmp] = new RCmp();
    commands_[rdiv] = new RDiv();
    commands_[radd] = new RAdd();
    commands_[rout] = new ROut();
    commands_[rin] = new RIn();
    commands_[isub] = new ISub();
    commands_[iadd] = new IAdd();
    commands_[stores] = new Store();
    commands_[load] = new Load();
    commands_[iin] = new IIn();
    commands_[iout] = new IOut();
    commands_[icmp] = new ICmp();
    commands_[imul] = new IMul();
    commands_[idiv] = new IDiv();
    commands_[imod] = new IMod();
    commands_[rsub] = new RSub();
    commands_[rmul] = new RMul();
    commands_[stIR] = new StIR();
    commands_[stRI] = new StRI();
    commands_[shiftL] = new ShiftL();
    commands_[shiftR] = new ShiftR();
    commands_[iand] = new And();
    commands_[ior] = new Or();
    commands_[ixor] = new Xor();
    commands_[inot] = new Not();
    commands_[ldIR] = new LdIR();
    commands_[ldRI] = new LdRI();
    commands_[call] = new Call();
    commands_[ret] = new Ret();
    commands_[inki] = new Inki();
    commands_[deci] = new Deci();
    commands_[laddr] = new Laddr();
    commands_[ldStr] = new LoadString();
    commands_[prStr] = new PrintString();
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

