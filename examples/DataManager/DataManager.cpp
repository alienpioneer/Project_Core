#include "core/DataManager.hpp"

namespace Core
{

void DataManager::shutdown()
{
    m_equipementId.shutdown();
    m_currentProtocol.shutdown();
    m_currentSession.shutdown();
};

AData<int> DataManager::m_equipementId{false};

AData<Protocol> DataManager::m_currentProtocol;

AData<Session> DataManager::m_currentSession;

std::atomic<bool> DataManager::m_isMotorRunning{false};

}// namespace Core

