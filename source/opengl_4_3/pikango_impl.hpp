#include "glad/glad.h"

#include <queue>
#include <thread>
#include <functional>
#include <any>

#include <iostream>
#include "pikango/pikango.hpp"

namespace {
    using opengl_task = std::function<void(std::vector<std::any>)>;
};
static std::queue<opengl_task> opengl_tasks_queue;
static bool should_execution_thread_shutdown = false;

//Not thread safe and bad in general
//Todo
void opengl_execution_thread_logic()
{
    if (gladLoadGL() != 0) 
        return;

    while (!should_execution_thread_shutdown || opengl_tasks_queue.size())
    {
        while (!should_execution_thread_shutdown)
        {
            if (opengl_tasks_queue.size() == 0) std::this_thread::yield();
            else break;
        }

        if (should_execution_thread_shutdown && opengl_tasks_queue.size() == 0) break;

        auto task = opengl_tasks_queue.front();
        opengl_tasks_queue.pop();

        task({});
    }
}

static std::thread* opengl_execution_thread;

/*
    Library Implementation
*/

std::string pikango::initialize()
{
    opengl_execution_thread = new std::thread{opengl_execution_thread_logic};
    return "";
}

std::string pikango::terminate()
{
    opengl_execution_thread->join();
    delete opengl_execution_thread;
    return "";
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
        glClear(GL_COLOR_BUFFER_BIT);
    };
    opengl_tasks_queue.push(func);
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

