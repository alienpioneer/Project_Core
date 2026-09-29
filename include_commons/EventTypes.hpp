/**
 * @file EventTypes.hpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#pragma once

/**
 * @page EventTypesPage Event Types
 * @brief Definition and extension rules for EventType.
 *
 * ## Overview
 * `EventType` defines all events used in the system.
 * It is used as:
 * - Routing key in EventManager
 * - Index for internal subscriber storage
 *
 * ## Constraints (MANDATORY)
 * - Must be:
 *   - Zero-based
 *   - Contiguous
 *   - Without explicit numeric assignments
 * - `EVENT_COUNT` must always be last
 * - Do not reorder existing values
 * 
 * DO NOT POLUTE with a lot of events like for example:
 * EventType::PlayClickSound
 * EventType::PlayErrorSound
 * EventType::PlayStartupSound
 * 
 * Use for example EventType::AudioPlay and attach a payload describing what to play.
 * 
 * Use commmon enums like:
 * 
 * enum class ActionId : uint8_t
 * {
 *     Click,
 *     Error,
 *     Startup,
 *     Notification,
 *     Count
 * };
 * 
 * to syncronize events/payloads
 *
 * ## Extension
 * The application must copy and extend this enum.
 *
 * ### Example
 * @code{.cpp}
 * enum class EventType : int
 * {
 *     DEFAULT = 0,
 *
 *     WifiConnected,
 *     AudioPlay,
 *
 *     SHUTDOWN,
 *     EVENT_COUNT
 * };
 * @endcode
 *
 * ## Limits
 * Must respect:
 * @code{.cpp}
 * static_assert(
 *     static_cast<std::size_t>(EventType::EVENT_COUNT) <= Core::MAX_EVENT_TYPES);
 * @endcode
 *
 * ## Notes
 * - Used for array indexing → must remain dense
 * - Sparse values will break the system
 */

namespace Core
{

enum class EventType : int
{
    // Do not remove, add events bellow
    DEFAULT = 0,
    // Do NOT remove, used for shutdown
    SHUTDOWN = 1,
    // For tests only, add events below !
    TestEvent = 2,

    // ADD Events here

    // Do NOT remove, DO NOT USE !!!
    EVENT_COUNT
};


}// namespace Core