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

void bind_vertex_layout(const pikango::data_layout_handle& handle)
{
    auto dli = pikango_internal::object_read_access(handle);

    int i = 0;

    size_t size = pikango::get_layout_size_with_stride(handle);
    size_t offset = 0;

    for (; i < dli->layout.size(); i++)
    {
        auto data_type = dli->layout.at(i);

        auto dt = dli->layout.at(i);

        glVertexAttribPointer(i, get_elements_in_data_type(dt), get_data_type(dt), GL_FALSE, size, (void*)(uintptr_t)(offset));
		glEnableVertexAttribArray(i);
    }

    for (; i < 16; i++)
    {
        glDisableVertexAttribArray(i);
    }
}

void pikango::draw_vertices(draw_vertices_args& args)
{
    auto func = [](std::vector<std::any> args)
    {
        auto draw_args = std::any_cast<pikango::draw_vertices_args>(args.front());

        //bind vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, pikango_internal::object_read_access(draw_args.vertex_buffer)->id);
        
        //bind layout
        bind_vertex_layout(draw_args.data_layout);

        //bind shader
        glUseProgram(pikango_internal::object_read_access(draw_args.graphics_shader)->id);

        glDrawArrays(
            get_primitive(draw_args.primitive), 
            draw_args.first_vertex_index, 
            draw_args.vertices_count
        );
    };

    enqueue_task(func, {args});
}

void pikango::draw_indexed(draw_indexed_args& args)
{
    auto func = [](std::vector<std::any> args)
    {
        auto draw_args = std::any_cast<pikango::draw_indexed_args>(args.front());

        //bind vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, pikango_internal::object_read_access(draw_args.vertex_buffer)->id);

        //bind layout
        bind_vertex_layout(draw_args.data_layout);
        
        //enable index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pikango_internal::object_read_access(draw_args.index_buffer)->id);

        //bind shader
        glUseProgram(pikango_internal::object_read_access(draw_args.graphics_shader)->id);

        glDrawElements(
            get_primitive(draw_args.primitive), 
            draw_args.indicies_count, 
            GL_UNSIGNED_INT, 
            0
        );
    };

    enqueue_task(func, {args});
}
