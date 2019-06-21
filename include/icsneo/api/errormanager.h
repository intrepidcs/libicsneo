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
	void get(std::vector<APIError>& outErrors, ErrorFilter filter, size_t max = 0) { get(outErrors, max, filter); }
	void get(std::vector<APIError>& outErrors, size_t max = 0, ErrorFilter filter = ErrorFilter());
	bool getLastError(APIError& outErrors, ErrorFilter filter = ErrorFilter());

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
		if(newLimit == errorLimit)
			return;
		
		if(newLimit < 10) {
			add(APIError::ParameterOutOfRange);
			return;
		}

		std::lock_guard<std::mutex> lk(mutex);
		errorLimit = newLimit;
		if(enforceLimit()) 
			add(APIError::TooManyErrors);
	}

	size_t getErrorLimit() const { return errorLimit; }

private:
	ErrorManager() {}
	// Used by functions for threadsafety
	mutable std::mutex mutex;

	// Stores all errors
	std::list<APIError> errors;
	size_t errorLimit = 10000;

	size_t count_internal(ErrorFilter filter = ErrorFilter()) const;

	// If errors is not full, add the error at the end
	// Otherwise, remove the least significant errors, push the error to the back and push a APIError::TooManyErrors to the back (in that order)
	void add_internal(APIError error) {
		// Ensure the error list is at most exactly full (size of errorLimit - 1, leaving room for a potential APIError::TooManyErrors) 
		enforceLimit();

		// We are exactly full, either because the list was truncated or because we were simply full before
		if(errors.size() == errorLimit - 1) {
			// If the error is worth adding
			if(APIError::SeverityForType(error.getType()) >= lowestCurrentSeverity()) {
				discardLeastSevere(1);
				errors.push_back(error);	
			}

			errors.push_back(APIError(APIError::TooManyErrors));
		} else {
			errors.push_back(error);
		}
	}
	void add_internal(APIError::ErrorType type) {
		// Ensure the error list is at most exactly full (size of errorLimit - 1, leaving room for a potential APIError::TooManyErrors) 
		enforceLimit();

		// We are exactly full, either because the list was truncated or because we were simply full before
		if(errors.size() == errorLimit - 1) {
			// If the error is worth adding
			if(APIError::SeverityForType(type) >= lowestCurrentSeverity()) {
				discardLeastSevere(1);
				errors.emplace_back(type);
			}

			errors.push_back(APIError(APIError::TooManyErrors));
		} else {
			errors.emplace_back(type);
		}
	}
	void add_internal(APIError::ErrorType type, const Device* forDevice) {
		// Ensure the error list is at most exactly full (size of errorLimit - 1, leaving room for a potential APIError::TooManyErrors) 
		enforceLimit();

		// We are exactly full, either because the list was truncated or because we were simply full before
		if(errors.size() == errorLimit - 1) {
			// If the error is worth adding
			if(APIError::SeverityForType(type) >= lowestCurrentSeverity()) {
				discardLeastSevere(1);
				errors.emplace_back(type);
			}

			errors.push_back(APIError(APIError::TooManyErrors));
		} else {
			errors.emplace_back(type, forDevice);
		}
	}

	bool enforceLimit(); // Returns whether the limit enforcement resulted in an overflow

	APIError::Severity lowestCurrentSeverity();
	void discardLeastSevere(size_t count = 1);
};

}

#endif