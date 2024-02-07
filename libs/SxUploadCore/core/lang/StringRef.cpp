
#include "StringRef.h"
#include "utils/hash.h"

using namespace h7;

int StringRef::hashCode(){
    return fasthash32(m_data, m_validLen, 11);
}

