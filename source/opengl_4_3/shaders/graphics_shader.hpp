/*
    Implementation
*/

PIKANGO_IMPL(graphics_shader)
{
    GLuint id;
};

PIKANGO_NEW(graphics_shader)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::graphics_shader_impl);
    auto gsi = pikango_internal::object_write_access(handle);
    gsi->id = 0;
    return handle;
};

PIKANGO_DELETE(graphics_shader)
{

};

/*
    Methods
*/

template<class T, GLuint ShaderTypeFlag>
void compile_shader_part_task(std::vector<std::any> args)
{
    T* sp = std::any_cast<T*>(args[0]);
    std::string source = std::any_cast<std::string>(args[1]);

    auto source_ptr = source.c_str();

    unsigned int shader = glCreateShader(ShaderTypeFlag);
    glShaderSource(shader, 1, &source_ptr, NULL);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        pikango_internal::log_error(infoLog);
    }

    sp->id = shader;
}

template<class T>
void delete_shader_part_task(std::vector<std::any> args)
{
    T* sp = std::any_cast<T*>(args[0]);
    glDeleteShader(sp->id);
    delete sp;
}

struct pikango::shader_part_vertex
{
    GLuint id;
};

struct pikango::shader_part_geometry
{
    GLuint id;
};

struct pikango::shader_part_pixel
{
    GLuint id;
};

pikango::shader_part_vertex* pikango::compile_shader_part_vertex(const std::string& source)
{
    shader_part_vertex* sp = new shader_part_vertex;
    enqueue_task(compile_shader_part_task<shader_part_vertex, GL_VERTEX_SHADER>, {sp, source}, queue_type::general);
    return sp;    
}

pikango::shader_part_geometry* pikango::compile_shader_part_geometry(const std::string& source)
{
    shader_part_geometry* sp = new shader_part_geometry;
    enqueue_task(compile_shader_part_task<shader_part_geometry, GL_GEOMETRY_SHADER>, {sp, source}, queue_type::general);
    return sp;    
}

pikango::shader_part_pixel* pikango::compile_shader_part_pixel(const std::string& source)
{
    shader_part_pixel* sp = new shader_part_pixel;
    enqueue_task(compile_shader_part_task<shader_part_pixel, GL_FRAGMENT_SHADER>, {sp, source}, queue_type::general);
    return sp;    
}

void pikango::free_shader_part_vertex(shader_part_vertex* sp)
{
    enqueue_task(delete_shader_part_task<shader_part_vertex>, {sp}, queue_type::general);
}

void pikango::free_shader_part_geometry(shader_part_geometry* sp)
{
    enqueue_task(delete_shader_part_task<shader_part_geometry>, {sp}, queue_type::general);
}

void pikango::free_shader_part_pixel(shader_part_pixel* sp)
{
    enqueue_task(delete_shader_part_task<shader_part_pixel>, {sp}, queue_type::general);
}

void pikango::link_graphics_shader(
    graphics_shader_handle target, 
    const shader_part_vertex* spv, 
    const shader_part_geometry* spg, 
    const shader_part_pixel* spp
)
{
    auto func = [](std::vector<std::any> args)
    {
        graphics_shader_handle handle = std::any_cast<graphics_shader_handle>(args[0]);
        auto gsi = pikango_internal::object_write_access(handle);

        const shader_part_vertex*   spv = std::any_cast<const shader_part_vertex*>(args[1]);
        const shader_part_geometry* spg = std::any_cast<const shader_part_geometry*>(args[2]);
        const shader_part_pixel*    spp = std::any_cast<const shader_part_pixel*>(args[3]);

        unsigned int graphics_shader = glCreateProgram();

        if (spv == nullptr || spv->id == 0)
            pikango_internal::log_error("Shader linking error: vertex shader part was nullptr/empty");
        else
            glAttachShader(graphics_shader, spv->id);

        if (spg != nullptr && spg->id == 0)
            pikango_internal::log_error("Shader linking error: geometry shader part was empty");
        else if (spg != nullptr)
            glAttachShader(graphics_shader, spg->id);

        if (spp == nullptr || spp->id == 0)
            pikango_internal::log_error("Shader linking error: pixel shader part was nullptr/empty");
        else
            glAttachShader(graphics_shader, spp->id);

        int success;
        char infoLog[512];

        glLinkProgram(graphics_shader);
        glGetProgramiv(graphics_shader, GL_LINK_STATUS, &success);

        if (!success) 
        {
            glGetProgramInfoLog(graphics_shader, 512, NULL, infoLog);
            pikango_internal::log_error(infoLog);
        }

        if (gsi->id != 0)
            glDeleteProgram(gsi->id);

        gsi->id = graphics_shader;
    };
    enqueue_task(func, {target, spv, spg, spp}, pikango::queue_type::general);
}

void pikango::cmd::bind_shader_sampler_to_pool(
    graphics_shader_handle target,
    const std::string& sampler_access,
    size_t pool_index
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<graphics_shader_handle>(args[0]);
        auto sampler = std::any_cast<std::string>(args[1]);
        auto index = std::any_cast<size_t>(args[2]);

        auto gsi = pikango_internal::object_read_access(handle);

        glUseProgram(gsi->id);
        glUniform1i(glGetUniformLocation(gsi->id, sampler.c_str()), index);
    };
    record_task(func, {target, sampler_access, pool_index});
}

void pikango::cmd::bind_shader_uniform_to_pool(
    graphics_shader_handle target,
    const std::string& uniform_access,
    size_t pool_index
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<graphics_shader_handle>(args[0]);
        auto uniform = std::any_cast<std::string>(args[1]);
        auto index = std::any_cast<size_t>(args[2]);

        auto gsi = pikango_internal::object_read_access(handle);

        auto uniform_index = glGetUniformBlockIndex(gsi->id, uniform.c_str());   
        glUniformBlockBinding(gsi->id, uniform_index, index);
    };
    record_task(func, {target, uniform_access, pool_index});
}
