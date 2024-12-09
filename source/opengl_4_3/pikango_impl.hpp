#include "glad/glad.h"

#include <mutex>
#include <condition_variable>
#include <thread>

#include <queue>
#include <functional>
#include <any>

#include "enumerations.hpp"

//delete
#include "pikango/pikango.hpp"
#include <iostream>

namespace {
    using opengl_task = std::function<void(std::vector<std::any>)>;
    using enqued_task = std::pair<opengl_task, std::vector<std::any>>;
};

static std::condition_variable  opengl_thread_sleep_condition;

static std::mutex               opengl_tasks_queue_mutex;
static std::queue<enqued_task>  opengl_tasks_queue;

static bool should_opengl_thread_terminate = false;

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
    }
}

void pikango::OPENGL_ONLY_execute_on_context_thread(opengl_thread_task task, std::vector<std::any> args)
{
    enqueue_task(task, std::move(args));
}

static std::thread* opengl_execution_thread;

/*
    Common Opengl Objects
*/
static GLuint VAO;

/*
    Library Implementation
*/

std::string pikango::initialize_library_cpu()
{
    should_opengl_thread_terminate = false;
    opengl_execution_thread = new std::thread{opengl_execution_thread_logic};
    return "";
}

std::string pikango::initialize_library_gpu()
{
    auto func = [](std::vector<std::any>)
    {
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
    };

    enqueue_task(func, {});
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
    void pikango::delete_##name (pikango::name##_handle handle)

//
// Vertex Buffer
//
PIKANGO_IMPL(vertex_buffer)
{
    GLuint id = 0;
    size_t buffer_size = 0;
    pikango::buffer_memory_profile memory_profile;
    pikango::buffer_access_profile access_profile;
};

PIKANGO_NEW(vertex_buffer)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::vertex_buffer_impl);
    auto vbi = pikango_internal::object_write_access(handle);
    vbi->buffer_size = 0;
    return handle;
}

PIKANGO_DELETE(vertex_buffer)
{ 

};


//
// Data Layout
//
PIKANGO_IMPL(data_layout)
{
    std::vector<pikango::data_types> layout{};
    size_t layouts_stride = 0;
};

PIKANGO_NEW(data_layout)
{
    return pikango_internal::make_handle(new pikango_internal::data_layout_impl);
};

PIKANGO_DELETE(data_layout)
{

};


//
// Graphics Shader
//
PIKANGO_IMPL(graphics_shader)
{
    GLuint id;
};

PIKANGO_NEW(graphics_shader)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::graphics_shader_impl);

    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<pikango::graphics_shader_handle>(args.front());
        auto gsi = pikango_internal::object_write_access(handle);

        const char *vertexShaderSource = "#version 330 core\n"
            "layout (location = 0) in vec2 aPos;\n"
            "void main()\n"
            "{\n"
            "   gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
            "}\0";
        const char *fragmentShaderSource = "#version 330 core\n"
            "out vec4 FragColor;\n"
            "void main()\n"
            "{\n"
            "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
            "}\n\0";

        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        // check for shader compile errors
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        // fragment shader
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        // check for shader compile errors
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        // link shaders
        unsigned int shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        // check for linking errors
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        gsi->id = shaderProgram;
    };
    
    enqueue_task(func, {handle});
    return handle;
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


/*
    Buffers Methods
*/

size_t pikango::get_buffer_size(vertex_buffer_handle target)
{
    auto vbi = pikango_internal::object_read_access(target);
    return vbi->buffer_size;
}

