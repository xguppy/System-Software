#include <sstream>
#include "Debugger.h"
#include "Type.h"
#include "Loaders.h"
using namespace std;

void Debugger::sayHello() const noexcept {
    cout << "Hello it's system 2DB \n";
    cout << "Prog start addr = " << proc_.regs_.PSW.IP << endl;
}

Debugger::Debugger(const std::string &execFile) noexcept: _execFile(execFile) {
    Loader(execFile, proc_);
    proc_.regs_.PSW.BF = 1;
    proc_.regs_.PSW.TF = 1;
    init();
    sayHello();
}

void Debugger::init() {
    _cmdDebugTable["setBP"] = setBP;
    _cmdDebugTable["remBP"] = remBP;
    _cmdDebugTable["showAR"] = showAddrReg;
    _cmdDebugTable["setAR"] = setAddrReg;
    _cmdDebugTable["showSMT"] = showSummator;
    _cmdDebugTable["setSMT"] = setSummator;
    _cmdDebugTable["showAddr"] = showAddr;
    _cmdDebugTable["setAddr"] = setAddr;
    _cmdDebugTable["NTE"] = NextToEnd;
    _cmdDebugTable["NTBP"] = NextToBP;
    _cmdDebugTable["STS"] = StepToStep;
}

std::vector<std::string> Debugger::split(const std::string &s, char delimiter) noexcept {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
        tokens.push_back(token);
    return tokens;
}

data Debugger::getData(address addr) {
    MemoryUnion result{};
    for (int i = addr, k = 0; i < addr + sizeof(result); ++i, ++k)
        result.bytes[k] = proc_.memory_[i];
    return result.dat;
}

void Debugger::setData(address addr, data dat) {
    MemoryUnion memUnion{dat};
    for (int i = addr, k = 0; i < addr + sizeof(memUnion); ++i, ++k)
        proc_.memory_[i] = memUnion.bytes[k];
}

void Debugger::Run() {
    string currentLine;
    string currentCmd;

    do {
        getline(cin, currentLine);
        vector<string> args = split(currentLine, ' ');
        if (!args.empty())
            currentCmd = args.front();
        try
        {
            if (_cmdDebugTable[currentCmd])
                _cmdDebugTable[currentCmd](args, *this);
        }
        catch (exception & e)
        {
            cout << e.what() << endl;
        }

    } while (currentCmd != "exit");

    ofstream protocol(_execFile + ".proto");
    protocol << _log;
}

void Debugger::setBP(const std::vector<std::string> &args, Debugger &dbg) {
    if(args.size() == 4)
    {
        address number, addr;
        if(args[1].size() != 1)
            throw InvalidNumberException();
        BreakPointsMode  md = getMode(args[1][0]);
        number = static_cast<address>(stoi(args[2]));
        addr = static_cast<address>(stoi(args[3]));
        dbg.proc_.SetBp(addr, 1, md, number);
        dbg._log += "break point set on addr " + to_string(number) + " type point is " + args[1] + " BP#" + to_string(number) + "\n";
    }
    else throw InvalidNumberException();
}

BreakPointsMode Debugger::getMode(char ch) {
    switch(ch)
    {
        case 'x':
            return BreakPointsMode::Execute;
        case 'w':
            return BreakPointsMode::Write;
        case 'r':
            return BreakPointsMode::Read;
        default:
            throw InvalidModeException();
    }
}

void Debugger::remBP(const std::vector<std::string> &args, Debugger &dbg) {
    if(args.size() >= 2)
    {
        for (int i = 1; i < args.size(); ++i) {
            dbg.proc_.RemoveBp(static_cast<address>(stoi(args[i])));
            dbg._log += "Remove BP# " + args[i] + "\n";
        }
    }
    else
    {
        dbg.proc_.RemoveBp(static_cast<address>(stoi(args[1])));
        dbg._log += "Remove BP# " + args[1] + "\n";
    }
}

void Debugger::showAddrReg(const std::vector<std::string> &args, Debugger &dbg) {
    dbg._log += "Addr reg is " + to_string(dbg.proc_.regs_.AddrReg) + "\n";
    std::cout << "Addr reg is " << dbg.proc_.regs_.AddrReg << endl;
}

void Debugger::setAddrReg(const std::vector<std::string> &args, Debugger &dbg) {
    if (args.size() != 2)
        throw InvalidCmdException();

    address addr = static_cast<address>(stoi(args[1]));
    dbg.proc_.regs_.AddrReg = addr;
    dbg._log += "Addr reg is " +  args[1] + "\n";
}

void Debugger::showSummator(const std::vector<std::string> &args, Debugger &dbg) {

    if (args.size() < 2 || (args[1] != "r" && args[1] != "i")) throw InvalidCmdException();
    dbg._log += "Summator ";
    if (args[1] == "r")
    {
        dbg._log += " exist real " + to_string(dbg.proc_.regs_.SummatorReg.real);
        cout << dbg.proc_.regs_.SummatorReg.real << endl;
    }
    else
    {
        dbg._log += " exist int " + to_string(dbg.proc_.regs_.SummatorReg.integer);
        cout << dbg.proc_.regs_.SummatorReg.integer << endl;
    }
    dbg._log += "\n";
}

