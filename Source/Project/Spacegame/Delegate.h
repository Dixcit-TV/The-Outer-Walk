#pragma once
#include <functional>
#include <type_traits>

template<typename RET_TYPE, typename TARGET_TYPE, typename... ARG_TYPE>
struct Delegate : std::function<RET_TYPE(ARG_TYPE...)>
{
	using MFunction = RET_TYPE(TARGET_TYPE::*)(ARG_TYPE...);
	using MCFunction = RET_TYPE(TARGET_TYPE::* const)(ARG_TYPE...) const;

	explicit Delegate(TARGET_TYPE* pTarget, MFunction function)
		: std::function<RET_TYPE(ARG_TYPE...)>([=](ARG_TYPE&&... args) -> RET_TYPE { return (pTarget->*function)(std::forward<ARG_TYPE>(args)...); }) {};

	explicit Delegate(TARGET_TYPE* pTarget, MCFunction function)
		: std::function<RET_TYPE(ARG_TYPE...)>([=](ARG_TYPE&&... args) -> RET_TYPE { return (pTarget->*function)(std::forward<ARG_TYPE>(args)...); }) {};

	explicit Delegate(TARGET_TYPE& target, MFunction function) : Delegate(&target, function) {}

	explicit Delegate(TARGET_TYPE& target, MCFunction function) : Delegate(&target, function) {}
};

template<typename RET_TYPE, typename TARGET_TYPE, typename... ARG_TYPE>
Delegate<RET_TYPE, TARGET_TYPE, ARG_TYPE...> Make_Delegate(TARGET_TYPE& target, RET_TYPE(TARGET_TYPE::* function)(ARG_TYPE...) const)
{
	return Delegate<RET_TYPE, TARGET_TYPE, ARG_TYPE...>(target, function);
}

template<typename RET_TYPE, typename TARGET_TYPE, typename... ARG_TYPE>
Delegate<RET_TYPE, TARGET_TYPE, ARG_TYPE...> Make_Delegate(TARGET_TYPE& target, RET_TYPE(TARGET_TYPE::* function)(ARG_TYPE...))
{
	return Delegate<RET_TYPE, TARGET_TYPE, ARG_TYPE...>(target, function);
}

template<typename RET_TYPE, typename TARGET_TYPE, typename... ARG_TYPE>
Delegate<RET_TYPE, TARGET_TYPE, ARG_TYPE...> Make_Delegate(TARGET_TYPE* target, RET_TYPE(TARGET_TYPE::* function)(ARG_TYPE...) const)
{
	return Delegate<RET_TYPE, TARGET_TYPE, ARG_TYPE...>(target, function);
}

template<typename RET_TYPE, typename TARGET_TYPE, typename... ARG_TYPE>
Delegate<RET_TYPE, TARGET_TYPE, ARG_TYPE...> Make_Delegate(TARGET_TYPE* target, RET_TYPE(TARGET_TYPE::* function)(ARG_TYPE...))
{
	return Delegate<RET_TYPE, TARGET_TYPE, ARG_TYPE...>(target, function);
}

template<typename... ARG_TYPE>
class Event
{
public:
	explicit Event() = default;
	
	void Register(const std::string& evtName, std::function<void(ARG_TYPE...)> pCallback) { m_pCallbacks.emplace(evtName, pCallback); }
	void Unregister(const std::string evtName)
	{
		auto it = m_pCallbacks.find(evtName);

		if (it != std::end(m_pCallbacks))
			m_pCallbacks.erase(it);
	}

	void Invoke(ARG_TYPE&&...) const;

private:
	std::map<std::string, std::function<void(ARG_TYPE...)>> m_pCallbacks;
};

template<typename... ARG_TYPE>
void Event<ARG_TYPE...>::Invoke(ARG_TYPE&&... args) const
{
	for (const auto& callBackPair : m_pCallbacks)
		callBackPair.second(std::forward<ARG_TYPE>(args)...);
}
