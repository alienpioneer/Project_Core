/**
 * @file AData.hpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#pragma once

#include <mutex>
#include <type_traits>
#include <utility>


/**
 * @brief Thread safe Data class, used to store data in DataManager
 */

namespace Core
{
   
template<typename T>
class AData
{
   public:
      using value_type = T;

   public:
      AData()
        : data_{}
      {}

      explicit AData(const T& value)
        : data_{value}
      {}

      explicit AData(T&& value)
        : data_{std::move(value)}
      {}

      AData(const AData& obj) = delete;
      AData& operator = (const AData& obj) = delete;
      AData& operator = (AData&&      obj) = delete;

      virtual ~AData() =default;

      /**
       * @brief Manager shutdown - no data can be set after shoutdown
       * 
       */
      void shutdown()
      {
         std::lock_guard<std::mutex> lock(mtx_);
         isShuttingDown_ = true;
      }

      /**
       * @brief Check if the manager is shut down or shutting down
       * 
       * @return value 
       */
      bool isShutdown() const
      {
         std::lock_guard<std::mutex> lock(mtx_);
         return isShuttingDown_;
      }

      /**
       * @brief Get underlying data. 
       * The class should only return a data copy, not reference! 
       * Otherwise the data can be changed during reading and sometimes we get UB
       * 
       * @return T 
       */
      T get()
      {
         std::lock_guard<std::mutex> lock(mtx_);
         return data_;
      }

      /**
       * @brief Set undelying data
       * 
       * @param value 
       */
      void set(const T& value)
      {
         std::lock_guard<std::mutex> lock(mtx_);
         if (isShuttingDown_) 
         {
            return;
         }
         data_ = value;
      }

      /**
       * @brief Set rvalue data
       * 
       * @param value 
       */
      void set(T&& value)
      {
         std::lock_guard<std::mutex> lock(mtx_);
         if (isShuttingDown_)
         {
            return;
         } 
         data_ = std::move(value);
      }

      /**
       * @brief Thread safe transformation "op" on the data
       * 
       * @tparam Fn - function object
       * @param op 
       * 
       * @warning The "op" operation should not make any type of synchronization (lock of mutex, sleep, wait, etc )
       */
      template<class Fn>
      void transform(Fn op)
      {
         std::lock_guard<std::mutex> lock(mtx_);
         if (isShuttingDown_)
         {
            return;
         } 
         op(data_);
      }

      /**
       * @brief Read the data unsing an operator object
       * 
       * @tparam Fn - function object
       * @param fn 
       */
      template<class Fn>
      void read(Fn fn) const
      {
         std::lock_guard<std::mutex> lock(mtx_);
         fn(data_);
      }

      /**
       * @brief Thread safe swap: set new value, return the old value
       * 
       * @param value 
       * @return T the value held previously
       */
      T exchange(T value)
      {
         std::lock_guard<std::mutex> lock(mtx_);

         if (!isShuttingDown_)
         {
            std::swap(data_, value);
         }

         return value;
      }

   private:
      mutable std::mutex  mtx_;
      T data_;
      bool isShuttingDown_{false};
};

} // namespace Core