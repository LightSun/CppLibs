
#include <stdlib.h>
#include <stdio.h>

extern int _tmain(int argc, char* argv[]);


int main(int argc, char* argv[]){
    setbuf(stdout, NULL);
    return _tmain(argc, argv);
}
