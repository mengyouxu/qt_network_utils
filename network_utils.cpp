#include "network_utils.h"

#include <winsock2.h>
#include <ws2tcpip.h>

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
    cout<<"Enter "<<__func__<<endl;
    ret_val = gethostname(name,256);
    if(ret_val != 0){
        cout<<"gethostname ret "<<ret_val<<", WSAGetLastError: "<<WSAGetLastError()<<endl;
        return NULL;
    }
    string *hostname = new string(name);
    flush(cout);

    return hostname;
}


//A host could have many address, so we need to return a address list
list<string> network_utils::getHostByName(string *name)
{
    struct hostent *hostent_p;
    struct in_addr addr;
    list<string> addr_list;
    cout<<"Enter "<<__func__<<endl;
    hostent_p = gethostbyname(name->c_str());
    if(hostent_p == NULL){
        cout<<"hostent is NULL err: "<<WSAGetLastError()<<endl;
        return addr_list;
    }
    cout<<"hostname is "<<hostent_p->h_name<<endl;

    if(hostent_p->h_aliases != NULL){
        char **alist;
        for(alist=hostent_p->h_aliases;*alist != NULL;alist++){
            //cout<<"alieases: "<<*alist<<endl;
        }
    }
    if(hostent_p->h_addr_list!=NULL){
        char **alist;
        for(alist=hostent_p->h_addr_list;*alist!=NULL;alist++){
            memcpy(&addr.S_un.S_addr,*alist,hostent_p->h_length);
            addr_list.push_back(string(inet_ntoa(addr)));
            cout<<"addr: "<<inet_ntoa(addr)<<endl;
        }
    }

    flush(cout);
    return addr_list;
}


float network_utils::pingHost(string *name,int times,int timeout,int ttl)
{
    float lost_percent = 0.0;
    list<string> addr_list = getHostByName(name);

    cout<<"target: "<<name->c_str()<<" -> "<<addr_list.begin()->c_str()<<endl;

    cout<<"Enter "<<__func__<<endl;
    cout<<"size of char: "<<sizeof(char)<<endl;
    cout<<"size of short: "<<sizeof(short)<<endl;
    cout<<"size of int: "<<sizeof(int)<<endl;
    cout<<"size of long: "<<sizeof(long)<<endl;
    cout<<"size of long lone: "<<sizeof(long long)<<endl;
    cout<<"size of IP_HEADER "<<sizeof(IP_HEADER)<<endl;
    cout<<"size of ICMP_HEADER "<<sizeof(ICMP_HEADER)<<endl;
    cout<<"size of align test "<<sizeof(struct align_test)<<endl;

    // 1. resulve host name into ip addr
    // 2. open socket
    // 3. fill ICMP echo packet
    // 4. send ICMP echo packet to target host
    // 5. recvfrom target host, parse ICMP echo replay packet

    int nTime = 2000;
    int nValue =20;
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(sock<0){
        cout<<" socket() failed: "<<WSAGetLastError()<<endl;
        return -1;
    }
    int ret = setsockopt(sock, SOL_SOCKET,true ? SO_RCVTIMEO : SO_SNDTIMEO, (char*)&nTime, sizeof(nTime));
    ret = setsockopt(sock, IPPROTO_IP, IP_TTL/*<ws2tcpip.h>*/, (char*)&nValue, sizeof(nValue));

    sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_port = htons(0);
    dest.sin_addr.s_addr = inet_addr(addr_list.begin()->c_str());


    char buff[sizeof(ICMP_HEADER) + 32];
    ICMP_HEADER* pIcmp = (ICMP_HEADER*)buff;

    pIcmp->type = 8;
    pIcmp->code = 0;
    pIcmp->id = (unsigned short)GetCurrentProcessId();
    pIcmp->checksum = 0;
    pIcmp->seq_num = 0;
    memset(&buff[sizeof(ICMP_HEADER)], 'E', 32);

    unsigned short nSeq = 0;
    char recvBuf[1024];
    sockaddr_in from;
    int nLen = sizeof(from);
    for(int i=0;i<times;i++)
    {
        int nRet;
        pIcmp->checksum = 0;
        pIcmp->timestamp = ::GetTickCount();
        pIcmp->seq_num = nSeq++;
        pIcmp->checksum = checksum((unsigned short*)buff, sizeof(ICMP_HEADER) + 32);
        nRet = sendto(sock, buff, sizeof(ICMP_HEADER) + 32, 0, (sockaddr *)&dest, sizeof(dest));
        if(nRet == SOCKET_ERROR)
        {
            cout<<" sendto() failed: "<<WSAGetLastError()<<endl;
            return -1;
        }
        nRet = recvfrom(sock, recvBuf, 1024, 0, (sockaddr*)&from, &nLen);
        if(nRet == SOCKET_ERROR)
        {
            if(WSAGetLastError() == WSAETIMEDOUT)
            {
                cout<<" timed out"<<endl;
                continue;
            }
            cout<<" recvfrom() failed: "<<WSAGetLastError()<<endl;
            return -1;
        }

        int nTick = GetTickCount(); //开机到现在过去的ms
        if(nRet < sizeof(IP_HEADER) + sizeof(ICMP_HEADER))
        {
            cout<<" Too few bytes from "<<inet_ntoa(from.sin_addr)<<endl;
        }
        ICMP_HEADER* pRecvIcmp = (ICMP_HEADER*)(recvBuf + sizeof(IP_HEADER));
        if(pRecvIcmp->type != 0)
        {
            cout<<" nonecho type  "<< (unsigned int)(pRecvIcmp->type)<<endl;
            cout<<" nonecho code  "<< (unsigned int)(pRecvIcmp->code)<<endl;
            return -1;
        }

        if(pRecvIcmp->id != GetCurrentProcessId())
        {
            cout<<" someone else's packet! "<<endl;
            return -1;
        }

        cout<<nRet<<" bytes from "<<inet_ntoa(from.sin_addr)<<", ";
        cout<<"icmp_seq = "<<pRecvIcmp->seq_num<<", ";
        cout<<"time: "<<nTick - pRecvIcmp->timestamp<<" ms"<<endl;
        flush(cout);
        Sleep(1000);
    }

    return lost_percent;
}
unsigned short network_utils::checksum(unsigned short *buff, int size)
{
    unsigned long cksum = 0;
    while(size>1)
    {
        cksum += *buff++;
        size -= sizeof(unsigned short);
    }

    if(size)
    {
        cksum += *(unsigned char*)buff;
    }

    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >> 16);
    return (unsigned short)(~cksum);
}
