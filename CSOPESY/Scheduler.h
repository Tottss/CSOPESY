#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace Sched
{
    constexpr int  NUM_CORES         = 4;
    constexpr int  EXEC_DELAY_MS     = 100;  // time one print command "takes" on a core
    // Homework only: one log file per process. SET TO false FOR THE MACHINE PROJECT.
    constexpr bool ENABLE_PRINT_LOGS = true;

    enum class ProcState { Ready, Running, Finished };

    struct Process
    {
        std::string          name;
        std::string          createdAt;
        int                  totalCommands;
        std::atomic<int>     executed{0};
        std::atomic<int>     core{-1};
        std::atomic<ProcState> state{ProcState::Ready};
        std::ofstream        log;

        Process(std::string n, int total);
        void ExecutePrint(int coreId);   // one "print" command
    };

    class Scheduler
    {
    public:
        static Scheduler& Instance();

        // Creates N processes (in order) and starts 1 scheduler thread + NUM_CORES worker threads.
        void Start(int numProcesses, int commandsPerProcess);
        void Stop();

        // Text for the "screen -ls" command.
        std::string StatusReport();

    private:
        Scheduler() = default;
        void SchedulerLoop();
        void WorkerLoop(int coreId);
        bool HasFreeCore() const;

        struct CoreSlot { std::shared_ptr<Process> job; };

        std::mutex                            m_;
        std::condition_variable               cv_;
        std::atomic<bool>                     running_{false};
        std::deque<std::shared_ptr<Process>>  ready_;     // FCFS queue
        std::vector<std::shared_ptr<Process>> all_;       // creation order, for reporting
        CoreSlot                              cores_[NUM_CORES];
        std::thread                           schedThread_;
        std::vector<std::thread>              workers_;
    };

    std::string Timestamp();   // (MM/DD/YYYY HH:MM:SSAM)
}
