#include <cstring>
#include "icsneo/communication/interprocessmailbox.h"

using namespace icsneo;

bool InterprocessMailbox::open(const std::string& name, bool create)
{
	if(!queuedSem.open(name + "-qs", create))
		return false;

	if(!emptySem.open(name + "-es", create, MESSAGE_COUNT))
		return false;

	if(!sharedMem.open(name + "-sm", BLOCK_SIZE * MESSAGE_COUNT, create))
		return false;

	valid = true;
	return true;
}

InterprocessMailbox::operator bool() const
{
	return valid;
}

bool InterprocessMailbox::close()
{
	valid = false;
	return queuedSem.close() && emptySem.close() && sharedMem.close();
}

bool InterprocessMailbox::read(void* data, LengthFieldType& messageLength, const std::chrono::milliseconds& timeout)
{
	if(!queuedSem.wait(timeout))
		return false;
	auto it = sharedMem.data() + (index * BLOCK_SIZE);
	messageLength = *(LengthFieldType*)it;
	it += LENGTH_FIELD_SIZE;
	std::memcpy(data, it, std::min(messageLength, MAX_DATA_SIZE));
	if(!emptySem.post())
		return false;
	++index;
	index %= MESSAGE_COUNT;
	return true;
}

bool InterprocessMailbox::write(const void* data, LengthFieldType messageLength, const std::chrono::milliseconds& timeout)
{
	if(!emptySem.wait(timeout))
		return false; // the buffer is full and we timed out
	auto it = sharedMem.data() + (index * BLOCK_SIZE);
	*(LengthFieldType*)it = messageLength;
	it += LENGTH_FIELD_SIZE;
	std::memcpy(it, data, messageLength);
	if(!queuedSem.post())
		return false;
	++index;
	index %= MESSAGE_COUNT;
	return true;
}
