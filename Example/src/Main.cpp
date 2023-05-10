#include <iostream>
#include "EventManager.h"

// events
struct LevelDown {
	int level;
};

struct LevelUp {
	int level;
};

// event handlers / listeners / subscribers
void handleLevelUp(const LevelUp& event) {
	std::cout << "level: " << event.level << '\n';
}
void handleLevelDown(const LevelDown& event) {
	std::cout << "downlevel: " << event.level << '\n';
}
void levelDownConsiquence(const LevelDown& event) {
	std::cout << "downlevel consiquence: " << event.level << '\n';
}

class DownLevelClass {
public:
	explicit DownLevelClass(EventManager* em) {
		em->subscribe<LevelDown>(&DownLevelClass::method, this);
	}
private:
	void method(const LevelDown& event) const {
		std::cout << "method down level: " << event.level << '\n';
	}
};

int main() {
    // EventManager instance only manages busses, subscribed functions are static
	EventManager em;

	int level{ 0 };

    // ways of subscribing
	auto levelUpHandle = em.subscribe<LevelUp>(&handleLevelUp);
	em.subscribe<LevelDown>(&handleLevelDown);
	em.subscribe<LevelDown>(&levelDownConsiquence);
	DownLevelClass DLC(&em);

	level--;
	em.publishBlocking<LevelDown>({ level });
	level++;
	em.publishBus<LevelUp>({ level });
    level++;
	em.publishBlocking<LevelUp>({ level });
    level++;
    em.publishBus<LevelUp>({ level });
	em.pollEvents();
    // publishing to the same event to the bus will not override, instead it will stack

	em.unsubscribe<LevelUp>(levelUpHandle);
    // will not fire
    em.publishBlocking<LevelUp>({ level });
}
