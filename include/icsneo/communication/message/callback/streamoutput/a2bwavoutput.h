#ifndef __A2BWAVOUTPUT_H_
#define __A2BWAVOUTPUT_H_

#ifdef __cplusplus

#include "icsneo/communication/message/callback/streamoutput/streamoutput.h"
#include "icsneo/communication/message/a2bmessage.h"
#include "icsneo/device/device.h"

namespace icsneo {

class A2BWAVOutput : public StreamOutput {
public:
	A2BWAVOutput(const char* filename, uint32_t sampleRate = 44100) 
		: StreamOutput(filename), wavSampleRate(sampleRate) {}

	A2BWAVOutput(std::unique_ptr<std::ostream>&& os, uint32_t sampleRate = 44100) 
		: StreamOutput(std::move(os)), wavSampleRate(sampleRate) {}

	void writeHeader(const std::shared_ptr<A2BMessage>& firstMsg) const;

	bool callIfMatch(const std::shared_ptr<Message>& message) const override;

	void close() const;

	~A2BWAVOutput() override {
		if(!closed) {
			close();
		}
	}

protected:

	uint32_t wavSampleRate;
	mutable uint32_t streamStartPos;
	mutable bool firstMessageFlag = true;
	mutable bool closed = false;

};

}

#endif // __cplusplus

#endif
