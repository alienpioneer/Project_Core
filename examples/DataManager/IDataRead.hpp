#pragma once
#include "DataManager.hpp"

namespace Core
{
    
class IDataRead: public DataManager
{
    public:
        // TODO Add getters
    static int getEquipementId();

    static Protocol getCurentProtocol();

    static Session getCurrentSession();

    static bool isMotorRunning();
};

}// namespace Core

