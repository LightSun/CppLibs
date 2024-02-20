#ifndef FIREWALL_H
#define FIREWALL_H

#include <windows.h>
#include <stdio.h>

namespace h7 {

void WriteFireWall(LPCTSTR ruleName, LPCTSTR appPath,bool NoopIfExist);
}


#endif // FIREWALL_H
