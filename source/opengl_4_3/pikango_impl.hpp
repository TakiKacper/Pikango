#include "glad/glad.h"

#include "runthread/context_thread.hpp"

/*
    Common Opengl Objects
*/
static GLuint VAO;
static pikango::frame_buffer_handle* default_frame_buffer_handle = nullptr;
static GLint textures_pool_size;
static GLint textures_operation_unit;

/*
    Library Implementation
*/

void pikango::OPENGL_ONLY_execute_on_context_thread(opengl_thread_task task, std::vector<std::any> args)
{
    enqueue_task(task, std::move(args));
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
    };

    enqueue_task(func, {});
    return "";
}

std::string pikango::terminate()
{
    auto func = [](std::vector<std::any>)
    {
        glDeleteVertexArrays(1, &VAO);
        delete default_frame_buffer_handle;
    };

    enqueue_task(func, {});
    stop_opengl_execution_thread();
    return "";
}

static const char* glsl = "glsl";
const char* pikango::get_used_shading_language_name()
{
    return glsl;
}

void pikango::wait_all_tasks_completion()
{
    if (opengl_tasks_queue.size() == 0) return;
    
    std::unique_lock lock(all_tasks_done_mutex);
    all_tasks_done_condition.wait(lock, []{ return opengl_tasks_queue.size() == 0; });
}

void pikango::wait_all_current_tasks_completion()
{
    static std::mutex mutex;
    static std::condition_variable condition;

    auto func = [](std::vector<std::any> args)
    {
        auto flag = std::any_cast<bool*>(args[0]);
        *flag = true;

        condition.notify_one();
    };

    std::unique_lock lock(mutex);
    bool flag = false;

    enqueue_task(func, {&flag});
    condition.wait(lock, [&]{return flag;});
}

size_t pikango::get_texture_pool_size()
{
    return textures_pool_size;
}

#define PIKANGO_IMPL(name)  \
    struct pikango_internal::name##_impl

#define PIKANGO_NEW(name)   \
    pikango::name##_handle pikango::new_##name ()

#define PIKANGO_DELETE(name)    \
    void pikango::delete_##name (pikango::name##_handle handle)

#include "enumerations/enumerations.hpp"

#include "buffers/generic.hpp"
#include "buffers/vertex_buffer.hpp"
#include "buffers/index_buffer.hpp"
#include "buffers/instance_buffer.hpp"

#include "data_layout/data_layout.hpp"

#include "shaders/graphics_shader.hpp"

#include "textures/generic.hpp"
#include "textures/texture_1d.hpp"
#include "textures/texture_2d.hpp"
#include "textures/texture_3d.hpp"
#include "textures/texture_cube.hpp"

#include "frame_buffer/frame_buffer.hpp"

PIKANGO_IMPL(frame_buffer)
{
    GLuint id = 0;
    pikango::texture_2d_handle color_attachment;
    pikango::texture_2d_handle depth_attachment;
    pikango::texture_2d_handle stencil_attachment;
};

PIKANGO_NEW(frame_buffer)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::frame_buffer_impl);

    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<frame_buffer_handle>(args[0]);
        auto fbi = pikango_internal::object_write_access(handle);
        glGenFramebuffers(1, &fbi->id);
    };

    enqueue_task(func, {handle});
    return handle;
}

PIKANGO_DELETE(frame_buffer)
{ 

};

pikango::frame_buffer_handle* create_default_framebuffer_handle()
{
    auto handle_ptr = pikango_internal::alloc_handle(new pikango_internal::frame_buffer_impl);
    auto fbi = pikango_internal::object_write_access(*handle_ptr);
    fbi->id = 0;
    return handle_ptr;
}

template<GLenum gl_attachment_type>
void attach_framebuffer_buffer_generic(
    pikango::frame_buffer_handle target,
    pikango::texture_2d_handle attachment
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto target = std::any_cast<pikango::frame_buffer_handle>(args[0]);
        auto attachment = std::any_cast<pikango::texture_2d_handle>(args[1]);

        auto fbi = pikango_internal::object_read_access(target);
        auto ti = pikango_internal::object_read_access(attachment);

        glBindFramebuffer(GL_FRAMEBUFFER, fbi->id);
        glFramebufferTexture2D(GL_FRAMEBUFFER, gl_attachment_type, GL_TEXTURE_2D, ti->id, 0);
    };

    enqueue_task(func, {target, attachment});
}

void pikango::attach_framebuffer_color_buffer(
    frame_buffer_handle target,
    texture_2d_handle attachment
)
{
    pikango_internal::object_write_access(target)->color_attachment = attachment;
    attach_framebuffer_buffer_generic<GL_COLOR_ATTACHMENT0>(target, attachment);
}

void pikango::attach_framebuffer_depth_buffer(
    frame_buffer_handle target,
    texture_2d_handle attachment
)
{
    pikango_internal::object_write_access(target)->depth_attachment = attachment;
    attach_framebuffer_buffer_generic<GL_DEPTH_ATTACHMENT>(target, attachment);
}

void pikango::attach_framebuffer_stencil_buffer(
    frame_buffer_handle target,
    texture_2d_handle attachment
)
{
    pikango_internal::object_write_access(target)->stencil_attachment = attachment;
    attach_framebuffer_buffer_generic<GL_STENCIL_ATTACHMENT>(target, attachment);
}

void pikango::attach_framebuffer_depth_stencil_buffer(
    frame_buffer_handle target,
    texture_2d_handle attachment
)
{
    auto fbi = pikango_internal::object_write_access(target);
    fbi->depth_attachment = attachment;
    fbi->stencil_attachment = attachment;
    attach_framebuffer_buffer_generic<GL_DEPTH_STENCIL_ATTACHMENT>(target, attachment);
}

#include "drawing/drawing.hpp"
