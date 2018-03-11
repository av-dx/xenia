/**
******************************************************************************
* Xenia : Xbox 360 Emulator Research Project                                 *
******************************************************************************
* Copyright 2018 Ben Vanik. All rights reserved.                             *
* Released under the BSD license - see LICENSE in the root for more details. *
******************************************************************************
*/

#include "xenia/base/threading.h"

#include "third_party/catch/include/catch.hpp"

namespace xe {
namespace base {
namespace test {
using namespace threading;
using namespace std::chrono_literals;

TEST_CASE("Fence") {
  std::unique_ptr<threading::Fence> pFence;
  std::unique_ptr<threading::HighResolutionTimer> pTimer;

  // Signal without wait
  pFence = std::make_unique<threading::Fence>();
  pFence->Signal();

  // Signal once and wait
  pFence = std::make_unique<threading::Fence>();
  pFence->Signal();
  pFence->Wait();

  // Signal twice and wait
  pFence = std::make_unique<threading::Fence>();
  pFence->Signal();
  pFence->Signal();
  pFence->Wait();

  // Test to synchronize multiple threads
  std::atomic<int> started(0);
  std::atomic<int> finished(0);
  pFence = std::make_unique<threading::Fence>();
  auto func = [&pFence, &started, &finished] {
    started.fetch_add(1);
    pFence->Wait();
    finished.fetch_add(1);
  };

  auto threads = std::array<std::thread, 5>({
      std::thread(func),
      std::thread(func),
      std::thread(func),
      std::thread(func),
      std::thread(func),
  });

  Sleep(100ms);
  REQUIRE(finished.load() == 0);

  // TODO(bwrsandman): Check if this is correct behaviour: looping with Sleep
  // is the only way to get fence to signal all threads on windows
  for (int i = 0; i < threads.size(); ++i) {
    Sleep(10ms);
    pFence->Signal();
  }
  REQUIRE(started.load() == threads.size());

  for (auto& t : threads) t.join();
  REQUIRE(finished.load() == threads.size());
}  // namespace test

TEST_CASE("Get number of logical processors") {
  auto count = std::thread::hardware_concurrency();
  REQUIRE(logical_processor_count() == count);
  REQUIRE(logical_processor_count() == count);
  REQUIRE(logical_processor_count() == count);
}

TEST_CASE("Enable process to set thread affinity") {
  EnableAffinityConfiguration();
}

TEST_CASE("Yield Current Thread", "MaybeYield") {
  // Run to see if there are any errors
  MaybeYield();
}

TEST_CASE("Sync with Memory Barrier", "SyncMemory") {
  // Run to see if there are any errors
  SyncMemory();
}

TEST_CASE("Sleep Current Thread", "Sleep") {
  auto wait_time = 50ms;
  auto start = std::chrono::steady_clock::now();
  Sleep(wait_time);
  auto duration = std::chrono::steady_clock::now() - start;
  REQUIRE(duration >= wait_time);
}

TEST_CASE("Sleep Current Thread in Alertable State", "Sleep") {
  // TODO(bwrsandman):
  REQUIRE(true);
}

TEST_CASE("TlsHandle") {
  // TODO(bwrsandman):
  REQUIRE(true);
}

TEST_CASE("HighResolutionTimer") {
  // The wait time is 500ms with an interval of 50ms
  // Smaller values are not as precise and fail the test
  const auto wait_time = 500ms;

  // Time the actual sleep duration
  {
    const auto interval = 50ms;
    std::atomic<uint64_t> counter;
    auto start = std::chrono::steady_clock::now();
    auto cb = [&counter] { ++counter; };
    auto pTimer = HighResolutionTimer::CreateRepeating(interval, cb);
    Sleep(wait_time);
    pTimer.reset();
    auto duration = std::chrono::steady_clock::now() - start;

    // Should have run as many times as wait_time / timer_interval plus or
    // minus 1 due to imprecision of Sleep
    REQUIRE(duration.count() >= wait_time.count());
    auto ratio = static_cast<uint64_t>(duration / interval);
    REQUIRE(counter >= ratio - 1);
    REQUIRE(counter <= ratio + 1);
  }

  // Test concurrent timers
  {
    const auto interval1 = 100ms;
    const auto interval2 = 200ms;
    std::atomic<uint64_t> counter1;
    std::atomic<uint64_t> counter2;
    auto start = std::chrono::steady_clock::now();
    auto cb1 = [&counter1] { ++counter1; };
    auto cb2 = [&counter2] { ++counter2; };
    auto pTimer1 = HighResolutionTimer::CreateRepeating(interval1, cb1);
    auto pTimer2 = HighResolutionTimer::CreateRepeating(interval2, cb2);
    Sleep(wait_time);
    pTimer1.reset();
    pTimer2.reset();
    auto duration = std::chrono::steady_clock::now() - start;

    // Should have run as many times as wait_time / timer_interval plus or
    // minus 1 due to imprecision of Sleep
    REQUIRE(duration.count() >= wait_time.count());
    auto ratio1 = static_cast<uint64_t>(duration / interval1);
    auto ratio2 = static_cast<uint64_t>(duration / interval2);
    REQUIRE(counter1 >= ratio1 - 1);
    REQUIRE(counter1 <= ratio1 + 1);
    REQUIRE(counter2 >= ratio2 - 1);
    REQUIRE(counter2 <= ratio2 + 1);
  }

  // TODO(bwrsandman): Check on which thread callbacks are executed when
  // spawned from differing threads
}

TEST_CASE("Wait on Multiple Handles", "Wait") {
  // TODO(bwrsandman):
  REQUIRE(true);
}

TEST_CASE("Signal and Wait") {
  // TODO(bwrsandman): Test semaphore, mutex and event
  REQUIRE(true);
}

TEST_CASE("Wait on Event", "Event") {
  // TODO(bwrsandman):
  REQUIRE(true);
}

TEST_CASE("Wait on Semaphore", "Semaphore") {
  // TODO(bwrsandman):
  REQUIRE(true);
}

TEST_CASE("Wait on Mutant", "Mutant") {
  // TODO(bwrsandman):
  REQUIRE(true);
}

TEST_CASE("Create and Trigger Timer", "Timer") {
  // TODO(bwrsandman):
  REQUIRE(true);
}

TEST_CASE("Set and Test Current Thread ID", "Thread") {
  // System ID
  auto system_id = current_thread_system_id();
  REQUIRE(system_id > 0);

  // Thread ID
  auto thread_id = current_thread_id();
  REQUIRE(thread_id == system_id);

  // Set a new thread id
  const uint32_t new_thread_id = 0xDEADBEEF;
  set_current_thread_id(new_thread_id);
  REQUIRE(current_thread_id() == new_thread_id);

  // Set back original thread id of system
  set_current_thread_id(std::numeric_limits<uint32_t>::max());
  REQUIRE(current_thread_id() == system_id);

  // TODO(bwrsandman): Test on Thread object
}

TEST_CASE("Set and Test Current Thread Name", "Thread") {
  std::string new_thread_name = "Threading Test";
  set_name(new_thread_name);
}

TEST_CASE("Create and Run Thread", "Thread") {
  // TODO(bwrsandman):
  REQUIRE(true);
}

}  // namespace test
}  // namespace base
}  // namespace xe