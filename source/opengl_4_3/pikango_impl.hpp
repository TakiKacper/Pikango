#include <mutex>
#include <condition_variable>
#include <thread>

#include <queue>
#include <any>

#include <sstream>

#include "glad/glad.h"
#include "enumerations.hpp"

namespace
{
    using opengl_task = void(*)(std::vector<std::any>);
    using recorded_task = std::pair<opengl_task, std::vector<std::any>>;
    using enqueued_task = recorded_task;
    thread_local pikango::command_buffer_handle recording_command_buffer;
}

#define PIKANGO_IMPL(name)  \
    struct pikango_internal::name##_impl

#define PIKANGO_NEW(name)   \
    pikango::name##_handle pikango::new_##name ()

#include "execution_thread.hpp"
#include "command_buffer.hpp"
#include "fence.hpp"

namespace
{
    void record_task(const opengl_task& task, std::vector<std::any> args)
    {
        auto cbi = pikango_internal::obtain_handle_object(recording_command_buffer);
        cbi->tasks.push_back({task, std::move(args)});
    }

    void enqueue_task(const opengl_task& task, std::vector<std::any> args, pikango::queue_type target_queue_type)
    {
        auto push_task = [&](std::queue<enqueued_task>& queue, std::mutex& mutex, std::condition_variable& condition)
        {
            mutex.lock();
            queue.push({task, std::move(args)});
            mutex.unlock();
        };

        switch (target_queue_type)
        {
        case pikango::queue_type::general:
            push_task(general_queue, general_queue_mutex, general_queue_empty_condition);
            break;

        case pikango::queue_type::compute:
            push_task(compute_queue, compute_queue_mutex, compute_queue_empty_condition);
            break;
            
        case pikango::queue_type::transfer:
            push_task(transfer_queue, transfer_queue_mutex, transfer_queue_empty_condition);
            break;
        }

        execution_thread_sleep_condition.notify_one();
    }
}

void pikango::submit_command_buffer(pikango::command_buffer_handle cb, pikango::queue_type target_queue_type, size_t target_queue_index)
{
    auto cbi = pikango_internal::obtain_handle_object(cb);
    
    std::queue<enqueued_task>*  queue;
    std::mutex*                 mutex;

    switch (target_queue_type)
    {
    case pikango::queue_type::general:
        queue = &general_queue;
        mutex = &general_queue_mutex;
        break;
    case pikango::queue_type::compute:
        queue = &compute_queue;
        mutex = &compute_queue_mutex;
        break;
    case pikango::queue_type::transfer:
        queue = &transfer_queue;
        mutex = &transfer_queue_mutex;
        break;
    };

    mutex->lock();

    for (auto& task : cbi->tasks)
        queue->push(task);

    mutex->unlock();

    execution_thread_sleep_condition.notify_one();
}

void pikango::submit_command_buffer_with_fence(pikango::command_buffer_handle cb, pikango::queue_type target_queue_type, size_t target_queue_index, fence_handle fence)
{
    auto cbi = pikango_internal::obtain_handle_object(cb);
    
    std::queue<enqueued_task>*  queue;
    std::mutex*                 mutex;

    switch (target_queue_type)
    {
    case pikango::queue_type::general:
        queue = &general_queue;
        mutex = &general_queue_mutex;
        break;
    case pikango::queue_type::compute:
        queue = &compute_queue;
        mutex = &compute_queue_mutex;
        break;
    case pikango::queue_type::transfer:
        queue = &transfer_queue;
        mutex = &transfer_queue_mutex;
        break;
    };

    auto func = [](std::vector<std::any> args)
    {
        auto fence = std::any_cast<fence_handle>(args[0]);
        auto fi = pikango_internal::obtain_handle_object(fence);

        fi->is_signaled = true;
        fi->condition.notify_one();
    };

    mutex->lock();

    auto fi = pikango_internal::obtain_handle_object(fence);
    fi->subbmitted = true;
    fi->is_signaled = false;

    for (auto& task : cbi->tasks)
        queue->push(task);

    queue->push({func, {fence}});

    mutex->unlock();

    execution_thread_sleep_condition.notify_one();
}

void pikango::wait_fence(fence_handle target)
{
    auto fi = pikango_internal::obtain_handle_object(target);
    if (fi->subbmitted == false) return;

    std::unique_lock<std::mutex> lock(fi->mutex);
    fi->condition.wait(lock, [&] { return fi->is_signaled || !fi->subbmitted; });
}

