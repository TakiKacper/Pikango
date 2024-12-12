#include "glad/glad.h"

#include "enumerations.hpp"
#include "context_thread.hpp"

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

    auto func = [&](std::vector<std::any> args)
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

void pikango::draw_indexed(draw_indexed_args& args)
{
    auto func = [](std::vector<std::any> args)
    {
        auto draw_args = std::any_cast<pikango::draw_indexed_args>(args.front());

        glBindBuffer(GL_ARRAY_BUFFER, pikango_internal::object_read_access(draw_args.vertex_buffer)->id);

        //push data layout
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        //enable index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pikango_internal::object_read_access(draw_args.index_buffer)->id);

        glUseProgram(pikango_internal::object_read_access(draw_args.graphics_shader)->id);

        glDrawElements(GL_TRIANGLES, draw_args.indicies_count, GL_UNSIGNED_INT, 0);
    };

    enqueue_task(func, {args});
}
