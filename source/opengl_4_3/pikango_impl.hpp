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
    using opengl_task = void(*)(std::vector<std::any>&);
    using recorded_task = std::pair<opengl_task, std::vector<std::any>>;
    using enqueued_task = recorded_task;
    thread_local pikango::command_buffer_handle recorded_command_buffer;
}

#include "execution_thread.hpp"
#include "command_buffer.hpp"
#include "fence.hpp"

namespace
{
    void record_task(const opengl_task& task, std::vector<std::any>&& args)
    {
        auto cbi = pikango_internal::obtain_handle_object(recorded_command_buffer);
        cbi->tasks.push_back({task, std::move(args)});
    }

    void enqueue_task(const opengl_task& task, std::vector<std::any>&& args, pikango::queue_type target_queue_type)
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

    void enqueue_task_and_wait(const opengl_task& task, std::vector<std::any>&& args, pikango::queue_type target_queue_type)
    {
        std::mutex              mutex;
        std::condition_variable condition;
        bool flag = false;

        auto wrapper = [](std::vector<std::any>& args)
        {
            auto task = std::any_cast<opengl_task>(args[args.size() - 3]);
            auto cond = std::any_cast<std::condition_variable*>(args[args.size() - 2]);
            auto flag = std::any_cast<bool*>(args[args.size() - 1]);

            task(args);

            *flag = true;
            cond->notify_one();
        };

        args.push_back(task);
        args.push_back(&condition);
        args.push_back(&flag);

        enqueue_task(wrapper, std::move(args), target_queue_type);

        std::unique_lock lock(mutex);
        condition.wait(lock, [&] { 
            return flag; 
        });
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

    auto func = [](std::vector<std::any>& args)
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
    for (auto& target : targets) wait_fence(target);
}

/*
    Common Opengl Objects
*/

namespace {
    GLuint VAO;
    GLint textures_pool_size;
    GLint textures_operation_unit;
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

void pikango::OPENGL_ONLY_execute_on_context_thread(opengl_thread_task task, std::vector<std::any>&& args)
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
    auto func = [](std::vector<std::any>&)
    {
        //create VAO object
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        //get textures pool size
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &textures_pool_size);
        textures_pool_size--;   //Reserve last active texture for writing
        textures_operation_unit = textures_pool_size;
        glActiveTexture(GL_TEXTURE0 + textures_operation_unit);

        //enable scissors
        glEnable(GL_SCISSOR_TEST);

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
    auto func = [](std::vector<std::any>&)
    {
        glDeleteVertexArrays(1, &VAO);
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
    Command Buffer Bindings
*/

namespace cmd_bindings
{
    bool                    vertex_buffers_changed = false;
    std::array<GLint, 16>   vertex_buffers;

    bool    index_buffer_changed = false;
    GLint   index_buffer;

    bool    frame_buffer_changed = false;
    GLint   frame_buffer;

    bool                                        graphics_pipeline_changed = false;
    pikango_internal::graphics_pipeline_impl*   graphics_pipeline;
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

void pikango::cmd::bind_texture(
    texture_sampler_handle sampler,
    texture_buffer_handle buffer,
    size_t slot        
)
{
    auto func = [](std::vector<std::any>& args)
    {
        auto texture_sampler = std::any_cast<texture_sampler_handle>(args[0]);
        auto texture_buffer  = std::any_cast<texture_buffer_handle>(args[1]);
        auto slot   = std::any_cast<size_t>(args[2]);

        auto tsi = pikango_internal::obtain_handle_object(texture_sampler);
        auto tbi = pikango_internal::obtain_handle_object(texture_buffer);

        glBindSampler(slot, tsi->id);
        
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(tbi->type, tbi->id);
    };

    record_task(func, {sampler, buffer, slot});
}

#include "frame_buffer.hpp"

#include "binding.hpp"
#include "drawing_related.hpp"
#include "drawing.hpp"
