#ifndef __ICSNEO_API_ERRORMANAGER_H_
#define __ICSNEO_API_ERRORMANAGER_H_

#include <vector>
#include <list>
#include <mutex>
#include <functional>
#include "icsneo/api/error.h"

namespace icsneo {

typedef std::function<void (APIError::ErrorType)> device_errorhandler_t;

class ErrorManager {
public:
	static ErrorManager& GetInstance();

	size_t count(ErrorFilter filter = ErrorFilter()) const {
		std::lock_guard<std::mutex> lk(mutex);
		return count_internal(filter);
	};

	std::vector<APIError> get(ErrorFilter filter, size_t max = 0) { return get(max, filter); }
	std::vector<APIError> get(size_t max = 0, ErrorFilter filter = ErrorFilter()) {
		std::vector<APIError> ret;
		get(ret, filter, max);
		return ret;
	}
	void get(std::vector<APIError>& errors, ErrorFilter filter, size_t max = 0) { get(errors, max, filter); }
	void get(std::vector<APIError>& errors, size_t max = 0, ErrorFilter filter = ErrorFilter());
	bool getOne(APIError& error, ErrorFilter filter = ErrorFilter());

	void add(APIError error) {
		std::lock_guard<std::mutex> lk(mutex);
		add_internal(error);
	}
	void add(APIError::ErrorType type) {
		std::lock_guard<std::mutex> lk(mutex);
		add_internal(type);
	}
	void add(APIError::ErrorType type, const Device* forDevice) {
		std::lock_guard<std::mutex> lk(mutex);
		add_internal(type, forDevice);
	}

	void discard(ErrorFilter filter = ErrorFilter());

	void setErrorLimit(size_t newLimit) {
		if(newLimit < 10) {
			add(APIError::ParameterOutOfRange);
			return;
		}

		std::lock_guard<std::mutex> lk(mutex);
		errorLimit = newLimit;
		enforceLimit();
	}

	size_t getErrorLimit() const { return errorLimit; }

private:
	ErrorManager() {}
	mutable std::mutex mutex;
	std::list<APIError> errors;
	size_t errorLimit = 10000;

	size_t count_internal(ErrorFilter filter = ErrorFilter()) const;

	void add_internal(APIError error) {
		if(!beforeAddCheck(error.getType()))
			return;
		errors.push_back(error);
		enforceLimit();
	}
	void add_internal(APIError::ErrorType type) {
		if(!beforeAddCheck(type))
			return;
		errors.emplace_back(type);
		enforceLimit();
	}
	void add_internal(APIError::ErrorType type, const Device* forDevice) {
		if(!beforeAddCheck(type))
			return;
		errors.emplace_back(type, forDevice);
		enforceLimit();
	}

	bool beforeAddCheck(APIError::ErrorType type); // Returns whether the error should be added

	bool enforceLimit(); // Returns whether the limit enforcement resulted in an overflow

	APIError::Severity lowestCurrentSeverity();
	void discardLeastSevere(size_t count = 1);
};

}

#endif