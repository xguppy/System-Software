#ifndef SYSTEMSOFTWARE2_STRTODATE_H
#define SYSTEMSOFTWARE2_STRTODATE_H

#include <string>
#include "Errors.h"

using namespace std;


inline int StrToInteger(const std::string& value, Error& err)
{
    auto toInt = [&](string s, int base){
    try {
        return stoi(s, nullptr, base);
    }
    catch(...)
    {
        err = Error::illInteger;
    }
        return 0;
    };

    if(value[0] == '0')
    {
        if(value[1] == 'x' || value[1] == 'X' )    //16-ая система
            return toInt(value.substr(2), 16);
        else if(value[1] == 'b' || value[1] == 'B')     //2-ая система
            return toInt(value.substr(2), 2);
        else if(value[1] == 'o' || value[1] == 'O')     //8-ая система
            return toInt(value.substr(2), 8);
        else                                            // Всё остальное 10-ая(в т.ч. с лидирующими нулями)
            return toInt(value, 10);
    }
    else
        return toInt(value, 10);    //10-ая без лидирующих нулей

}

inline float StrToReal(const std::string& value, Error& err)
{
    float res{};
    try {
        res = stof(value);
    }
    catch(...)
    {
        err = Error::illRealValue;
    }
    return res;
}

inline std::string parseSimpleStr(const std::string &value, Error &err)
{

    string res{};
    char delimeter = '|';
    size_t lDelimeter = value.find(delimeter);
    size_t rDelimeter = value.rfind(delimeter);
    if(lDelimeter != string::npos && (lDelimeter != rDelimeter))
    {
        res = value.substr(lDelimeter + 1, rDelimeter - lDelimeter - 1);
    }
    else err = Error::illString;

    return res;
}

#endif //SYSTEMSOFTWARE2_STRTODATE_H
