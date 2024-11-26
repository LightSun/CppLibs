#include "ZipHelper.h"
#include "zip.h"
#include "unzip.h"

namespace h7 {

class ZipHelper_Ctx0
{
public:
    ZipHelper_Ctx0();

    bool createZip(CString fn){
        CreateZip(fn.data(), 0);
    }
    bool openZip(CString fn);

    int getItemCount();
private:
    HZIP* zip {nullptr};
};
}
