#include <iostream>
#include "Assembler.h"
using namespace std;

int main(int argc, char *argv[]) {
    if (argc == 3) {
        string file = argv[1];
        string bin = file + ".bin";
        Assembler assembler(file);
        assembler.createExecute(bin);
        if(stoi(argv[2]) == 1) assembler.createListing(file + ".lst");
        if(!assembler.isExistError())
            system(("SystemSoftware1 " + bin).c_str());
    } else
    {
        cout << "error";
    }
    return 0;
}