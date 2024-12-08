#include "glad/glad.h"

#include <mutex>
#include <condition_variable>
#include <thread>

#include <queue>
#include <functional>
#include <any>

#include <iostream>
#include "pikango/pikango.hpp"

namespace {
    using opengl_task = std::function<void(std::vector<std::any>)>;
    using enqued_task = std::pair<opengl_task, std::vector<std::any>>;
};

static std::condition_variable  opengl_thread_sleep_condition;

static std::mutex               opengl_tasks_queue_mutex;
static std::queue<enqued_task>  opengl_tasks_queue;

static bool should_opengl_thread_terminate = false;

void enqueue_task(const opengl_task& task, std::vector<std::any>& args)
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
    }
}

void pikango::OPENGL_ONLY_execute_on_context_thread(opengl_thread_task task, std::vector<std::any> args)
{
    enqueue_task(task, args);
}

static std::thread* opengl_execution_thread;

/*
    Library Implementation
*/

std::string pikango::initialize()
{
    should_opengl_thread_terminate = false;
    opengl_execution_thread = new std::thread{opengl_execution_thread_logic};
    return "";
}

std::string pikango::terminate()
{
    //Notify the thread to stop
    should_opengl_thread_terminate = true;
    opengl_thread_sleep_condition.notify_one();

    //Wait for the opengl thread
    opengl_execution_thread->join();

    //Delete the thread
    delete opengl_execution_thread;
    return "";
}

void pikango::wait_all_tasks_completion()
{
    while (opengl_tasks_queue.size())
        std::this_thread::yield();
}

void pikango::wait_all_current_tasks_completion()
{
    //todo
    //dummy
}

/*
    Contructors / Deconstructors
*/

#define PIKANGO_IMPL(name)  \
    struct pikango_internal::name##_impl

#define PIKANGO_NEW(name)   \
    pikango::name##_handle pikango::new_##name ()

#define PIKANGO_DELETE(name)    \
    void pikango::delete_##name (pikango::name##_handle )

//
// Vertex Buffer
//
PIKANGO_IMPL(vertex_buffer)
{
    GLuint id;
};

PIKANGO_NEW(vertex_buffer)
{
    auto func = [](std::vector<std::any>)
    {
        std::cout << glGetError() << '\n';
    };
    opengl_tasks_queue.push({func, {}});
    return {};
}

PIKANGO_DELETE(vertex_buffer)
{

};


//
// Data Layout
//
PIKANGO_IMPL(data_layout)
{

};

PIKANGO_NEW(data_layout)
{
    return {};
};

PIKANGO_DELETE(data_layout)
{

};


//
// Graphics Shader
//
PIKANGO_IMPL(graphics_shader)
{

};

PIKANGO_NEW(graphics_shader)
{
    return {};
};

PIKANGO_DELETE(graphics_shader)
{

};


//
// Frame Buffer
//
PIKANGO_IMPL(frame_buffer)
{

};

PIKANGO_NEW(frame_buffer)
{
    return {};
};

PIKANGO_DELETE(frame_buffer)
{

};

