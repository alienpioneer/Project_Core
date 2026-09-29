#pragma once
#include <atomic>
#include "AData.hpp"

/*
All the variables, parameters,etc are stored as static members.
To read, use only the interface IDataRead
To write, use speciliwed interfaces reserved for each service, ex: IDataWriteMotorService
*/

namespace Core
{

// TODO remove - For test only
class Session
{
    Session()=default;
};

class Protocol
{
    Protocol()=default;
};

enum class DataReadStatus
{
    OK = 0,
    NOK,
    DataNotFound,
    InternalError,
    Empty,
    FileError,
    XmlError,
    CsvError
};

class DataManager
{
public:
    DataManager() = default;
    virtual ~DataManager() = default;

    void shutdown();

protected:

    static AData<int> m_equipementId;

    static AData<Protocol>  m_currentProtocol;

    static AData<Session> m_currentSession;

    static std::atomic<bool> m_isMotorRunning;
};

} // namespace DataManager

