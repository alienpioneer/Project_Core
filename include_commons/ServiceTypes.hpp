/**
 * @file ServiceTypes.hpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#pragma once
#include <cstdint>

/**
 * @page ServiceTypesPage Service Types
 * @brief Definition and extension rules for ServiceId.
 *
 * ## Overview
 * `ServiceId` uniquely identifies each service.
 *
 * It is used for:
 * - Service indexing in ServiceManager
 * - Event metadata (sender identification)
 *
 * ## Constraints (MANDATORY)
 * - Must be:
 *   - Zero-based
 *   - Contiguous
 *   - Without explicit numeric assignments
 * - `COUNT` must always be last
 * - Do not reorder existing values
 *
 * ## Extension
 * The application must copy and extend this enum.
 *
 * ### Example
 * @code{.cpp}
 * enum class ServiceId : uint8_t
 * {
 *     BASE = 0,
 *     ServiceManager,
 *
 *     WifiService,
 *     AudioService,
 *
 *     COUNT
 * };
 * @endcode
 *
 * ## Limits
 * Must respect:
 * @code{.cpp}
 * static_assert(
 *     static_cast<std::size_t>(ServiceId::COUNT) <= Core::MAX_SERVICES);
 * @endcode
 *
 * ## Notes
 * - Used as array index → must remain dense
 * - Not used for routing logic (EventType is)
 */

namespace Core
{

enum class ServiceId : uint8_t 
{
    BASE = 0,
    ServiceManager = 1,
    // For tests only
    TestService = 2,

    //ADD services here

    // Do NOT remove, DO NOT USE, add services above COUNT 
    COUNT  // number of services
};

}// namespace Core
