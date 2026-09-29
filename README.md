# Core Architecture

The project is built around a small event-driven service architecture designed for embedded Linux. The core separates event routing, service execution, hardware access, configuration/data handling, and common utilities.

## Architecture Overview

```text
                    +------------------+
                    |   EventManager   |
                    |------------------|
                    | Event queue      |
                    | Subscribers      |
                    | Dispatcher thread|
                    +--------+---------+
                             |
                    Events / subscriptions
                             |
          +------------------+------------------+
          |                  |                  |
+---------v---------+ +------v-------+ +--------v--------+
|   ServiceBase     | |   Service    | |     Service     |
|     Queue         | |              | |                 |
+-------------------+ +--------------+ +-----------------+
          |                  |
          |                  |
          v                  v
    Service thread      Hardware / HAL
                             |
                    +--------+--------+
                    | CAN / other HW  |
                    +-----------------+
```

## Event System

EventManager is the central asynchronous event bus.

* Services subscribe to specific EventType values.
* Events are posted into a thread-safe queue.
* A dedicated dispatcher thread removes events from the queue.
* The dispatcher takes a snapshot of the subscribers and calls `onEvent()`.
* Subscribers are stored as `weak_ptr`, so the event manager does not own services.
* Subscription access is protected by a mutex.

This keeps event producers and consumers decoupled.

## ServiceBaseQueue

ServiceBaseQueue is the base class for active services.

Each service has:
* Its own service ID.
* A private event queue.
* A worker execution loop.
* A configurable maximum number of events processed per cycle.
* Thread-safe event reception.
* `start()` / `stop()` lifecycle control.
* `setup()` and `update()` hooks.
* A `dispatch()` function implemented by derived services.

The important distinction is:

```text
EventManager thread
        |
        | onEvent()
        v
ServiceBaseQueue event queue
        |
        | dispatch()
        v
Service worker thread
```

The EventManager therefore does not execute service logic directly. It only delivers events to the service's queue.

## Concrete Services

A service derives from ServiceBaseQueue and implements its domain-specific behavior.

For example, `CanService`:
* Subscribes to CAN-related events.
* Receives commands through its service queue.
* Communicates with the CAN HAL.
* Decodes received CAN frames.
* Converts hardware data into application-level values.
* Publishes response events through EventManager.

This keeps the service responsible for domain logic while hardware-specific operations remain in the HAL layer.

## HAL Separation

Hardware access is kept below the service layer.

For example:

```text
CanService
    |
    +-- CANSocket
    +-- CanDriver
    +-- CanCodecs
```

`CanService` handles CAN application logic, while socket and interface operations are delegated to HAL components.

This allows the service architecture to remain independent from low-level Linux device operations.

## Core Utilities

The core also provides reusable infrastructure components:
* `ATimer` — thread-safe timer/callback mechanism.
* `AQueue` — fixed-capacity queue used by event infrastructure.
* `AVector` — fixed-capacity vector.
* `AMap` — fixed-capacity associative container.
* `AFile` — move-only in-memory file representation.
* `FileLoader` — file and directory loading utilities.

The fixed-capacity containers are particularly suited to the embedded environment because their storage limits are known at compile time.

## Configuration and Data

Services obtain configuration through dedicated data/configuration components rather than embedding configuration handling into the service infrastructure.

For example, `CanService` obtains:
* CAN interface name.
* CAN bitrate.
* Safe fallback values.
* CAN message codec configuration.

The service therefore consumes configuration but does not own the general configuration mechanism.

## Threading Model

The architecture uses explicit worker threads rather than a global thread pool.

At a high level:

```text
                  EventManager
                  dispatcher
                      thread
                         |
              +----------+----------+
              |          |          |
              v          v          v
           Service A  Service B  Service C
           thread     thread     thread
```

EventManager serializes event distribution, while each service processes its own events independently in its service thread.

Synchronization is provided through:
* `std::mutex`
* `std::condition_variable`
* `std::atomic`
* thread-safe queues

## Design Principles

The core architecture is based on:
* Event-driven communication instead of direct service-to-service coupling.
* Dedicated service queues to isolate service processing.
* Dedicated service threads for active components.
* HAL separation between application/domain logic and hardware access.
* RAII-based lifetime management using smart pointers and automatic thread cleanup.
* Fixed-capacity containers where predictable memory usage is important.
* Asynchronous communication between producers, the event manager, and services.

The result is a modular embedded architecture in which new services can be added by deriving from ServiceBaseQueue, subscribing to the required events, implementing `dispatch()`, and encapsulating their hardware/domain-specific processing.

Please consult the doxygen documentation for specifics. (Must be generated first)