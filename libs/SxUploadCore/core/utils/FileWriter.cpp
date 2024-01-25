#include "FileWriter.h"


namespace h7 {

void FileWriter::writeLine(CString str)
{
    m_stream.write(str.c_str(), str.length());
    m_stream << std::endl;
}
}
