# EventSystem

# Example usage

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
