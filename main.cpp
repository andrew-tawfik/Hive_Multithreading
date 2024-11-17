#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>

class Hive {
private:
    std::mutex m;                          // Protects access to shared resources (the hive's data)
    std::condition_variable cv;            // Used to synchronize workers (worker bees)
    std::queue<int> qTasks;                // Tasks assigned by the queen (shared task queue)
    std::vector<std::thread> vecWorkers;   // Worker bees (threads)
    bool bShuttingDown = false;            // Signal to stop the hive (shutdown signal)
    int iTotalTasksCompleted = 0;          // Total tasks completed (the queen's broader goal)

public:

    // Constructor: Creates and sets up the hive, assigning iNumberWorkers amount of bees the job of handling upcoming tasks equally.
    Hive(int iNumWorkers)
    {
        // The queen sets up the hive by creating a fixed number of worker bees
        for (int i = 0; i < iNumWorkers; ++i)
        {
            vecWorkers.emplace_back(&Hive::workerTask, this); // Worker threads are created
        }
    }

    // Handle a Bee's Task
    void workerTask()
    {
        while (true)
        {
            int iTask;

            // Bees return to the hive and wait for tasks or instructions from the queen
            {
                std::unique_lock<std::mutex> ulWorkerLock(m); // upon construction automatically locks the mutex

                // Bees wait for tasks or shutdown instructions
                while (!bShuttingDown && qTasks.empty())
                {
                    cv.wait(ulWorkerLock);
                }

                // If the hive is shutting down and there are no tasks, exit
                if (bShuttingDown && qTasks.empty())
                {
                    return; // The bee leaves the hive permanently
                }

                // Fetch a task assigned by the queen
                iTask = qTasks.front();
                qTasks.pop();
            } // The bee leaves the hive to work (mutex is automatically unlocked)

            // Worker bee processes the task
            std::cout << "Worker " << std::this_thread::get_id() << " is processing task " << iTask << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // work delay

            // Worker bee returns to the hive and reports to the queen
            {
                std::unique_lock<std::mutex> ulWorkerLock(m);
                std::cout << "Worker " << std::this_thread::get_id() << " completed the task, notifying the Queen. " << std::endl;
                iTotalTasksCompleted++; // Updates the queen's progress
            } // Mutex is unlocked as the bee awaits further instructions
        }
    }

    // Queen announces the current results of the hive
    void displayProgress()
    {
        // The queen periodically reviews the hive's progress
        std::unique_lock<std::mutex> lock(m);
        std::cout << "Queen: Total tasks completed so far: " << iTotalTasksCompleted  << std::endl;
    }

    // Destructor: Shuts down the hive
    ~Hive()
    {
        {
            std::unique_lock<std::mutex> lock(m);
            bShuttingDown = true; // The queen signals the hive to shut down
        }
        cv.notify_all(); // All bees are instructed to stop working

        for (auto& thrWorkers : vecWorkers)
        {
            if (thrWorkers.joinable())
            {
                thrWorkers.join(); // The queen ensures all bees return before closing the hive
            }
        }
    }

    void assignTask(int task)
    {
        {
            std::unique_lock<std::mutex> lock(m);

            // The queen assigns a task to the hive's task queue
            qTasks.push(task);
        }
        // Notify a worker bee that a task is available
        cv.notify_one();
    }
};

int main()
{
    Hive hive(4); // The queen establishes a hive with 4 worker bees

    // Assign tasks with a delay and display progress after every 5 tasks
    const int totalTasks = 20; // Total number of tasks to accomplish (queen's broader goal)
    for (int i = 1; i <= totalTasks; ++i)
    {
        hive.assignTask(i); // The queen assigns a task
        std::cout << "[Main Thread] Assigned task " << i << std::endl;

        // Simulate the delay between task generation (tasks come in periodically)
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // The queen reviews progress every 5 tasks
        if (i % 5 == 0)
        {
            hive.displayProgress();
        }
        std::cout << std::endl;
    }
    // Allow the worker bees to finish their work
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return 0;
}