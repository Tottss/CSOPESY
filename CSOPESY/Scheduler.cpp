#include "Scheduler.h"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <sstream>
#include <cstdio>

namespace Sched
{
    std::string Timestamp()
    {
        std::time_t t = std::time(nullptr);
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif
        char buf[64];
        std::strftime(buf, sizeof(buf), "%m/%d/%Y %I:%M:%S%p", &tm);
        return buf;
    }

    // ---------------- Process ----------------
    Process::Process(std::string n, int total)
        : name(std::move(n)), createdAt(Timestamp()), totalCommands(total)
    {
        if (ENABLE_PRINT_LOGS)
        {
            std::filesystem::create_directories("logs");
            log.open("logs/" + name + ".txt");
            log << "Process name: " << name << "\nLogs:\n\n";
        }
    }

    void Process::ExecutePrint(int coreId)
    {
        if (!ENABLE_PRINT_LOGS) return;
        log << "(" << Timestamp() << ") Core:" << coreId
            << " \"Hello world from " << name << "!\"\n";
    }

    // ---------------- Scheduler ----------------
    Scheduler& Scheduler::Instance()
    {
        static Scheduler s;
        return s;
    }

    void Scheduler::Start(int numProcesses, int commandsPerProcess)
    {
        if (running_) return;

        {
            std::lock_guard<std::mutex> lk(m_);
            for (int i = 1; i <= numProcesses; ++i)
            {
                char nm[32];
                std::snprintf(nm, sizeof(nm), "process%02d", i);
                auto p = std::make_shared<Process>(nm, commandsPerProcess);
                all_.push_back(p);
                ready_.push_back(p);          // FCFS: queue order == creation order
            }
        }

        running_ = true;
        schedThread_ = std::thread(&Scheduler::SchedulerLoop, this);
        for (int c = 0; c < NUM_CORES; ++c)
            workers_.emplace_back(&Scheduler::WorkerLoop, this, c);
    }

    void Scheduler::Stop()
    {
        if (!running_) return;
        {
            std::lock_guard<std::mutex> lk(m_);
            running_ = false;
        }
        cv_.notify_all();
        if (schedThread_.joinable()) schedThread_.join();
        for (auto& w : workers_) if (w.joinable()) w.join();
        workers_.clear();
    }

    bool Scheduler::HasFreeCore() const
    {
        for (int c = 0; c < NUM_CORES; ++c)
            if (!cores_[c].job) return true;
        return false;
    }

    // The scheduler thread: hands the HEAD of the ready queue to any idle core.
    void Scheduler::SchedulerLoop()
    {
        std::unique_lock<std::mutex> lk(m_);
        while (running_)
        {
            cv_.wait(lk, [&] { return !running_ || (!ready_.empty() && HasFreeCore()); });
            if (!running_) break;

            for (int c = 0; c < NUM_CORES && !ready_.empty(); ++c)
            {
                if (!cores_[c].job)
                {
                    cores_[c].job = ready_.front();
                    ready_.pop_front();
                }
            }
            cv_.notify_all();   // wake the workers that just got a job
        }
    }

    // One thread per CPU core: run the assigned process to completion (non-preemptive FCFS).
    void Scheduler::WorkerLoop(int coreId)
    {
        while (true)
        {
            std::shared_ptr<Process> p;
            {
                std::unique_lock<std::mutex> lk(m_);
                cv_.wait(lk, [&] { return !running_ || cores_[coreId].job != nullptr; });
                if (!running_) return;
                p = cores_[coreId].job;
            }

            p->core  = coreId;
            p->state = ProcState::Running;

            while (running_ && p->executed < p->totalCommands)
            {
                p->ExecutePrint(coreId);
                ++p->executed;
                std::this_thread::sleep_for(std::chrono::milliseconds(EXEC_DELAY_MS));
            }

            if (p->executed >= p->totalCommands)
            {
                p->state = ProcState::Finished;
                if (p->log.is_open()) p->log.close();
            }

            {
                std::lock_guard<std::mutex> lk(m_);
                cores_[coreId].job.reset();   // core is free again
            }
            cv_.notify_all();                 // tell the scheduler
        }
    }

    // ---------------- screen -ls ----------------
    std::string Scheduler::StatusReport()
    {
        std::lock_guard<std::mutex> lk(m_);
        std::ostringstream out;
        const char* bar = "----------------------------------------";

        out << bar << "\nRunning processes:\n";
        for (auto& p : all_)
            if (p->state == ProcState::Running)
                out << p->name << "   (" << p->createdAt << ")   Core: " << p->core
                    << "   " << p->executed << " / " << p->totalCommands << "\n";

        out << "\nFinished processes:\n";
        for (auto& p : all_)
            if (p->state == ProcState::Finished)
                out << p->name << "   (" << p->createdAt << ")   Finished   "
                    << p->executed << " / " << p->totalCommands << "\n";

        out << bar << "\n";
        return out.str();
    }
}
