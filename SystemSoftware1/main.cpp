#include <iostream>
#include "Processor.h"
#include "Loaders.h"
using namespace std;

int main(const int argc, char *argv[]) {
    if(argc == 2)   // Если указан только файл
    {
        Processor &proc = Processor::Instance(0);   //Отпустить флаг трассировки
        Loader(argv[1], proc); //Загрузка бинарного файла
        proc.Run(); // Запуск
    }
    if (argc == 3) {    //Если указан файл и флаг трассировки
        Processor &proc = Processor::Instance(static_cast<uint8_t>(stoi(argv[2]))); // Получили флаг трассировки
        Loader(argv[1], proc); //Загрузка бинарного файла
        proc.Run(); // Запуск
    } else if (argc == 4) { //Если указан файл, флаг трассировки и режим чтения файла в байт кодах
        Processor &proc = Processor::Instance(static_cast<uint8_t>(stoi(argv[2]))); // Получили флаг трассировки
        if(stoi(argv[3]) == 1)
        {
            ConvertBCinBin(argv[1]);
            std::string strBin = argv[1];
            Loader(strBin + ".bin", proc); // Загрузили исходники
        }
        else
        {
            Loader(argv[1], proc); //Загрузка бинарного файла
        }
        proc.Run(); // Запуск
    }
    return 0;
}