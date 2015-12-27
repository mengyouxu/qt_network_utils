#include "network_utils.h"

#include <winsock2.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

network_utils::network_utils()
{
    // Init Windows Socket
    WSADATA wsaData;

    WSAStartup(MAKEWORD(2,0),&wsaData);
}

network_utils::~network_utils()
{
    WSACleanup();
}

//get local host name
//return string
string* network_utils::getHostName()
{
    char *name = (char *)malloc(256);
    int ret_val = 0;

    ret_val = gethostname(name,256);
    if(ret_val != 0){
        cout<<"gethostname ret "<<ret_val<<", WSAGetLastError: "<<WSAGetLastError()<<endl;
        return NULL;
    }
    string *hostname = new string(name);
    flush(cout);

    return hostname;
}

string *network_utils::getHostByName(string *name)
{
    struct hostent *hostent_p;
    struct in_addr addr;


    hostent_p = gethostbyname(name->c_str());
    if(hostent_p == NULL){
        cout<<"hostent is NULL err: "<<WSAGetLastError()<<endl;
        return NULL;
    }
    cout<<"hostname is "<<hostent_p->h_name<<endl;

    if(hostent_p->h_aliases != NULL){
        char **alist;
        for(alist=hostent_p->h_aliases;*alist != NULL;alist++){
            cout<<"alieases: "<<*alist<<endl;
        }
    }
    if(hostent_p->h_addr_list!=NULL){
        char **alist;
        for(alist=hostent_p->h_addr_list;*alist!=NULL;alist++){
            memcpy(&addr.S_un.S_addr,*alist,hostent_p->h_length);
            cout<<"addr: "<<inet_ntoa(addr)<<endl;
        }
    }
    string *hostAddress = new string(inet_ntoa(addr));
    flush(cout);

    return hostAddress;
}
