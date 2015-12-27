#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include <string>
using namespace std;
class network_utils
{
public:
    network_utils();
    ~network_utils();
    string* getHostName();
    string* getHostByName(string *name);
};

#endif // NETWORK_UTILS_H
