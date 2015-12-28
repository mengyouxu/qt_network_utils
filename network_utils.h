#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include <string>
#include <list>

using namespace std;
class network_utils
{
public:
    network_utils();
    ~network_utils();
    string* getHostName();
    list<string> getHostByName(string *name);
};

#endif // NETWORK_UTILS_H
