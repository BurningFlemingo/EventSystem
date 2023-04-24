# Single Header EventSystem

## Credit
these posts / articles were used to help me make this.

https://austinmorlan.com/posts/entity_component_system/

https://codereview.stackexchange.com/questions/252884/c-event-system-game-engine

https://codereview.stackexchange.com/questions/79211/ecs-event-messaging-implementation

## Design goals

* as few virtual methods as possible
* events do not derive from base event class
* no explicit registering of types, POD structs should just work
* simple api interaction 
    * no std::bind
    * no lambdas
* fast, contiguoues event dispatching
* single header

## Features

### subscribe

* subscribes to an EventType POD
* when EventType gets published, every function attached to that EventType will be called
* returns a handle that can be used to unsubscribe, ignorable
* requires ```&function```
* requires EventType, and this event type must be a POD struct
* requires function to be void, and have const refrence POD as its only parameter i.e. ```void function(const POD& pod);```
* requires function parameter type to be the same as the EventType called with subscribe


### publish

* POD instance will be used as arguments for the EventType attached functions
* blocking
    * will call every function attached to the published EventType immediately one after another
* bus
    * will delay every function call attached to the published EventType to when ```pollEvents();``` is called
* requires POD EventType and an instance of the same POD EventType

    
### unsubscribe

* takes in a handle and an EventType to immediatly unsubscribe a function from being called when EventType gets published
* requires valid handle

### poll events

* will call every event published by ```publishBus();```
* will clear event bus after every event is called

## Example usage

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
	    EventManager em;

	    int level{ 0 };

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
