#pragma once
#include "Singleton.h"

class PersistentDataManager final : public Singleton<PersistentDataManager>
{
	class IBlackBoardData
	{
	public:
		explicit IBlackBoardData() = default;
		IBlackBoardData(const IBlackBoardData& other) = delete;
		IBlackBoardData(IBlackBoardData&& other) noexcept = delete;
		IBlackBoardData& operator=(const IBlackBoardData& other) = delete;
		IBlackBoardData& operator=(IBlackBoardData&& other) noexcept = delete;
		virtual ~IBlackBoardData() = default;
	};

	template<typename DATATYPE>
	class BlackBoardData final : public IBlackBoardData
	{
	public:
		explicit BlackBoardData(const DATATYPE& data) : IBlackBoardData(), m_Data{ data } {}
		BlackBoardData(const BlackBoardData& other) = delete;
		BlackBoardData(BlackBoardData&& other) noexcept = delete;
		BlackBoardData& operator=(const BlackBoardData& other) = delete;
		BlackBoardData& operator=(BlackBoardData&& other) noexcept = delete;
		~BlackBoardData() override = default;
		DATATYPE m_Data;
	};

public:
	PersistentDataManager(const PersistentDataManager& other) = delete;
	PersistentDataManager(PersistentDataManager&& other) noexcept = delete;
	PersistentDataManager& operator=(const PersistentDataManager& other) = delete;
	PersistentDataManager& operator=(PersistentDataManager&& other) noexcept = delete;
	virtual ~PersistentDataManager()
	{
		for (auto& dataPair : m_pPersistentData)
		{
			SafeDelete(dataPair.second);
		}
	};

	template<typename DATATYPE>
	void Add(const std::string& dataName, DATATYPE data);
	template<typename DATATYPE>
	bool Get(const std::string& dataName, DATATYPE& data) const;
	template<typename DATATYPE>
	void Set(const std::string& dataName, DATATYPE data);

private:
	friend class Singleton<PersistentDataManager>;
	PersistentDataManager() = default;
	
	std::map<std::string, IBlackBoardData*> m_pPersistentData = {};
};

template<typename DATATYPE>
void PersistentDataManager::Add(const std::string& dataName, DATATYPE data)
{
	auto it{ m_pPersistentData.find(dataName) };
	if (it != std::end(m_pPersistentData))
	{
		const std::wstring tmp{ dataName.begin(), dataName.end() };
		Logger::LogError(L"PersistentDataManager::Add > Data with name " + tmp + L" already exists! ");
		return;
	}

	m_pPersistentData[dataName] = new BlackBoardData<DATATYPE>(data);
}

template<typename DATATYPE>
void PersistentDataManager::Set(const std::string& dataName, DATATYPE data)
{
	auto it{ m_pPersistentData.find(dataName) };
	if (it == std::end(m_pPersistentData))
	{
		Add(dataName, data);
		return;
	}

	BlackBoardData<DATATYPE>* pDataHolder{ dynamic_cast<BlackBoardData<DATATYPE>*>(it->second) };
	if (pDataHolder)
		pDataHolder->m_Data = data;
	else
	{
		const std::wstring tmp{ dataName.begin(), dataName.end() };
		Logger::LogError(L"PersistentDataManager::Set > Data with name " + tmp + L" could not be changed to new value! ");
	}
}

template<typename DATATYPE>
 bool PersistentDataManager::Get(const std::string& dataName, DATATYPE& data) const
{
	auto it{ m_pPersistentData.find(dataName) };
	if (it == std::end(m_pPersistentData))
		return false;

	BlackBoardData<DATATYPE>* pDataHolder{ dynamic_cast<BlackBoardData<DATATYPE>*>(it->second) };
	if (pDataHolder)
		data = pDataHolder->m_Data;
	else
	{
		const std::wstring tmp{ dataName.begin(), dataName.end() };
		Logger::LogError(L"PersistentDataManager::Get > Data with name " + tmp + L" could not be retrieved! ");
		return false;
	}
	
	return true;
}