string Debugger::getString(address addr) {
    data sz = getData(addr);
    string res;
    size_t PrintStrSize = addr + sizeof(sz);
    for (int i = 0; i < sz.integer; ++i)
        res += proc_.memory_[i + PrintStrSize];
    return res;
}

void Debugger::setString(address addr, std::string str) {
    data sz_str{};
    sz_str.integer = static_cast<int>(str.size());
    address  loadStrStart = addr;
    setData(addr, sz_str);
    loadStrStart += sizeof(data);
    for (int i = 0; i < sz_str.integer; ++i)
        proc_.memory_[i + loadStrStart] = static_cast<unsigned char>(str[i]);
}

void Debugger::setSummator(const std::vector<std::string> &args, Debugger &dbg) {
    if (args.size() < 3 || (args[1] != "r" && args[1] != "i")) throw InvalidCmdException();
    dbg._log += "Summator ";
    if (args[1] == "r")
    {
        dbg.proc_.regs_.SummatorReg.real = stof(args[2]);
        dbg._log += " exist real " + to_string(dbg.proc_.regs_.SummatorReg.real);
    }
    else
    {
        dbg.proc_.regs_.SummatorReg.integer = stoi(args[2]);
        dbg._log += " exist int " + to_string(dbg.proc_.regs_.SummatorReg.integer);
    }
    dbg._log += "\n";
}

void Debugger::showAddr(const std::vector<std::string> &args, Debugger &dbg) {
    if (args.size() < 4 || (args[1] != "r" && args[1] != "i" && args[1] != "s" && args[1] != "c")) throw InvalidCmdException();

    address addr = static_cast<address>(stoi(args[2]));
    address count = static_cast<address>(stoul(args[3]));

    dbg._log += "Data in memory " + args[1] + " " + args[2] + " " + args[3] + "\n";

    if (args[1] == "i")
    {
        for (int i = 0; i < count; ++i) {
            data dat = dbg.getData(addr + i * sizeof(dat));
            dbg._log += to_string(dat.integer) + "\n";
            cout << to_string(dat.integer) << endl;
        }
    }
    else if (args[1] == "r")
    {
        for (int i = 0; i < count; ++i) {
            data dat = dbg.getData(addr + i * sizeof(dat));
            dbg._log += to_string(dat.real) + "\n";
            cout << to_string(dat.real) << endl;
        }
    }
    else if (args[1] == "s")
    {
        int size = 0;
        for (int i = 0; i < count; ++i) {
            data dat = dbg.getData(addr + i * sizeof(dat) + size);
            size = dat.integer;
            dbg._log += dbg.getString(addr) + "\n";
            cout << dbg.getString(addr) << endl;
        }
    }
    else
    {
        for (int i = 0; i < count; ++i) {
            command cmd = dbg.getCommand(addr + i * sizeof(cmd));
            dbg._log += "Op " + to_string(cmd.opcode) + " b bit " + to_string(cmd.b) + " addr " + to_string(cmd.addr) + "\n";
            cout << "Op " << to_string(cmd.opcode) << " b bit " << to_string(cmd.b) << " addr " + to_string(cmd.addr) << endl;
        }
    }
}

command Debugger::getCommand(address addr) {
    MemoryUnion dat{};
    for (int j = addr, k = 0; j < (addr + sizeof(MemoryUnion)); ++j, ++k)
        dat.bytes[k] = proc_.memory_[j];
    return dat.cmd;
}

void Debugger::setAddr(const std::vector<std::string> &args, Debugger &dbg) {
    if (args.size() < 4 || (args[1] != "r" && args[1] != "i" && args[1] != "s")) throw InvalidCmdException();

    address addr = static_cast<address>(stoi(args[2]));

    if (args[1] == "i")
    {
        data dat{};
        dat.integer = stoi(args[3]);
        dbg.setData(addr, dat);
    }
    else if (args[1] == "r")
    {
        data dat{};
        dat.real = stof(args[3]);
        dbg.setData(addr, dat);
    }
    else
    {
        dbg.setString(addr, args[3]);
    }
}

void Debugger::NextToEnd(const std::vector<std::string> &args, Debugger &dbg) {
    dbg.proc_.regs_.PSW.BF = 0;
    dbg.proc_.regs_.PSW.TF = 0;
    dbg.proc_.Run();
    cout << endl << ">_";
    dbg._log += "exec ==== \n";
    dbg._log += dbg.proc_.logT_;
}

void Debugger::NextToBP(const std::vector<std::string> &args, Debugger &dbg) {
    dbg.proc_.regs_.PSW.BF = 0;
    dbg.proc_.Run();
    cout << endl << ">_";
    dbg._log += "exec to bp ==== \n";
    dbg._log += dbg.proc_.logT_;
}

void Debugger::StepToStep(const std::vector<std::string> &args, Debugger &dbg) {
    dbg.proc_.regs_.PSW.BF = 1;
    dbg.proc_.regs_.PSW.TF = 1;
    dbg.proc_.Run();
    cout << endl << ">_";
    dbg._log += "exec with trace ==== \n";
    dbg._log += dbg.proc_.logT_;
}


const char *Debugger::InvalidNumberException::what() const noexcept {
    return "Invalid number of arguments";
}

const char *Debugger::InvalidCmdException::what() const noexcept {
    return "Invalid cmd name";
}

const char *Debugger::InvalidModeException::what() const noexcept {
    return "Invalid mode BreakPoint";
}
