#include <iostream>
#include "EventManager.h"

struct LevelDown {
	int level;
};

struct LevelUp {
	int level;
};

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
	DownLevelClass(EventManager* em) {
		em->subscribe<LevelDown>(&DownLevelClass::method, this);
	}
private:
	void method(const LevelDown& event) {
		std::cout << "method down level: " << event.level << std::endl;
	}
};

int main() {
	EventManager em;

	int level = 0;
	int number = 1000000;

	auto levelUpHandle = em.subscribe<LevelUp>(&handleLevelUp);
	auto levelDownHandle = em.subscribe<LevelDown>(&handleLevelDown);
	auto levelDownConsiquenceHandle = em.subscribe<LevelDown>(&levelDownConsiquence);
	DownLevelClass DLC(&em);

	level--;
	em.publishBlocking<LevelDown>({ level });
	level++;
	em.publishBus<LevelUp>({ level });
	em.publishBlocking<LevelUp>({ level });
	em.pollEvents();

	em.unsubscribe<LevelUp>(levelUpHandle);
}