#ifndef TESTROUTINE_H
#define TESTROUTINE_H

#include <iostream>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>


#include <sstream>
#include <vector>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define __okStr "Ok"
#define __defectedStr "Defected"
#define __repairedStr "Repaired"

#define __okID 0
#define __defectedID 1
#define __repairedID 2


#include "ICA2315.h"
    int SaveResult(std::string _fileNameTest);
    int SaveEUI(std::string _fileNameEUI,bool testResualt);
    int checkForOldTestingData(std::string _fileName);
#endif
