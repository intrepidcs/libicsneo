#ifndef __ICSNEO_LIFETIME_H_
#define __ICSNEO_LIFETIME_H_

#include <functional>

namespace icsneo {

class Lifetime {
public:
	Lifetime() = default;
	Lifetime(std::function<void(void)> onDeath) : fnOnDeath(onDeath) {}
	~Lifetime() {
		if(fnOnDeath)
			fnOnDeath();
	}

	// Disallow copies so the fnOnDeath only happens once
	Lifetime(const Lifetime&) = delete;
	Lifetime& operator=(const Lifetime&) = delete;

	// Explicitly allow moves
	Lifetime(Lifetime&& moved) { *this = std::move(moved); }
	Lifetime& operator=(Lifetime&& rhs) {
		fnOnDeath = std::move(rhs.fnOnDeath);
		rhs.fnOnDeath = std::function<void(void)>();
		return *this;
	}
private:
	std::function<void(void)> fnOnDeath;
};

}

#endif