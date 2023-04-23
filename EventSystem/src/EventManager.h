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
	template<class EventType>
	using CallbackType = std::function<void(const EventType&)>;
	using SubscriberHandle = size_t;

	SubscriberHandle addCallback(CallbackType<EventType> callback);

	void removeCallback(SubscriberHandle handle);

	template<typename EventType>
	void operator() (const EventType& event) const {
		for (auto& callback : m_Callbacks) {
			callback(event);
		}
	}

	void save(const EventType& event);
	void callSaved() const override;

private:
	std::vector<CallbackType<EventType>> m_Callbacks{};
	std::vector<SubscriberHandle> m_FreeHandles{};
	std::unordered_map<SubscriberHandle, size_t> m_HandleToIndex{};
	std::unordered_map<size_t, SubscriberHandle> m_IndexToHandle{};

	EventType m_SavedEvent{};
};

template<typename EventType>
auto CallbackContainer<EventType>::addCallback(CallbackType<EventType> callback) -> SubscriberHandle {
	SubscriberHandle handle;
	size_t newIndex = m_Callbacks.size();

	if (m_FreeHandles.empty()) {
		handle = m_Callbacks.size();
	}
	else {
		handle = m_FreeHandles.back();
		m_FreeHandles.pop_back();
	}
	m_HandleToIndex[handle] = newIndex;
	m_IndexToHandle[newIndex] = handle;

	if (newIndex >= m_Callbacks.size()) {
		m_Callbacks.resize(newIndex + 1);
	}
	m_Callbacks[newIndex] = callback;
	return handle;
}

template<typename EventType>
void CallbackContainer<EventType>::removeCallback(SubscriberHandle handle) {
	assert(m_HandleToIndex.find(handle) != m_HandleToIndex.end());

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

	m_HandleToIndex.erase(handle);
	m_IndexToHandle.erase(indexOfLastElement);
	m_FreeHandles.emplace_back(handle);
}

template<typename EventType>
void CallbackContainer<EventType>::save(const EventType& event) {
	m_SavedEvent = event;
}

template<typename EventType>
void CallbackContainer<EventType>::callSaved() const {
	for (auto& callback : m_Callbacks) {
		callback(m_SavedEvent);
	}
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
	std::vector<const ICallbackContainer*> m_EventBus;
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


void EventManager::pollEvents() {
	for (const auto& callback : m_EventBus) {
		callback->callSaved();
	}
	m_EventBus.clear();
}
