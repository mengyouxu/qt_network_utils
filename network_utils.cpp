/*
    Only works on Windows
*/

#include "network_utils.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <unistd.h>

#include <windows.h>

#include <stdlib.h>
#include <string.h>
#include <iostream>

network_utils::network_utils()
:print_debug_info(false)
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
    if(print_debug_info)cout<<"Enter "<<__func__<<endl;

    {
        if(print_debug_info)cout<<"Enter "<<__func__<<endl;
        if(print_debug_info)cout<<"size of char: "<<sizeof(char)<<endl;
        if(print_debug_info)cout<<"size of short: "<<sizeof(short)<<endl;
        if(print_debug_info)cout<<"size of int: "<<sizeof(int)<<endl;
        if(print_debug_info)cout<<"size of long: "<<sizeof(long)<<endl;
        if(print_debug_info)cout<<"size of long lone: "<<sizeof(long long)<<endl;
        if(print_debug_info)cout<<"size of IP_HEADER "<<sizeof(IP_HEADER)<<endl;
        if(print_debug_info)cout<<"size of ICMP_HEADER "<<sizeof(ICMP_HEADER)<<endl;
        if(print_debug_info)cout<<"size of align test "<<sizeof(struct align_test)<<endl;
    }
    ret_val = gethostname(name,256);
    if(ret_val != 0){
        std::cerr<<"gethostname ret "<<ret_val<<", WSAGetLastError: "<<WSAGetLastError()<<endl;
        return NULL;
    }
    string hostname;
    hostname.assign(name);
    flush(cout);

    return hostname;
}
int network_utils::dnsPacketFactory(unsigned char* data,int size,string *hostname){
    DNS_HEADER header;
    header.id=GetCurrentProcessId();

    header.flags = 0x00;
    header.question_count = 1;
    header.anwser_record_count = 0;
    header.authority_record_count = 0;
    header.additional_record_count = 0;

    unsigned char question_data[hostname->size()+2] = {'\0'};

    int endPos = hostname->find(".");
    int offset = endPos + 1;
    string substring = hostname->substr(0,endPos);
    cout<<"substring = "<<substring.c_str()<<endl;
    cout<<"endPos = "<<endPos<<endl;
    cout<<"offset = "<<offset<<endl;
    for(int i = 0;;){

        question_data[i]=(unsigned char)substring.size();
        i++;
        memcpy(&question_data[i],substring.data(),substring.size());
        i+=substring.size();
        if(offset>=hostname->size()){
                question_data[i] = 0;
                break;
        }

        substring = hostname->substr(offset,hostname->size());
        cout<<"substring = "<<substring.c_str()<<endl;
        endPos = substring.find(".");
        cout<<"endPos = "<<endPos<<endl;
        if(endPos<=0){
            endPos = substring.size() - 1;
        }else{
            substring = substring.substr(0,endPos);
        }
        cout<<"substring = "<<substring.c_str()<<endl;
        offset = offset + endPos +1;
        cout<<"offset = "<<offset<<endl;
    }

    if(size < sizeof(DNS_HEADER) + sizeof(question_data)){
        return 0;
    }
    memcpy(data,&header,sizeof(DNS_HEADER));
    memcpy(data+sizeof(DNS_HEADER),question_data,sizeof(question_data));

    return sizeof(DNS_HEADER) + sizeof(question_data);
}
//A host could have many address, so we need to return a address list
list<string> network_utils::getHostByName(string *name)
{
    struct hostent *hostent_p;
    struct in_addr addr;
    list<string> addr_list;
    if(print_debug_info)cout<<"Enter "<<__func__<<endl;
    hostent_p = gethostbyname(name->c_str());
    if(hostent_p == NULL){
        std::cerr<<"hostent is NULL err: "<<WSAGetLastError()<<endl;
        return addr_list;
    }
    if(print_debug_info)cout<<"hostname is "<<hostent_p->h_name<<endl;

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
            if(print_debug_info)cout<<"addr: "<<inet_ntoa(addr)<<endl;
        }
    }

    flush(cout);
    return addr_list;
}
bool network_utils::isIPv4Addr(string *addr){

    if(addr->size()>15){// example: 192.168.123.123
        return false;
    }else if(addr->size()<7){// example : 1.1.1.1
        return false;
    }

    int pos_begin = 0;
    int pos_end = 0;
    int offset = 0;

    string temp_string;
    unsigned int temp_int = 0;

    // part 1
    pos_end = addr->find(".");

    temp_string = addr->substr(pos_begin,pos_end);
    offset = pos_end;
    if(print_debug_info)cout<<"substr : "<<temp_string.c_str()<<endl;
    temp_int = temp_string.size();
    if(temp_int>3 || temp_int<1){
        return false;
    }
    for(temp_int = 0;temp_int < temp_string.size();temp_int++){
        if(isdigit(temp_string.at(temp_int))){
            if(print_debug_info)cout<<"this is digit : "<<temp_string.at(temp_int)<<endl;
        }else{
            if(print_debug_info)cout<<"this is nto digit or dot: "<<temp_string.at(temp_int)<<endl;
            return false;
        }
    }
    temp_int = stoi(temp_string);
    if(temp_int > 0xff || temp_int < 0){
        return false;
    }
    // part 2
    pos_begin = offset+1;
    pos_end=addr->size()-1;
    temp_string = addr->substr(pos_begin,pos_end);
    if(print_debug_info)cout<<"substr : "<<temp_string.c_str()<<endl;
    pos_begin=0;
    pos_end = temp_string.find(".");
    temp_string = temp_string.substr(pos_begin,pos_end);
    offset = offset + pos_end +1;
    if(print_debug_info)cout<<"substr : "<<temp_string.c_str()<<endl;
    temp_int = temp_string.size();
    if(temp_int>3 || temp_int<1){
        return false;
    }
    for(temp_int = 0;temp_int < temp_string.size();temp_int++){
        if(isdigit(temp_string.at(temp_int))){
            if(print_debug_info)cout<<"this is digit : "<<temp_string.at(temp_int)<<endl;
        }else{
            if(print_debug_info)cout<<"this is nto digit or dot: "<<temp_string.at(temp_int)<<endl;
            return false;
        }
    }
    temp_int = stoi(temp_string);
    if(temp_int > 0xff || temp_int < 0){
        return false;
    }

    // part 3
    pos_begin = offset+1;
    pos_end=addr->size()-1;
    temp_string = addr->substr(pos_begin,pos_end);
    if(print_debug_info)cout<<"substr : "<<temp_string.c_str()<<endl;
    pos_begin=0;
    pos_end = temp_string.find(".");
    temp_string = temp_string.substr(pos_begin,pos_end);
    offset = offset + pos_end +1;
    if(print_debug_info)cout<<"substr : "<<temp_string.c_str()<<endl;
    temp_int = temp_string.size();
    if(temp_int>3 || temp_int<1){
        return false;
    }
    for(temp_int = 0;temp_int < temp_string.size();temp_int++){
        if(isdigit(temp_string.at(temp_int))){
            if(print_debug_info)cout<<"this is digit : "<<temp_string.at(temp_int)<<endl;
        }else{
            if(print_debug_info)cout<<"this is nto digit or dot: "<<temp_string.at(temp_int)<<endl;
            return false;
        }
    }
    temp_int = stoi(temp_string);
    if(temp_int > 0xff || temp_int < 0){
        return false;
    }
    // part 4
    pos_begin = offset+1;
    pos_end=addr->size()-1;
    temp_string = addr->substr(pos_begin,pos_end);
    if(print_debug_info)cout<<"substr : "<<temp_string.c_str()<<endl;
    for(temp_int = 0;temp_int < temp_string.size();temp_int++){
        if(isdigit(temp_string.at(temp_int))){
            if(print_debug_info)cout<<"this is digit : "<<temp_string.at(temp_int)<<endl;
        }else{
            if(print_debug_info)cout<<"this is not digit or dot: "<<temp_string.at(temp_int)<<endl;
            return false;
        }
    }
    temp_int = stoi(temp_string);
    if(temp_int > 0xff || temp_int < 0){
        return false;
    }

    return true;
}
bool network_utils::isIPv6Addr(string *addr){

    return false;
}
float network_utils::pingHost(string *name,int times,int timeout,int ttl)
{
    float lost_count = 0.0;
    int ret_val = 0;
    list<string> addr_list;
    if(print_debug_info)cout<<"Enter "<<__func__<<" ";
    if(print_debug_info)cout<<"target: "<<name->c_str()<<", time: "<<times;
    if(print_debug_info)cout<<", timeout: "<<timeout<<", ttl: "<<ttl<<endl;

    // 1. resulve host name into ip addr
    // 2. open socket
    // 3. fill ICMP echo packet
    // 4. send ICMP echo packet to target host
    // 5. recvfrom target host, parse ICMP echo replay packet
    sockaddr_in addr_dest;
    addr_dest.sin_family = AF_INET;
    addr_dest.sin_port = htons(0);
    if(!isIPv4Addr(name)){
        addr_list = getHostByName(name);
        addr_dest.sin_addr.s_addr = inet_addr(addr_list.begin()->c_str());
        if(print_debug_info)cout<<"target: "<<name->c_str()<<" -> "<<addr_list.begin()->c_str()<<endl;
    }else{
        addr_dest.sin_addr.s_addr = inet_addr(name->c_str());
    }

    int nTime = timeout;
    int nValue =ttl;
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(sock<0){
        std::cerr<<" socket() failed: "<<WSAGetLastError()<<endl;
        return 0.0;
    }

    ret_val = setsockopt(sock, SOL_SOCKET,SO_RCVTIMEO, (char*)&nTime, sizeof(nTime));
    //ret_val = setsockopt(sock, SOL_SOCKET,SO_SNDTIMEO, (char*)&nTime, sizeof(nTime));
    ret_val = setsockopt(sock, IPPROTO_IP, IP_TTL/*<ws2tcpip.h>*/, (char*)&nValue, sizeof(nValue));

    char send_buff[sizeof(ICMP_HEADER) + 32] = {'\0'};
    ICMP_HEADER* pIcmp_send = (ICMP_HEADER*)send_buff;

    pIcmp_send->type = 8;
    pIcmp_send->code = 0;
    pIcmp_send->id = (unsigned short)GetCurrentProcessId();
    pIcmp_send->checksum = 0;
    pIcmp_send->seq_num = 0;
    memset(&send_buff[sizeof(ICMP_HEADER)], 'p', 32);
    if(print_debug_info)cout<<"GetCurrentProcessId() "<<GetCurrentProcessId()<<endl;

    unsigned short nSeq = 0;
    char recvBuf[1024]= {'\0'};
    sockaddr_in addr_recv;
    int nLen = sizeof(addr_recv);
    for(int i=0;i<times;i++)
    {
        int nRet;
        pIcmp_send->checksum = 0; // MUST assign checksum value to 0 before calculate checksum
        pIcmp_send->timestamp = GetTickCount();
        pIcmp_send->seq_num = nSeq++;
        pIcmp_send->checksum = checksum((unsigned short*)send_buff, sizeof(ICMP_HEADER) + 32);
        nRet = sendto(sock, send_buff, sizeof(ICMP_HEADER) + 32, 0, (sockaddr *)&addr_dest, sizeof(addr_dest));
        if(nRet == SOCKET_ERROR)
        {
            std::cerr<<" sendto() failed: "<<WSAGetLastError()<<endl;
            lost_count += 1.0;
        }
        nRet = recvfrom(sock, recvBuf, 1024, 0, (sockaddr*)&addr_recv, &nLen);
        if(nRet == SOCKET_ERROR)
        {
            lost_count += 1.0;
            if(WSAGetLastError() == WSAETIMEDOUT)
            {
                cout<<" timed out"<<endl;
            }
            cout<<" recvfrom() failed: "<<WSAGetLastError()<<endl;
            continue;
        }

        int nTick = GetTickCount(); //开机到现在过去的ms
        if(nRet < sizeof(IP_HEADER) + sizeof(ICMP_HEADER))
        {
            if(print_debug_info)cout<<" Too few bytes from "<<inet_ntoa(addr_recv.sin_addr)<<endl;
        }
        ICMP_HEADER* pRecvIcmp = (ICMP_HEADER*)(recvBuf + sizeof(IP_HEADER));
        if(pRecvIcmp->type != 0)
        {
            lost_count += 1.0;
            switch(pRecvIcmp->type){
            case 11:
                if(print_debug_info)cout<<"Time exceeded message from "<<inet_ntoa(addr_recv.sin_addr)<<endl;
                if(print_debug_info)cout<<", packet id: "<<pRecvIcmp->id<<endl;
                break;
            default:
                break;
            }
            if(print_debug_info)cout<<" nonecho type  "<< (unsigned int)(pRecvIcmp->type)<<endl;
            if(print_debug_info)cout<<" nonecho code  "<< (unsigned int)(pRecvIcmp->code)<<endl;
            continue;
        }

        if(pRecvIcmp->id != GetCurrentProcessId())
        {
            if(print_debug_info)cout<<" someone else's packet! id: "<<pRecvIcmp->id<<endl;
            lost_count += 1.0;
            continue;
        }

        if(print_debug_info)cout<<nRet<<" bytes from "<<inet_ntoa(addr_recv.sin_addr)<<", ";
        if(print_debug_info)cout<<"icmp_seq = "<<pRecvIcmp->seq_num<<", ";
        if(print_debug_info)cout<<"time: "<<nTick - pRecvIcmp->timestamp<<" ms"<<endl;
        flush(cout);
        //Sleep(1000);
    }
    if(print_debug_info)cout<<"lost_count : "<<lost_count<<endl;
    if(sock>0){
        close(sock);
    }
    return (times - lost_count)/times;
}
unsigned short network_utils::checksum(unsigned short *buff, int size)
{
    unsigned long cksum = 0;
    while(size>1)
    {
        cksum += *buff++;
        size -= sizeof(unsigned short);
    }

    if(size==1)
    {
        if(isHWBigEndian()){
            cksum += *(unsigned char*)buff<<8;
        }else{
            cksum += *(unsigned char*)buff;
        }
    }

    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >> 16);
    return (unsigned short)(~cksum);
}

