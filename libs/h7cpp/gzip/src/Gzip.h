#pragma once

#include <vector>

namespace h7 {

struct IZlibInput{

    virtual bool hasNext() = 0;

    //isFinish: true means no more data.
    //return valid len of buffer.
    virtual size_t next(std::vector<char>& _vec,bool& isFinish) = 0;
};

struct IZlibOutput{

    virtual bool write(std::vector<char>& buf, size_t len) = 0;
};

}
