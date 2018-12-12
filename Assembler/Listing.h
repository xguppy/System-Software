#include <utility>

#include <utility>

#ifndef SYSTEMSOFTWARE2_LISTING_H
#define SYSTEMSOFTWARE2_LISTING_H

#include <string>
#include <vector>
#include "Operator.h"
#include "Errors.h"
#include "MParser.h"

class Listing {
private:
    std::string _listFile;
    Context _ctx;
    std::vector<Operator> _program;
    static std::string getErrorStr(Error error) noexcept;
public:
    Listing(std::string listFile, Context ctx, std::vector<Operator> program): _listFile(std::move(listFile)), _ctx(
            std::move(ctx)), _program(std::move(program))
    {}
    void WriteListing() const noexcept;
};


#endif //SYSTEMSOFTWARE2_LISTING_H