list<string> network_utils::getRoute(string *name)
{
    if(print_debug_info)cout<<"enter "<<__func__<<endl;

    int ret_val = 0;
    int sock_recv_timeout = 2000;// 5sec
    int sock_send_timeout = 2000;
    int sock_ip_ttl = 0;

    list<string> route;
    list<string> addr_list;
    sockaddr_in addr_dest;

    int sock_main = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(sock_main < 0){
        std::cerr<<"Line "<<__LINE__<<" : "<<"open socket error WSAGetLastError : "<<WSAGetLastError()<<endl;
        return route;
    }

    //ret_val = setsockopt(sock_main,SOL_SOCKET,SO_SNDTIMEO,(char *)&sock_send_timeout,sizeof(sock_send_timeout));
    ret_val = setsockopt(sock_main,SOL_SOCKET,SO_RCVTIMEO,
                         (char *)&sock_recv_timeout,sizeof(sock_recv_timeout));

    if(!isIPv4Addr(name)){
        addr_list = getHostByName(name);
        addr_dest.sin_addr.s_addr=inet_addr(addr_list.begin()->c_str());
        if(print_debug_info)cout<<"target : "<<name->c_str()<<" -> "<<addr_list.begin()->c_str()<<endl;
    }else{
        addr_dest.sin_addr.s_addr=inet_addr(name->c_str());
    }

    addr_dest.sin_family = AF_INET;
    addr_dest.sin_port = htons(0);
    /*
    ret_val = connect(sock_main,(struct sockaddr *)&addr_dest,sizeof(addr_dest));
    if(ret_val != 0){
        std::cerr<<"connect ret : "<<WSAGetLastError()<<endl;
        return route;
    }
    */

    char send_buff[sizeof(ICMP_HEADER) + 32] = {'\0'};
    ICMP_HEADER* pIcmp_send = (ICMP_HEADER*)send_buff;

    unsigned short nSeq = 0;
    pIcmp_send->type = 8;
    pIcmp_send->code = 0;
    pIcmp_send->id = (unsigned short)GetCurrentProcessId();
    pIcmp_send->checksum = 0;
    pIcmp_send->seq_num = 0;
    memset(&send_buff[sizeof(ICMP_HEADER)], 'r', 32);
    cout<<"GetCurrentProcessId() "<<GetCurrentProcessId()<<endl;

    char recv_buff[1024] = {'\0'};
    sockaddr_in addr_recv;
    int addr_recv_len = sizeof(addr_recv);
    ICMP_HEADER* pIcmpRecv = (ICMP_HEADER*)(recv_buff + sizeof(IP_HEADER));

    if(print_debug_info)cout<<"setsockopt return "<<ret_val<<endl;
    do{
        pIcmp_send->checksum = 0;// MUST assign checksum to 0!!!
        pIcmp_send->timestamp = GetTickCount();
        pIcmp_send->seq_num = nSeq++;
        pIcmp_send->checksum = checksum((unsigned short*)send_buff, sizeof(ICMP_HEADER) + 32);
        sock_ip_ttl++;
        ret_val = setsockopt(sock_main,IPPROTO_IP,IP_TTL,(char *)&sock_ip_ttl,sizeof(sock_ip_ttl));
        ret_val = sendto(sock_main, send_buff, sizeof(ICMP_HEADER) + 32, 0, (sockaddr *)&addr_dest, sizeof(addr_dest));
        if(ret_val == SOCKET_ERROR){
            std::cerr<<"Line "<<__LINE__<<"--";
            std::cerr<<"sendto return "<<ret_val<<" WSAGetLastError() "<<WSAGetLastError()<<std::endl;
            close(sock_main);
            return route;
        }
        ret_val = recvfrom(sock_main, recv_buff, 1024, 0, (sockaddr*)&addr_recv, &addr_recv_len);
        if(ret_val == SOCKET_ERROR){
            if(WSAGetLastError() == WSAETIMEDOUT)
            {
                std::cerr<<"Line "<<__LINE__<<"-- recvfrom timeout "<<WSAETIMEDOUT<<std::endl;
                route.push_back(string("*.*.*.*")); // Timeout means this host do not respond ICMP ECHO request
                continue;
            }else{
                std::cerr<<"Line "<<__LINE__<<"--";
                std::cerr<<"sendto return "<<ret_val<<" WSAGetLastError() "<<WSAGetLastError()<<std::endl;
                close(sock_main);
                return route;
            }

        }else if(ret_val < sizeof(IP_HEADER)+sizeof(ICMP_HEADER)){
            std::cerr<<"Line "<<__LINE__<<"--";
            std::cerr<<"recvfrom get too few data "<<ret_val<<std::endl;
        }

        if(pIcmpRecv->id != GetCurrentProcessId())
        {
            //std::cerr<<" someone else's packet! id = "<<pIcmpRecv->id<<endl;
            //continue;
        }
        if(pIcmpRecv->type != 0)
        {
            switch(pIcmpRecv->type){
            case 11:
                if(print_debug_info)cout<<"Time exceeded message from "<<inet_ntoa(addr_recv.sin_addr)<<endl;
                route.push_back(string(inet_ntoa(addr_recv.sin_addr)));
                break;

            default:
                break;
            }
            if(print_debug_info)cout<<" nonecho type  "<< (unsigned int)(pIcmpRecv->type)<<endl;
            if(print_debug_info)cout<<" nonecho code  "<< (unsigned int)(pIcmpRecv->code)<<endl;
            continue;
        }else if(pIcmpRecv->type == 0){
            break;
        }
    }while(true);
    if(sock_main>0){
        close(sock_main);
    }
    return route;
}

