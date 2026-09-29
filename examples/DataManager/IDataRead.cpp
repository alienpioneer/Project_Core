#include "core/IDataRead.hpp"

namespace Core
{

int IDataRead::getEquipementId()
{
    return m_equipementId.get();
}

Protocol IDataRead::getCurentProtocol()
{
    return m_currentProtocol.get();
}

Session IDataRead::getCurrentSession()
{
    return m_currentSession.get();
}

bool IDataRead::isMotorRunning()
{
    return m_isMotorRunning.load(std::memory_order_relaxed);
}

}// namespace Core