void pikango::assign_buffer_memory(
    vertex_buffer_handle target,
    size_t memory_block_size_bytes, 
    buffer_memory_profile memory_profile, 
    buffer_access_profile access_profile
)
{
    auto vbi = pikango_internal::object_write_access(target);

    vbi->buffer_size    = memory_block_size_bytes;
    vbi->memory_profile = memory_profile;
    vbi->access_profile = access_profile;

    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<pikango::vertex_buffer_handle>(args[0]);
        auto size   = std::any_cast<size_t>(args[1]);
        auto memory = std::any_cast<buffer_memory_profile>(args[2]);
        auto access = std::any_cast<buffer_access_profile>(args[3]);

        auto vbi = pikango_internal::object_write_access(handle);
        if (vbi->id == 0)
            glGenBuffers(1, &vbi->id);

        //if (size == 0)
        //todo error

        glBindBuffer(GL_COPY_WRITE_BUFFER, vbi->id);
        glBufferData(GL_COPY_WRITE_BUFFER, size, nullptr, get_buffer_usage_flag(memory, access));   
    };
    enqueue_task(func, {target, memory_block_size_bytes, memory_profile, access_profile});
}

void pikango::write_buffer(vertex_buffer_handle target, size_t data_size_bytes, void* data)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<pikango::vertex_buffer_handle>(args[0]);
        auto size = std::any_cast<size_t>(args[1]);
        auto data = std::any_cast<void*>(args[2]);

        auto vbi = pikango_internal::object_read_access(handle);
        
        //if (vbi->id == 0)
        //todo error
        
        //if (size > vbi->buffer_size)
        //todo error

        //if (data == nullptr)
        //todo error

        glBindBuffer(GL_COPY_WRITE_BUFFER, vbi->id);
        glBufferSubData(GL_COPY_WRITE_BUFFER, 0, size, data);
    };
    
    enqueue_task(func, {target, data_size_bytes, data});
}

void pikango::write_buffer_region(vertex_buffer_handle target, size_t data_size_bytes, void* data, size_t data_offset_bytes)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<pikango::vertex_buffer_handle>(args[0]);
        auto size = std::any_cast<size_t>(args[1]);
        auto data = std::any_cast<void*>(args[2]);
        auto offset = std::any_cast<size_t>(args[3]);

        auto vbi = pikango_internal::object_read_access(handle);
        
        //if (vbi->id == 0)
        //todo error
        
        //if (size + offset > vbi->buffer_size)
        //todo error

        //if (data == nullptr)
        //todo error

        glBindBuffer(GL_COPY_WRITE_BUFFER, vbi->id);
        glBufferSubData(GL_COPY_WRITE_BUFFER, offset, size, data);
    };
    
    enqueue_task(func, {target, data_size_bytes, data, data_offset_bytes});
}



/*
    Data Layout
*/

size_t pikango::get_layout_size(data_layout_handle target)
{
    auto dli = pikango_internal::object_read_access(target);

    size_t size = 0;
    for (auto& e : dli->layout)
        size += size_of(e);
    
    return size;
}

size_t pikango::get_layout_size_with_stride(data_layout_handle target)
{
    auto dli = pikango_internal::object_read_access(target);

    size_t size = 0;
    for (auto& e : dli->layout)
        size += size_of(e);

    size += dli->layouts_stride;
    
    return size;
}

void pikango::assign_data_layout(data_layout_handle target, std::vector<data_types> layout, size_t layouts_stride)
{
    auto dli = pikango_internal::object_write_access(target);
    dli->layout = std::move(layout);
    dli->layouts_stride = layouts_stride;
}

/*
    Shaders Methods
*/

void pikango::link_vertex_shader  (graphics_shader_handle target, const std::string& shader);
void pikango::link_geometry_shader(graphics_shader_handle target, const std::string& shader);
void pikango::link_pixel_shader   (graphics_shader_handle target, const std::string& shader);

/*
    Drawing
*/

void pikango::draw_vertices(draw_vertices_args& args)
{
    auto func = [](std::vector<std::any> args)
    {
        auto draw_args = std::any_cast<pikango::draw_vertices_args>(args.front());

        glBindBuffer(GL_ARRAY_BUFFER, pikango_internal::object_read_access(draw_args.vertex_buffer)->id);

        //push data layout
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glUseProgram(pikango_internal::object_read_access(draw_args.graphics_shader)->id);

        glDrawArrays(GL_TRIANGLES, 0, 3);
    };

    enqueue_task(func, {args});
}
