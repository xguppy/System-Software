#ifndef SYSTEMSOFTWARE1_DEBUGGER_H
#define SYSTEMSOFTWARE1_DEBUGGER_H


#include <map>
#include "Processor.h"

class Debugger {
    Processor &proc_ = Processor::Instance(0);
    std::string _log, _execFile;
    std::map<std::string, std::function<void(const std::vector<std::string>&, Debugger&)>> _cmdDebugTable;
    class InvalidNumberException: public std::exception
    {
    public:
        const char *what() const noexcept override;
    };
    class InvalidCmdException: public std::exception
    {
    public:
        const char *what() const noexcept override;
    };
    class InvalidModeException: public std::exception
    {
    public:
        const char *what() const noexcept override;
    };
    //====Вспомогательные методы
    void sayHello() const noexcept;
    void init();
    std::vector<std::string> split(const std::string &s, char delimiter) noexcept;
    data getData(address addr);
    void setData(address addr, data dat);
    std::string getString(address addr);
    void setString(address addr, std::string str);
    command getCommand(address addr);
    static BreakPointsMode getMode(char ch);
    //====!Вспомогательные методы

    //====Команды отладчика
    static void setBP(const std::vector<std::string> &args, Debugger &dbg);
    static void remBP(const std::vector<std::string> &args, Debugger &dbg);

    static void showAddrReg(const std::vector<std::string> &args, Debugger &dbg);
    static void setAddrReg(const std::vector<std::string> &args, Debugger &dbg);
    static void showSummator(const std::vector<std::string> &args, Debugger &dbg);
    static void setSummator(const std::vector<std::string> &args, Debugger &dbg);
    static void showAddr(const std::vector<std::string> &args, Debugger &dbg);
    static void setAddr(const std::vector<std::string> &args, Debugger &dbg);

    static void NextToEnd(const std::vector<std::string> &args, Debugger &dbg);
    static void NextToBP(const std::vector<std::string> &args, Debugger &dbg);
    static void StepToStep(const std::vector<std::string> &args, Debugger &dbg);
    //====!Команды отладчика
public:
    explicit Debugger(const std::string &execFile) noexcept;
    void Run();
};


#endif
