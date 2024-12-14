#include "glad/glad.h"

#include "enumerations.hpp"
#include "context_thread.hpp"

constexpr size_t textures_pool_size = 16;

void pikango::OPENGL_ONLY_execute_on_context_thread(opengl_thread_task task, std::vector<std::any> args)
{
    enqueue_task(task, std::move(args));
}

/*
    Common Opengl Objects
*/
static GLuint VAO;

/*
    Library Implementation
*/

std::string pikango::initialize_library_cpu()
{
    start_opengl_execution_thread();
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

/*
    Contructors / Deconstructors
*/

#define PIKANGO_IMPL(name)  \
    struct pikango_internal::name##_impl

#define PIKANGO_NEW(name)   \
    pikango::name##_handle pikango::new_##name ()

#define PIKANGO_DELETE(name)    \
    void pikango::delete_##name (pikango::name##_handle handle)

#include "buffers.hpp"
#include "data_layout.hpp"
#include "graphics_shader.hpp"

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

//
// Texture
//

size_t pikango::get_texture_pool_size()
{
    return textures_pool_size;
}

PIKANGO_IMPL(texture_2d)
{
    GLuint id     = 0;
    size_t width  = 0;
    size_t height = 0;
};

PIKANGO_NEW(texture_2d)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::texture_2d_impl);
    auto ti = pikango_internal::object_write_access(handle);
    ti->id = 0;
    return handle;
};

PIKANGO_DELETE(texture_2d)
{

};

void pikango::write_texture(
    texture_2d_handle target, 
    texture_format source_format, 
    texture_format inner_format,
    size_t width, 
    size_t height, 
    void* pixel_data
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<texture_2d_handle>(args[0]);
        auto source_format = std::any_cast<GLuint>(args[1]);
        auto inner_format = std::any_cast<GLuint>(args[2]);
        auto width = std::any_cast<size_t>(args[3]);
        auto height = std::any_cast<size_t>(args[4]);
        auto data = std::any_cast<void*>(args[5]);

        auto ti = pikango_internal::object_write_access(handle);

        if (ti->id == 0)
            glGenTextures(1, &ti->id);
        glBindTexture(GL_TEXTURE_2D, ti->id);

        glTexImage2D(GL_TEXTURE_2D, 0, inner_format, width, height, 0, source_format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    };
    enqueue_task(func, {target, get_texture_format(source_format), get_texture_format(inner_format), width, height, pixel_data});
}

void pikango::set_texture_wraping(texture_2d_handle target, texture_wraping x, texture_wraping y)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<texture_2d_handle>(args[0]);
        auto tw_x   = std::any_cast<GLuint>(args[1]);
        auto tw_y   = std::any_cast<GLuint>(args[2]);

        auto ti = pikango_internal::object_read_access(handle);

        glBindTexture(GL_TEXTURE_2D, ti->id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tw_x);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tw_y);
    };
    enqueue_task(func, {target, get_texture_wraping(x), get_texture_wraping(y)});
}

void pikango::set_texture_filtering(
    texture_2d_handle target, 
    texture_filtering magnifying, 
    texture_filtering minifying, 
    texture_filtering mipmap
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<texture_2d_handle>(args[0]);
        auto mag_filter = std::any_cast<GLuint>(args[1]);
        auto min_filter = std::any_cast<GLuint>(args[2]);

        auto ti = pikango_internal::object_read_access(handle);

        glBindTexture(GL_TEXTURE_2D, ti->id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    };

    enqueue_task(func, {target, get_texture_filtering(magnifying), combine_min_filters(minifying, mipmap)});
}

void pikango::bind_texture_to_pool(texture_2d_handle target, size_t index)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<texture_2d_handle>(args[0]);
        auto index = std::any_cast<size_t>(args[1]);

        auto ti = pikango_internal::object_read_access(handle);

        glActiveTexture(GL_TEXTURE0 + index);
        glBindTexture(GL_TEXTURE_2D, ti->id);
    };
    enqueue_task(func, {target, index});
}

#include "drawing.hpp"
