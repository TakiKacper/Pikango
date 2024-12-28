#include "glad/glad.h"
#include "runthread/context_thread.hpp"

/*
    Common Opengl Objects
*/
static GLuint VAO;
static pikango::frame_buffer_handle* default_frame_buffer_handle = nullptr;
static GLint textures_pool_size;
static GLint uniforms_pool_size;
static GLint textures_operation_unit;
static GLint max_color_attachments;

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

        //get uniforms pool size
        glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &uniforms_pool_size);

        //get max color attachments
        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max_color_attachments);
    };

    enqueue_task(func, {});
    pikango::wait_all_queues_empty();
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

size_t pikango::get_texture_pool_size()
{
    return textures_pool_size;
}

size_t pikango::get_uniform_pool_size()
{
    return uniforms_pool_size;
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
#include "buffers/uniform_buffer.hpp"

#include "data_layout/data_layout.hpp"

#include "shaders/graphics_shader.hpp"

#include "textures/generic.hpp"
#include "textures/texture_1d.hpp"
#include "textures/texture_2d.hpp"
#include "textures/texture_3d.hpp"
#include "textures/texture_cube.hpp"

#include "frame_buffer/generic.hpp"
#include "frame_buffer/frame_buffer.hpp"

#include "drawing/drawing.hpp"
