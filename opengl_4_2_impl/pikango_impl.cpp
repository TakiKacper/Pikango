#define PIKANGO_IMPLEMENTATION
#include "../include/pikango.hpp"

#include <queue>
#include <thread>

#include <iostream>

//static std::queue

void opengl_execution_thread_logic()
{
    std::cout << "HELLO THERE!";
}

static std::thread* opengl_execution_thread;

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

#define PIKANGO_NEW(name)    \
    pikango::name##_handle pikango::new_##name ()

#define PIKANGO_DELETE(name)    \
    void pikango::delete_##name (pikango::name##_handle )

PIKANGO_NEW(vertex_buffer)
{
    return {};
}

PIKANGO_DELETE(vertex_buffer)
{

}

PIKANGO_NEW(data_layout)
{
    return {};
}

PIKANGO_DELETE(data_layout)
{

}

PIKANGO_NEW(graphics_shader)
{
    return {};
}

PIKANGO_DELETE(graphics_shader)
{

}

/*
    Buffers Methods
*/

