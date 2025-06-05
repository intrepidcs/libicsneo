#include "icsneo/platform/windows/internal/pcapdll.h"
#include <tchar.h>

using namespace icsneo;

const PCAPDLL& PCAPDLL::getInstance()
{
	static PCAPDLL instance;
	return instance;
}

PCAPDLL::~PCAPDLL()
{
	closeDLL();
}

void PCAPDLL::closeDLL()
{
	if (dll)
		FreeLibrary(dll);
	dll = nullptr;
}

bool PCAPDLL::ok() const
{
	return dll != nullptr;
}

PCAPDLL::PCAPDLL()
{
	TCHAR dir[512] = {0};
	if(GetSystemDirectory(dir, 480)) {
		_tcscat_s(dir, 512, TEXT("\\Npcap"));
		if(SetDllDirectory(dir)) {
			// will search for Npcap first, then fall back to WinPcap
			dll = LoadLibrary(TEXT("wpcap.dll"));
			SetDllDirectory(nullptr); // reset
		}
	}

	if(dll == NULL) {
		closeDLL();
	} else {
		findalldevs_ex = (PCAPFINDDEVICE)GetProcAddress(dll, "pcap_findalldevs_ex");
		open = (PCAPOPEN)GetProcAddress(dll, "pcap_open");
		freealldevs = (PCAPFREEDEVS)GetProcAddress(dll, "pcap_freealldevs");
		close = (PCAPCLOSE)GetProcAddress(dll, "pcap_close");
		stats = (PCAPSTATS)GetProcAddress(dll, "pcap_stats");
		next_ex = (PCAPNEXTEX)GetProcAddress(dll, "pcap_next_ex");
		sendpacket = (PCAPSENDPACKET)GetProcAddress(dll, "pcap_sendpacket");
		sendqueue_alloc = (PCAPSENDQUEUEALLOC)GetProcAddress(dll, "pcap_sendqueue_alloc");
		sendqueue_queue = (PCAPSENDQUEUEQUEUE)GetProcAddress(dll, "pcap_sendqueue_queue");
		sendqueue_destroy = (PCAPSENDQUEUEDESTROY)GetProcAddress(dll, "pcap_sendqueue_destroy");
		sendqueue_transmit = (PCAPSENDQUEUETRANSMIT)GetProcAddress(dll, "pcap_sendqueue_transmit");
		datalink = (PCAPDATALINK)GetProcAddress(dll, "pcap_datalink");
		createsrcstr = (PCAPCREATESRCSTR)GetProcAddress(dll, "pcap_createsrcstr");
		setbuff = (PCAPSETBUFF)GetProcAddress(dll, "pcap_setbuff");
		if(findalldevs_ex == NULL || open == NULL ||
			freealldevs == NULL || close == NULL ||
			stats == NULL || next_ex == NULL ||
			sendpacket == NULL ||
			sendqueue_alloc == NULL ||
			sendqueue_queue == NULL || sendqueue_destroy == NULL ||
			sendqueue_transmit == NULL ||
			datalink == NULL ||
			createsrcstr == NULL || setbuff == NULL) {
			closeDLL();
		}
	}
}
