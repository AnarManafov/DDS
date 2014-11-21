// DDS
#include "KeyValue.h"
// STD
#include <iostream>
#include <exception>

using namespace std;
using namespace dds;

int main()
{
    try
    {
        CKeyValue ddsKeyValue;
        ddsKeyValue.putValue("prop_test_key", "prop_test_value");
    }
    catch (exception& _e)
    {
        cerr << "Error: " << _e.what();
    }
    return 0;
}