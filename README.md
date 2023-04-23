# Single Header EventSystem

## Credit
these posts / articles were used to help me make this.

https://austinmorlan.com/posts/entity_component_system/

https://codereview.stackexchange.com/questions/252884/c-event-system-game-engine

https://codereview.stackexchange.com/questions/79211/ecs-event-messaging-implementation

## Example usage

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

    int main() {
	    EventManager em;

	    int level{ 0 };

	    auto levelUpHandle = em.subscribe<LevelUp>(&handleLevelUp);
	    auto levelDownHandle = em.subscribe<LevelDown>(&handleLevelDown);
	    auto levelDownConsiquenceHandle = em.subscribe<LevelDown>(&levelDownConsiquence);

	    level++;
	    em.publishBus<LevelUp>({ level });
	    em.publishBlocking<LevelUp>({ level });
	    em.pollEvents();
	    em.unsubscribe<LevelUp>(levelUpHandle);
      }
