#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <cmath>
#include "Utils.h"

using namespace std;

int main()
{
    Decoder decoder("20150310.00.W.dwa_griby.grib");
    decoder.CreateOutputFile("output.txt");

    return 0;
}
