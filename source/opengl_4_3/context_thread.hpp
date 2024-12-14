#include <mutex>
#include <condition_variable>
#include <thread>

#include <queue>
#include <any>

namespace {
    using opengl_task = void(*)(std::vector<std::any>);
    using enqued_task = std::pair<opengl_task, std::vector<std::any>>;
};

static std::condition_variable  opengl_thread_sleep_condition;

static std::mutex               opengl_tasks_queue_mutex;
static std::queue<enqued_task>  opengl_tasks_queue;

static bool should_opengl_thread_terminate = false;

static std::mutex              all_tasks_done_mutex;
static std::condition_variable all_tasks_done_condition;

void enqueue_task(const opengl_task& task, std::vector<std::any> args)
{
    opengl_tasks_queue_mutex.lock();
    opengl_tasks_queue.push({task, std::move(args)});
    opengl_tasks_queue_mutex.unlock();
    opengl_thread_sleep_condition.notify_one();
}

void opengl_execution_thread_logic()
{
    while (true)
    {
        //Wait for tasks
        std::unique_lock<std::mutex> lock(opengl_tasks_queue_mutex);
        opengl_thread_sleep_condition.wait(lock, []() { return !opengl_tasks_queue.empty() || should_opengl_thread_terminate; });

        //If no tasks left and should terminate -> Leave
        if (should_opengl_thread_terminate && opengl_tasks_queue.size() == 0) break;

        //Take task
        auto task = opengl_tasks_queue.front();
        opengl_tasks_queue.pop();
        
        //Unlock the queue for write
        lock.unlock();

        //Execute the task
        task.first(std::move(task.second));

        //Notify about the tasks finish
        if (opengl_tasks_queue.size() == 0)
            all_tasks_done_condition.notify_all();
    }
}

static std::thread* opengl_execution_thread;

void start_opengl_execution_thread()
{
    should_opengl_thread_terminate = false;
    opengl_execution_thread = new std::thread{opengl_execution_thread_logic};
}

void stop_opengl_execution_thread()
{
    //Notify the thread to stop
    should_opengl_thread_terminate = true;
    opengl_thread_sleep_condition.notify_one();

    //Wait for the opengl thread
    opengl_execution_thread->join();

    //Delete the thread
    delete opengl_execution_thread;
}
