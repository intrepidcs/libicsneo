#ifndef __ICSNEO_API_ERRORMANAGER_H_
#define __ICSNEO_API_ERRORMANAGER_H_

#include <vector>
#include <list>
#include <mutex>
#include <functional>
#include <unordered_map>
#include <thread>
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
	bool getLastError(APIError& errorOutput, std::thread::id id);

	void add(APIError error) {
		std::lock_guard<std::mutex> lk(mutex);
		add_internal(error);
	}
	void add(APIError error, std::thread::id id) {
		std::lock_guard<std::mutex> lk(mutex);
		if(id == std::thread::id())
			add_internal(error);
		else 
			add_internal_threaded(error, id);
	}
	void add(APIError::ErrorType type) {
		std::lock_guard<std::mutex> lk(mutex);
		add_internal(APIError::APIError(type));
	}
	void add(APIError::ErrorType type, std::thread::id id) {
		std::lock_guard<std::mutex> lk(mutex);
		if(id == std::thread::id())
			add_internal(APIError::APIError(type));
		else
			add_internal_threaded(APIError::APIError(type), id);
		
	}
	void add(APIError::ErrorType type, const Device* forDevice) {
		std::lock_guard<std::mutex> lk(mutex);
		add_internal(APIError::APIError(type, forDevice));
	}
	void add(APIError::ErrorType type, const Device* forDevice, std::thread::id id) {
		std::lock_guard<std::mutex> lk(mutex);
		if(id == std::thread::id())
			add_internal(APIError::APIError(type, forDevice));
		else
			add_internal_threaded(APIError::APIError(type, forDevice), id);
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
	std::unordered_map<std::thread::id, APIError> lastUserErrors;
	size_t errorLimit = 10000;

	size_t count_internal(ErrorFilter filter = ErrorFilter()) const;

	/**
	 * Places a {id, error} pair into the lastUserErrors
	 * If the key id already exists in the map, replace the error of that pair with the new one
	 */
	void add_internal_threaded(APIError error, std::thread::id id) {
		auto iter = lastUserErrors.find(id);
		if(iter == lastUserErrors.end()) {
			lastUserErrors.insert(std::make_pair(id, error));
		} else {
			iter->second = error;
		}
	}

	/**
	 * If errors is not full, add the error at the end
	 * Otherwise, remove the least significant errors, push the error to the back and push a APIError::TooManyErrors to the back (in that order)
	 */
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

	bool enforceLimit(); // Returns whether the limit enforcement resulted in an overflow

	APIError::Severity lowestCurrentSeverity();
	void discardLeastSevere(size_t count = 1);
};

}

#endif