void pikango::wait_multiple_fences(std::vector<fence_handle> targets)
{
    for (auto& target : targets)
        wait_fence(target);
}

//Utility
namespace pikango_internal
{
    struct size_t_pair 
    {
        size_t first;
        size_t second;

        bool operator==(const size_t_pair& other) const 
        {
            return first == other.first && second == other.second;
        }

        size_t_pair(size_t _f, size_t _s) : first(_f), second(_s) {};
    };

    struct size_t_pair_hash 
    {
        size_t operator()(const size_t_pair& p) const 
        {
            size_t h1 = std::hash<size_t>{}(p.first);
            size_t h2 = std::hash<size_t>{}(p.second);
            return h1 ^ (h2 << 1);
        }
    };
} 

/*
    Common Opengl Objects
*/

namespace {
    GLuint VAO;
    GLuint FBO;

    GLint textures_pool_size;
    GLint uniforms_pool_size;
    GLint textures_operation_unit;
    GLint max_color_attachments;
}

/*
    Library Implementation
*/

namespace {
    pikango::error_notification_callback error_callback;

    void log_error(const char* text)
    {
        if (error_callback != nullptr)
            error_callback(text);
        else
            abort();
    }

    void GLAPIENTRY gl_log_error
    ( 
        GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar* message,
        const void* userParam
    )
    {
        if (severity < GL_DEBUG_SEVERITY_LOW) return;

        std::stringstream ss;
        
        ss << "\n" << "OPENGL GENERATED ERROR: " << '\n';
        ss << "message: "<< message << '\n';
        ss << "type: ";

        switch (type) 
        {
        case GL_DEBUG_TYPE_ERROR:
            ss << "ERROR";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            ss << "DEPRECATED_BEHAVIOR";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            ss << "UNDEFINED_BEHAVIOR";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            ss << "PORTABILITY";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            ss << "PERFORMANCE";
            break;
        case GL_DEBUG_TYPE_OTHER:
            ss << "OTHER";
            break;
        }

        ss << '\n';
        ss << "id: " << id << '\n';
        
        ss << "severity: ";
        switch (severity)
        {
        case GL_DEBUG_SEVERITY_LOW:
            ss << "LOW";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            ss << "MEDIUM";
            break;
        case GL_DEBUG_SEVERITY_HIGH:
            ss << "HIGH";
            break;
        }
        ss << '\n';

        auto str = ss.str();
        log_error(&str[0]);
    }
}


void pikango::OPENGL_ONLY_execute_on_context_thread(opengl_thread_task task, std::vector<std::any> args)
{
    enqueue_task(task, std::move(args), pikango::queue_type::general);
}

std::string pikango::initialize_library_cpu(const initialize_library_cpu_settings& settings)
{
    error_callback = settings.error_callback;

    start_opengl_execution_thread();
    return "";
}

std::string pikango::initialize_library_gpu()
{
    auto func = [](std::vector<std::any>)
    {
        //create VAO object
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        //create FBO object
        glGenFramebuffers(1, &FBO);

        //get textures pool size
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &textures_pool_size);
        textures_pool_size--;   //Reserve last active texture for writing
        textures_operation_unit = textures_pool_size;
        glActiveTexture(GL_TEXTURE0 + textures_operation_unit);

        //get uniforms pool size
        glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &uniforms_pool_size);

        //get max color attachments
        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max_color_attachments);

        //enable scissors
        glEnable( GL_SCISSOR_TEST);

        //enable error callback
        if (error_callback)
        {
            glEnable(GL_DEBUG_OUTPUT);
            glDebugMessageCallback(gl_log_error, 0);
        }
    };

    enqueue_task(func, {}, pikango::queue_type::general);
    pikango::wait_all_queues_empty();
    return "";
}

void delete_all_program_pipelines();

std::string pikango::terminate()
{
    auto func = [](std::vector<std::any>)
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteFramebuffers(1, &FBO);
        delete_all_program_pipelines();
    };

    enqueue_task(func, {}, pikango::queue_type::general);
    stop_opengl_execution_thread();
    return "";
}

static const char* glsl = "glsl";
const char* pikango::get_used_shading_language_name()
{
    return glsl;
}

/*
    Commands Implementations
*/

#include "graphics_pipeline.hpp"

#include "shader.hpp"
#include "program.hpp"

#include "buffer.hpp"

#include "texture_sampler.hpp"
#include "texture_buffer.hpp"

#include "binding.hpp"
#include "drawing_related.hpp"
#include "drawing.hpp"
