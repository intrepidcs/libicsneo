#include "icsneo/communication/livedata.h"
#include <cmath>
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

std::optional<LiveDataValue> liveDataDoubleToValue(const double& dFloat) {
	LiveDataValue value;
	union {
		struct
		{
			uint32_t ValueFractionPart;
			int32_t ValueInt32;
		} parts;
		int64_t ValueLarge;
	} CminiFixedPt;
	constexpr double CM_FIXED_POINT_TO_DOUBLEVALUE = (1.0 / (double)(1ULL << 32)); // 2^-32
	constexpr double CM_DOUBLEVALUE_TO_FIXED_POINT = ((double)(1ULL << 32)); // 2^32
	// Use const for limits (C++98 compatible)
	const double INT32_MAX_DOUBLE =
		static_cast<double>(std::numeric_limits<int32_t>::max()) + (1.0 - std::numeric_limits<double>::epsilon());
	const double INT32_MIN_DOUBLE = static_cast<double>(std::numeric_limits<int32_t>::min());
	const double MIN_FIXED_POINT_DOUBLE = (double)(1ull * CM_FIXED_POINT_TO_DOUBLEVALUE);

	// This needs to be assigned separately, otherwise, for dFloat >= 2^31,
	// long double dBigFloat = dFloat * CM_DOUBLEVALUE_TO_FIXED_POINT overflows
	// long long (value is >= 2^63) and so the assignment ValueLarge = dBigFloat is undefined

	int32_t intPart; //creating temp variable due to static analysis warning about writing and reading to different union members
	if(dFloat < 0.0)
		intPart = (int32_t)std::floor(dFloat);
	else
		intPart = (int32_t)dFloat;

	//using temp varialbes to avoid static analysis warning about read/write to different union members
	double frac = dFloat - (double)(intPart);
	uint32_t fracPart = (uint32_t)std::floor((frac * CM_DOUBLEVALUE_TO_FIXED_POINT) + 0.5);

	//write temp vars back into the union
	CminiFixedPt.parts.ValueInt32 = intPart;
	CminiFixedPt.parts.ValueFractionPart = fracPart;
	value.value = CminiFixedPt.ValueLarge;

	if(dFloat == (double)0.0)
		return value;

	//check if double can be stored as 32.32
	// 0x1 0000 0000 0000 0000 * CM_FIXED_POINT_TO_DOUBLEVALUE = 0x1 0000 0000
	if(dFloat > INT32_MAX_DOUBLE || dFloat < INT32_MIN_DOUBLE) {
		EventManager::GetInstance().add(APIEvent::Type::FixedPointOverflow, APIEvent::Severity::Error);
		return std::nullopt;
	}

	// Use absolute value for minimum fixed point check
	double absFloat = (dFloat < 0.0) ? -dFloat : dFloat;
	if(absFloat < MIN_FIXED_POINT_DOUBLE) {
		EventManager::GetInstance().add(APIEvent::Type::FixedPointPrecision, APIEvent::Severity::Error);
		return std::nullopt;
	}

	return value;
}

} // namespace LiveDataUtil
} // namespace icsneo