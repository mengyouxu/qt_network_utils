#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include <string>
#include <list>


using namespace std;
struct align_test{

};
typedef struct _icmp_header  // size 12 bytes
{
    unsigned int type:8;
    unsigned int code:8;
    unsigned int checksum:16;

    unsigned int id:16;
    unsigned int seq_num:16;
    unsigned int timestamp;
}ICMP_HEADER;

typedef struct _ip_header  // size 20 bytes
{
    union
    {
        char header_data[20];
        struct
        {
            unsigned int version:4;
            unsigned int ihl:4;
            unsigned int dscp:6;
            unsigned int ecn:2;
            unsigned int total_len:16;
            // ^ 4bytes
            unsigned int id:16;
            unsigned int flags:4;
            unsigned int fragment_offset:12;
            // ^ 4bytes

            unsigned int time_to_live:8;
            unsigned int protocol:8;
            unsigned int checksum:16;
            // ^ 4bytes

            unsigned int src_addr;
            unsigned int dest_addr;
        };
    };
}IP_HEADER;

typedef struct _dns_header{ // 12 bytes
    unsigned short id;
    unsigned short flags;
    unsigned short question_count;
    unsigned short anwser_record_count;
    unsigned short authority_record_count;
    unsigned short additional_record_count;
}DNS_HEADER;
class network_utils
{
public:
    network_utils();
    ~network_utils();
    void printDebugInfo(bool print){print_debug_info = print;};
    string getHostName();
    bool isIPv4Addr(string *addr);
    bool isIPv6Addr(string *addr);

    int dnsPacketFactory(unsigned char* data,int size,string *hostname);
    list<string> getHostByName(string *name);
    list<string> getRoute(string *name);
    /* Param: name -> host name or address
     *        times -> how many times try to ping
     *
     * return percentage of lost packet, return 0 means the host is available
     */
    float pingHost(string *name,int times=4,int timeout=2000,int ttl=20);
    static unsigned short checksum(unsigned short *buffer, int size);
    static bool isHWBigEndian(){unsigned short i = 0x100; return (bool)((unsigned char*)&i)[1];};
private:
    bool print_debug_info;

};

#endif // NETWORK_UTILS_H
