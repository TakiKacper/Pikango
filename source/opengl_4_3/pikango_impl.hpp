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
            "  FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
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
            pikango_internal::log_error(infoLog);
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
            pikango_internal::log_error(infoLog);
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
            pikango_internal::log_error(infoLog);
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
