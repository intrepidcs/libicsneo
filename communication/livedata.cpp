#include "icsneo/communication/livedata.h"
namespace icsneo {

namespace LiveDataUtil {

LiveDataHandle getNewHandle() {
	static LiveDataHandle currentHandle = 0;
	++currentHandle;
	if(currentHandle == std::numeric_limits<LiveDataHandle>::max()) {
		EventManager::GetInstance().add(APIEvent::Type::LiveDataInvalidHandle, APIEvent::Severity::Error);
		currentHandle = 1;
	}
	return currentHandle;
}

double liveDataValueToDouble(const LiveDataValue& val) {
	constexpr double liveDataFixedPointToDouble = 0.00000000023283064365386962890625;
	return val.value * liveDataFixedPointToDouble; 
} 

} // namespace LiveDataUtil
} // namespace icsneo