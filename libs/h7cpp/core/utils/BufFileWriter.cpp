#include "BufFileWriter.h"


namespace h7 {

void BufFileWriter::writeLine(CString str)
{
    m_buf.sputn(str.c_str(), str.length());
#ifdef WIN32
    m_buf.sputn("\r\n", 2);
#else
    m_buf.sputc('\n');
#endif
}
}
