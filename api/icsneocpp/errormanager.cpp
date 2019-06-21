#include "icsneo/api/errormanager.h"
#include <memory>

using namespace icsneo;

static std::unique_ptr<ErrorManager> singleton;

ErrorManager& ErrorManager::GetInstance() {
	if(!singleton)
		singleton = std::unique_ptr<ErrorManager>(new ErrorManager());
	return *singleton.get();
}

void ErrorManager::get(std::vector<APIError>& errorOutput, size_t max, ErrorFilter filter) {
	std::lock_guard<std::mutex> lk(mutex);

	if(max == 0) // A limit of 0 indicates no limit
		max = (size_t)-1;

	size_t count = 0;
	errorOutput.clear();
	auto it = errors.begin();
	while(it != errors.end()) {
		if(filter.match(*it)) {
			errorOutput.push_back(*it);
			errors.erase(it++);
			if(count++ >= max)
				break; // We now have as many written to output as we can
		} else {
			std::advance(it, 1);
		}
	}
}

bool ErrorManager::getLastError(APIError& errorOutput, ErrorFilter filter) {
	std::lock_guard<std::mutex> lk(mutex);
	
	auto it = errors.rbegin();
	while(it != errors.rend()) {
		if(filter.match(*it)) {
			errorOutput = *it;
			errors.erase(std::next(it).base());
			return true;
		}
		std::advance(it, 1);
	}

	return false;
}

bool ErrorManager::getLastError(APIError& errorOutput, std::thread::id id) {
	auto iter = lastUserErrors.find(id);
	if(iter == lastUserErrors.end()) {
		return false;
	} else {
		errorOutput = iter->second;
		return true;
	}
}

void ErrorManager::discard(ErrorFilter filter) {
	std::lock_guard<std::mutex> lk(mutex);
	errors.remove_if([&filter](const APIError& error) {
		return filter.match(error);
	});
}

size_t ErrorManager::count_internal(ErrorFilter filter) const {
	size_t ret = 0;
	for(auto& error : errors)
		if(filter.match(error))
			ret++;
	return ret;
}

/**
 * Ensures errors is always at most errorLimit - 1 in size.
 * Returns true if any errors were removed in the process of doing so.
 */
bool ErrorManager::enforceLimit() {
	// Remove all TooManyErrors before checking
	errors.remove_if([](icsneo::APIError err){ return err.getType() == icsneo::APIError::TooManyErrors; });
	
	// We are not overflowing
	if(errors.size() < errorLimit)
		return false;

	size_t amountToRemove = errors.size() + 1 - errorLimit;

	discardLeastSevere(amountToRemove);

	return true;
}

APIError::Severity ErrorManager::lowestCurrentSeverity() {
	if(errors.empty())
		return APIError::Severity(0);

	APIError::Severity lowest = APIError::Severity::Error;
	auto it = errors.begin();
	while(it != errors.end()) {
		if((*it).getSeverity() < lowest)
			lowest = (*it).getSeverity();
		it++;
	}
	return lowest;
}

void ErrorManager::discardLeastSevere(size_t count) {
	if(count == 0)
		return;

	// Erase needed Info level errors, starting from the beginning
	ErrorFilter infoFilter(APIError::Severity::Info);
	auto it = errors.begin();
	while(it != errors.end()) {
		if(infoFilter.match(*it)) {
			errors.erase(it++);
			if(--count == 0)
				break;
		} else {
			it++;
		}
	}

	// Erase needed Warning level errors, starting from the beginning
	if(count != 0) {
		ErrorFilter warningFilter(APIError::Severity::Warning);
		it = errors.begin();
		while(it != errors.end()) {
			if(warningFilter.match(*it)) {
				errors.erase(it++);
				if(--count == 0)
					break;
			} else {
				it++;
			}
		}
	}

	// Erase needed Error level errors, starting from the beginning
	if(count != 0) {
		ErrorFilter errorFilter(APIError::Severity::Error);
		it = errors.begin();
		while(it != errors.end()) {
			if(errorFilter.match(*it)) {
				errors.erase(it++);
				if(--count == 0)
					break;
			} else {
				it++;
			}
		}
	}
}