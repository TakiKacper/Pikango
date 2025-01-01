//to delete
#include <iostream>

#include "glad/glad.h"
#include "enumerations/enumerations.hpp"

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

#include "execution_thread/execution_thread.hpp"
#include "command_buffer/command_buffer.hpp"
#include "fence/fence.hpp"

namespace
{
    void record_task(const opengl_task& task, std::vector<std::any> args)
    {
        auto cbi = pikango_internal::object_write_access(recording_command_buffer);
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
    auto cbi = pikango_internal::object_read_access(cb);
    
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
    auto cbi = pikango_internal::object_read_access(cb);
    
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
        auto const_fi = pikango_internal::object_read_access(fence);
        auto fi = const_cast<pikango_internal::fence_impl*>(*const_fi);

        fi->is_signaled = true;
        fi->condition.notify_one();
    };

    mutex->lock();

    auto fi = pikango_internal::object_write_access(fence);
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
    //using read access does not block the handle mutex
    //it is required so the exectution thread could signal the fence
    auto const_fi = pikango_internal::object_read_access(target);
    auto fi = const_cast<pikango_internal::fence_impl*>(*const_fi);

    if (fi->subbmitted == false) return;

    std::unique_lock<std::mutex> lock(fi->mutex);
    fi->condition.wait(lock, [&] { return fi->is_signaled || !fi->subbmitted; });
}

struct graphics_shaders_pipeline_config_hash
{
    std::size_t operator()(const pikango::graphics_shaders_pipeline_config& config) const
    {
        std::size_t seed = 0;
        seed = seed ^ (size_t)pikango_internal::get_handle_meta_block_address(config.vertex_shader);
        seed = seed ^ (size_t)pikango_internal::get_handle_meta_block_address(config.pixel_shader);
        seed = seed ^ (size_t)pikango_internal::get_handle_meta_block_address(config.geometry_shader);
        return seed;
    }
};

struct graphics_shaders_pipeline_config_equal
{
    bool operator()(const pikango::graphics_shaders_pipeline_config& a, const pikango::graphics_shaders_pipeline_config& b) const
    {
        return  a.vertex_shader == b.vertex_shader && 
                a.pixel_shader == b.pixel_shader && 
                a.geometry_shader == b.geometry_shader;
    }
};

/*
    Common Opengl Objects
*/

namespace
{
    GLuint VAO;
    pikango::frame_buffer_handle* default_frame_buffer_handle = nullptr;
    GLint textures_pool_size;
    GLint uniforms_pool_size;
    GLint textures_operation_unit;
    GLint max_color_attachments;
    std::unordered_map<
        pikango::graphics_shaders_pipeline_config, 
        GLuint, 
        graphics_shaders_pipeline_config_hash, 
        graphics_shaders_pipeline_config_equal> 
    program_registry;
}

/*
    Library Implementation
*/

void pikango::OPENGL_ONLY_execute_on_context_thread(opengl_thread_task task, std::vector<std::any> args)
{
    enqueue_task(task, std::move(args), pikango::queue_type::general);
}

pikango::frame_buffer_handle pikango::OPENGL_ONLY_get_default_frame_buffer()
{
    return *default_frame_buffer_handle;
}

std::string pikango::initialize_library_cpu()
{
    start_opengl_execution_thread();
    return "";
}

//fwd
pikango::frame_buffer_handle* create_default_framebuffer_handle();

std::string pikango::initialize_library_gpu()
{
    auto func = [](std::vector<std::any>)
    {
        //create VAO object
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        //create default frame buffer
        default_frame_buffer_handle = create_default_framebuffer_handle();

        //get textures pool size
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &textures_pool_size);
        textures_pool_size--;   //Reserve last active texture for writing
        textures_operation_unit = textures_pool_size;
        glActiveTexture(GL_TEXTURE0 + textures_operation_unit);

        //get uniforms pool size
        glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &uniforms_pool_size);

        //get max color attachments
        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max_color_attachments);
    };

    enqueue_task(func, {}, pikango::queue_type::general);
    pikango::wait_all_queues_empty();
    return "";
}

std::string pikango::terminate()
{
    auto func = [](std::vector<std::any>)
    {
        glDeleteVertexArrays(1, &VAO);
        delete default_frame_buffer_handle;

        for (auto& [_, id] : program_registry)
            glDeleteProgram(id);
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

size_t pikango::get_texture_pool_size()
{
    return textures_pool_size;
}

size_t pikango::get_uniform_pool_size()
{
    return uniforms_pool_size;
}

/*
    Commands Implementations
*/

#include "pipelines/graphics_pipeline.hpp"

#include "shaders/generic.hpp"
#include "shaders/vertex_shader.hpp"
#include "shaders/pixel_shader.hpp"
#include "shaders/geometry_shader.hpp"
//#include "shaders/compute_shader.hpp"
#include "shaders/program.hpp"

#include "buffers/generic.hpp"
#include "buffers/vertex_buffer.hpp"
#include "buffers/index_buffer.hpp"
#include "buffers/instance_buffer.hpp"
#include "buffers/uniform_buffer.hpp"

#include "shaders/graphics_shader.hpp"

#include "textures/generic.hpp"
#include "textures/texture_1d.hpp"
#include "textures/texture_2d.hpp"
#include "textures/texture_3d.hpp"
#include "textures/texture_cube.hpp"

#include "frame_buffer/generic.hpp"
#include "frame_buffer/frame_buffer.hpp"

#include "drawing/binding.hpp"
#include "drawing/drawing.hpp"
