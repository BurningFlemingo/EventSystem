#pragma once
#include <vector>
#include <functional>
#include <memory>
#include <unordered_map>
#include <assert.h>

class ICallbackContainer {
public:
	virtual ~ICallbackContainer() = default;

	virtual void callSaved() const = 0;
};

template<typename EventType>
class CallbackContainer : public ICallbackContainer {
public:
    CallbackContainer();

	using CallbackType = std::function<void(const EventType&)>;
	using SubscriberHandle = size_t;

	SubscriberHandle addCallback(CallbackType callback);

	void removeCallback(SubscriberHandle handle);

	void operator() (const EventType& event) const {
		for (int i = 0; i < m_NumberOfCallbacks; i++) {
			m_Callbacks[i](event);
		}
	}

	void save(const EventType& event);
	void callSaved() const override;
private:
    void clearSaved() const;

	std::vector<CallbackType> m_Callbacks;
	std::vector<SubscriberHandle> m_FreeHandles;
    std::vector<size_t> m_HandleToIndex;
    std::vector<SubscriberHandle> m_IndexToHandle;

    mutable std::vector<EventType> m_SavedEvents{};
    mutable unsigned int m_NumberOfCallbacks{ 0 };
};

template<typename EventType>
inline CallbackContainer<EventType>::CallbackContainer() {
    m_Callbacks.assign(10, 0);
    m_HandleToIndex.assign(10, 0);
    m_IndexToHandle.assign(10, 0);
}

template<typename EventType>
inline auto CallbackContainer<EventType>::addCallback(CallbackType callback) -> SubscriberHandle {
	SubscriberHandle handle;
	size_t newIndex = m_NumberOfCallbacks;

	if (m_FreeHandles.empty()) {
		handle = newIndex;
	}
	else {
		handle = m_FreeHandles.back();
		m_FreeHandles.pop_back();
	}

    if (m_IndexToHandle.size() <= newIndex) {
        m_IndexToHandle.resize(newIndex * 2 + 1);
    }
    if (m_HandleToIndex.size() <= handle) {
        m_HandleToIndex.resize(handle * 2 + 1);
    }

	m_HandleToIndex[handle] = newIndex;
	m_IndexToHandle[newIndex] = handle;

	if (newIndex >= m_Callbacks.size()) {
		m_Callbacks.resize(newIndex * 2 + 1);
	}
	m_Callbacks[newIndex] = callback;
    m_NumberOfCallbacks++;

	return handle;
}

template<typename EventType>
inline void CallbackContainer<EventType>::removeCallback(SubscriberHandle handle) {
	assert(m_HandleToIndex[handle] != -1);

	size_t indexOfRemovedHandle = m_HandleToIndex[handle];
	size_t indexOfLastElement = m_Callbacks.size() - 1;

	if (indexOfRemovedHandle != indexOfLastElement) {
		SubscriberHandle handleOfLastElement = m_IndexToHandle[indexOfLastElement];
		m_HandleToIndex[handleOfLastElement] = indexOfRemovedHandle;
		m_IndexToHandle[indexOfRemovedHandle] = handleOfLastElement;
		m_Callbacks[indexOfRemovedHandle] = m_Callbacks[indexOfLastElement];
	}
	else {
		m_Callbacks.pop_back();
	}

	m_HandleToIndex[handle] = -1;
	m_IndexToHandle[indexOfLastElement] = -1;
	m_FreeHandles.emplace_back(handle);

    m_NumberOfCallbacks--;
}

template<typename EventType>
inline void CallbackContainer<EventType>::save(const EventType& event) {
	m_SavedEvents.emplace_back(event);
}

template<typename EventType>
inline void CallbackContainer<EventType>::callSaved() const {
    if (m_SavedEvents.size() == 0) {
        return;
    }
	for (int i{0}; i < m_NumberOfCallbacks; i++) {
        for (const auto& event : m_SavedEvents) {
		    m_Callbacks[i](event);
        }
	}
    clearSaved();
}

template<typename EventType>
inline void CallbackContainer<EventType>::clearSaved() const {
    m_SavedEvents.clear();
}

class EventManager {
public:
	template<typename EventType, typename Function>
	typename CallbackContainer<EventType>::SubscriberHandle subscribe(Function callback);

	template<typename EventType, typename Method, typename Instance>
	typename CallbackContainer<EventType>::SubscriberHandle subscribe(Method callback, Instance instance);

	template<typename EventType>
	void unsubscribe(typename CallbackContainer<EventType>::SubscriberHandle handle);

	template<typename EventType>
	void publishBlocking(const EventType& event) const;

	template<typename EventType>
	void publishBlocking(EventType&& event) const;
	
	template<typename EventType>
	void publishBus(const EventType& event);

	template<typename EventType>
	void publishBus(EventType&& event);

	void pollEvents();

private:
	template<typename EventType>
	static inline CallbackContainer<EventType> s_Callbacks;
	std::vector<ICallbackContainer*> m_EventBus;
};

template<typename EventType, typename Function>
inline typename CallbackContainer<EventType>::SubscriberHandle EventManager::subscribe(Function callback) {
	return s_Callbacks<EventType>.addCallback(callback);
}

template<typename EventType, typename Method, typename Instance>
typename CallbackContainer<EventType>::SubscriberHandle EventManager::subscribe(Method callback, Instance instance) {
	std::function<void(const EventType&)> function{ std::bind(callback, instance, std::placeholders::_1) };
	return s_Callbacks<EventType>.addCallback(std::move(function));
}

template<typename EventType>
inline void EventManager::unsubscribe(typename CallbackContainer<EventType>::SubscriberHandle handle) {
	s_Callbacks<EventType>.removeCallback(handle);
}

template<typename EventType>
inline void EventManager::publishBlocking(const EventType& event) const {
	s_Callbacks<EventType>(event);
}

template<typename EventType>
inline void EventManager::publishBlocking(EventType&& event) const {
	s_Callbacks<EventType>(std::forward<EventType>(event));
}

template<typename EventType>
void EventManager::publishBus(const EventType& event) {
	s_Callbacks<EventType>.save(event);

	m_EventBus.emplace_back(&s_Callbacks<EventType>);
}

template<typename EventType>
void EventManager::publishBus(EventType&& event) {
	s_Callbacks<EventType>.save(std::forward<EventType>(event));

	m_EventBus.emplace_back(&s_Callbacks<EventType>);
}

inline void EventManager::pollEvents() {
	for (auto& callback : m_EventBus) {
		callback->callSaved();
	}

	m_EventBus.clear();
}
