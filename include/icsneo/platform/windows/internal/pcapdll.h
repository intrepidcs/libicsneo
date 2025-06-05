#ifndef __PCAPDLL_WINDOWS_H_
#define __PCAPDLL_WINDOWS_H_

#ifdef __cplusplus

#include <windows.h>
#include <pcap.h>
#include <memory>

namespace icsneo {

// Helper loader for the PCAP DLL
class PCAPDLL {
public:
	// The first time we use the DLL we keep it in here and it won't get freed until the user unloads us (for speed reasons)
	static const PCAPDLL& getInstance();

	// Functions
	typedef int(__cdecl* PCAPFINDDEVICE)(char* source, struct pcap_rmtauth* auth, pcap_if_t** alldevs, char* errbuf);
	typedef pcap_t*(__cdecl* PCAPOPEN)(const char* source, int snaplen, int flags, int read_timeout, struct pcap_rmtauth* auth, char* errbuf);
	typedef void(__cdecl* PCAPFREEDEVS)(pcap_if_t* alldevsp);
	typedef void(__cdecl* PCAPCLOSE)(pcap_t* p);
	typedef int(__cdecl* PCAPSTATS)(pcap_t* p, struct pcap_stat* ps);
	typedef int(__cdecl* PCAPNEXTEX)(pcap_t* p, struct pcap_pkthdr** pkt_header, const unsigned char** pkt_data);
	typedef int(__cdecl* PCAPSENDPACKET)(pcap_t* p, const unsigned char* buf, int size);
	typedef pcap_send_queue*(__cdecl* PCAPSENDQUEUEALLOC)(unsigned int memsize);
	typedef int(__cdecl* PCAPSENDQUEUEQUEUE)(pcap_send_queue* queue, const struct pcap_pkthdr* pkt_header, const unsigned char* pkt_data);
	typedef void(__cdecl* PCAPSENDQUEUEDESTROY)(pcap_send_queue* queue);
	typedef unsigned int(__cdecl* PCAPSENDQUEUETRANSMIT)(pcap_t* p, pcap_send_queue* queue, int sync);
	typedef int(__cdecl* PCAPDATALINK)(pcap_t* p);
	typedef int(__cdecl* PCAPCREATESRCSTR)(char* source, int type, const char* host, const char* port, const char* name, char* errbuf);
	typedef int(__cdecl* PCAPSETBUFF)(pcap_t* p, int dim);
	PCAPFINDDEVICE findalldevs_ex;
	PCAPOPEN open;
	PCAPFREEDEVS freealldevs;
	PCAPCLOSE close;
	PCAPSTATS stats;
	PCAPNEXTEX next_ex;
	PCAPSENDPACKET sendpacket;
	PCAPSENDQUEUEALLOC sendqueue_alloc;
	PCAPSENDQUEUEQUEUE sendqueue_queue;
	PCAPSENDQUEUEDESTROY sendqueue_destroy;
	PCAPSENDQUEUETRANSMIT sendqueue_transmit;
	PCAPDATALINK datalink;
	PCAPCREATESRCSTR createsrcstr;
	PCAPSETBUFF setbuff;


	~PCAPDLL();
	bool ok() const;
private:
	PCAPDLL();
	HINSTANCE dll = nullptr;
	void closeDLL();
};
}

#endif // __cplusplus

#endif