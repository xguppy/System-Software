#ifndef SYSTEMSOFTWARE1_LOADERS_H
#define SYSTEMSOFTWARE1_LOADERS_H

#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include "Type.h"
#include "Processor.h"
#include "OpCodes.h"
//Загрузить в память(mem), указанные данные(dat), длинной(n), по заданному адресу(i) и комманды и данные(real and integer)
void LoadData(Memory &mem, address a, MemoryUnion dat, int n) noexcept
{
    for (int i = a, k = 0; i < a + n; ++i, ++k)
        mem[i] = dat.bytes[k];
}
//Трим начиная с начала
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

//Трим с конца
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

//Трим с начала и с конца
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

//Конверт байт-кода в бинарный код
void ConvertBCinBin(const std::string &filename) noexcept {
    std::ifstream ifile(filename);
    std::ofstream ofile(filename + ".bin", std::ios::binary);
    std::string s;                            // входная строка
    if (ifile.is_open()) {
        char typeRecord = 0;            // тип записи
        while (typeRecord != 'e' && getline(ifile, s))        // е - последняя запись
        {
            typeRecord = s[0];            // тип записи присвоен
            address Address, Code_tmp;
            byte b_tmp, Code;
            data temp{};  // Данные
            std::string strDat{};
            ofile.write(&typeRecord, sizeof(typeRecord));
            if (!s.empty()) {
                std::istringstream lineP(s.substr(1));    // для перевода
                switch (typeRecord)            // в зависимости от типа
                {
                    case Prefix::cmd:                //команда
                        // читаем и заносим в первую команду
                        lineP >> Code_tmp >> b_tmp >> Address;    // читаем КОП и адрес
                        Code = static_cast<byte>(Code_tmp);
                        ofile.write((char *) &Code, sizeof(Code));
                        ofile.write((char *) &b_tmp, sizeof(b_tmp));
                        ofile.write((char *) &Address, sizeof(Address));
                        break;
                    case Prefix::integer:                //целое число
                        lineP >> temp.integer;
                        ofile.write((char *) &temp, sizeof(temp));
                        break;
                    case Prefix::real:                //вещественное число
                        lineP >> temp.real;
                        ofile.write((char *) &temp, sizeof(temp));
                        break;
                    case Prefix::vmstring:        //Строка
                        strDat = lineP.str();        //Полная  строка
                        strDat = strDat.substr(0, strDat.find('$'));
                        trim(strDat);
                        temp.integer = strDat.size();
                        ofile.write((char *) &temp, sizeof(temp));
                        for(const auto &elem: strDat)
                            ofile.write((char *) &elem, sizeof(elem));
                        break;
                    case Prefix::start:
                    case Prefix::loadAddr:
                        address adr;
                        lineP >> adr;
                        ofile.write((char *) &adr, sizeof(adr));
                        break;
                    case Prefix::comment:
                    default:
                        break;
                }
            }
        }
    }
    ifile.close();
    ofile.close();
}
//Загрузка бинарного кода в память VM
void Loader(const std::string &filename, Processor &proc) noexcept {
    std::ifstream ifile(filename, std::ios::binary);
    //счётчик размещение в памяти(i), стартвый IP(startIp), смещение при чтении записи данных
    address i = 0, startIp = 0, offset = 0, readAddr = 0;
    char typeRecord = 0;
    if (ifile.is_open() && !ifile.bad()) {
        while (ifile.read(&typeRecord, sizeof(typeRecord)) && typeRecord != 'e') {
            address Address;
            byte b_tmp, Code;            // команда: КОП, адрес	            //Для b
            MemoryUnion dataAndCmd{};
            switch (typeRecord)            // в зависимости от типа
            {
                case Prefix::cmd:                // команда
                    // читаем и заносим в слово команду(В битовые поля нельзя записывать в бинарном виде)
                    ifile.read((char *) &Code, sizeof(Code));
                    ifile.read((char *) &b_tmp, sizeof(b_tmp));
                    ifile.read((char *) &Address, sizeof(Address));
                    dataAndCmd.cmd.addr = Address;
                    dataAndCmd.cmd.b = b_tmp;
                    dataAndCmd.cmd.opcode = Code;
                    offset = sizeof(command);
                    LoadData(proc.memory_, i, dataAndCmd, sizeof(command));
                    break;
                case Prefix::integer:                // целое число
                case Prefix::real:                // вещественное число
                    ifile.read((char *) &dataAndCmd, sizeof(dataAndCmd));
                    LoadData(proc.memory_, i, dataAndCmd, sizeof(MemoryUnion));
                    offset = sizeof(MemoryUnion);
                    break;
                case Prefix::vmstring:
                    ifile.read((char *) &dataAndCmd, sizeof(dataAndCmd));
                    LoadData(proc.memory_, i, dataAndCmd, sizeof(MemoryUnion));
                    i += sizeof(MemoryUnion); //Сдвинем на размер целого числа(длинна строки)
                    for (int j = i; j < dataAndCmd.dat.integer + i; ++j)
                        ifile.read((char *) &proc.memory_[j], sizeof(char));    //Запишем на  прямую в память байт
                    offset = dataAndCmd.dat.integer;
                    break;
                case Prefix::start:   //Стартовый адрес IP
                    ifile.read((char *) &readAddr, sizeof(readAddr));
                    startIp = readAddr;
                    offset = 0; // Смещаем только на символ
                    break;
                case Prefix::loadAddr:   //Адрес загрузки
                    ifile.read((char *) &i, sizeof(i));
                    offset = 0; // При новом адресе размещения смещаться никуда не надо
                    break;
                case Prefix::comment:
                default:
                    offset = 0; //Комментарий не трогает ничего
                    break;
            }
            i+= offset; //Перемещаем счётчик размещения
        }
    }
    proc.SetIp(startIp);
    ifile.close();
}

#endif //SYSTEMSOFTWARE1_LOADERS_H
