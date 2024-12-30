#include <mutex>
#include <condition_variable>
#include <thread>

#include <queue>
#include <any>

//In opengl there is no such concept as queues
//therefore we emulate this behavior by executing the general queue first 
//and the other ones when the general queue is empty
//therefore, since there are no actual queues, and there would be no gain in having multiple fake queues
//pikango only allow for one compute and one tranfer queue in it's opengl implementation
size_t pikango::get_queues_max_amount(pikango::queue_type type)
{
    switch (type)
    {
    case pikango::queue_type::general: return 1;
    case pikango::queue_type::compute: return 1;
    case pikango::queue_type::transfer: return 1;
    }

    //Will never happen
    return 0;
};

void pikango::enable_queues(pikango::queue_type type, size_t amount)
{
    if (amount != 1)                          pikango_internal::log_error("OpenGL implementation only supports one queue per type");
    if (type == pikango::queue_type::general) pikango_internal::log_error("General queue is always enabled");

    //Do nothing
    //Theres no queues to be created
}

/*
    Execution
*/

namespace {
    std::mutex                  execution_thread_sleep_mutex;
    std::condition_variable     execution_thread_sleep_condition;

    std::mutex                  general_queue_mutex;
    std::queue<enqueued_task>   general_queue;
    std::condition_variable     general_queue_empty_condition;

    std::mutex                  compute_queue_mutex;
    std::queue<enqueued_task>   compute_queue;
    std::condition_variable     compute_queue_empty_condition;

    std::mutex                  transfer_queue_mutex;
    std::queue<enqueued_task>   transfer_queue;
    std::condition_variable     transfer_queue_empty_condition;

    bool should_execution_thread_terminate = false;

    std::mutex                  all_tasks_done_mutex;
    std::condition_variable     all_tasks_done_condition;
};

static bool all_queues_empty()
{
    return !(general_queue.size() + compute_queue.size() + transfer_queue.size());
}

static void opengl_execution_thread_logic()
{
    while (true)
    {
        //Wait for tasks
        std::unique_lock<std::mutex> lock(execution_thread_sleep_mutex);
        execution_thread_sleep_condition.wait(lock, []() { return !all_queues_empty() || should_execution_thread_terminate; });

        //If no tasks left and should terminate -> Leave
        if (should_execution_thread_terminate && all_queues_empty() == 0) break;

        //Take task
        enqueued_task task;

        auto take_task_from_queue = [&](std::queue<enqueued_task>& queue, std::mutex& mutex, std::condition_variable& condition)
        {
            mutex.lock();
            task = queue.front();
            queue.pop();
            mutex.unlock();

            if (queue.size() == 0) 
                condition.notify_one();
        };
        
        if (general_queue.size() != 0)
            take_task_from_queue(general_queue, general_queue_mutex, general_queue_empty_condition);

        else if (compute_queue.size() != 0)
            take_task_from_queue(compute_queue, compute_queue_mutex, compute_queue_empty_condition);

        else if (transfer_queue.size() != 0)
            take_task_from_queue(transfer_queue, transfer_queue_mutex, transfer_queue_empty_condition);
        
        //Unlock the sleep mutex
        execution_thread_sleep_mutex.unlock();

        //Execute the task
        task.first(std::move(task.second));

        //Notify about the tasks completion
        if (all_queues_empty())
            all_tasks_done_condition.notify_all();
    }
}

static std::thread* opengl_execution_thread;

static void start_opengl_execution_thread()
{
    should_execution_thread_terminate = false;
    opengl_execution_thread = new std::thread{opengl_execution_thread_logic};
}

static void stop_opengl_execution_thread()
{
    //Notify the thread to stop
    should_execution_thread_terminate = true;
    execution_thread_sleep_condition.notify_one();

    //Wait for the opengl thread
    opengl_execution_thread->join();

    //Delete the thread
    delete opengl_execution_thread;
}

/*
    Waiting
*/

void pikango::wait_queue_empty(queue_type type, size_t queue_index)
{
    std::queue<enqueued_task>*  queue;
    std::mutex*                 mutex;
    std::condition_variable*    condition;

    switch (type)
    {
    case pikango::queue_type::general:
        queue = &general_queue;
        mutex = &general_queue_mutex;
        condition = &general_queue_empty_condition;
        break;
    case pikango::queue_type::compute:
        queue = &compute_queue;
        mutex = &compute_queue_mutex;
        condition = &compute_queue_empty_condition;
        break;
    case pikango::queue_type::transfer:
        queue = &transfer_queue;
        mutex = &transfer_queue_mutex;
        condition = &transfer_queue_empty_condition;
        break;
    };

    std::unique_lock lock(*mutex);
    condition->wait(lock, [&]{ return queue->size() == 0; });
}

void pikango::wait_all_queues_empty()
{
    if (all_queues_empty()) return;
    
    std::unique_lock lock(all_tasks_done_mutex);
    all_tasks_done_condition.wait(lock, []{ return all_queues_empty(); });
}
