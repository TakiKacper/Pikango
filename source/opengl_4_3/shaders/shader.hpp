PIKANGO_IMPL(shader)
{
    GLuint id = 0;
    pikango::shader_type type = pikango::shader_type::vertex;

    shader_uniforms_to_descriptors_maping bindings;

    ~shader_impl();
};

PIKANGO_NEW(shader)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::shader_impl);
    return handle;
};

void delete_dangling_program_pipelines(void* impl_ptr, pikango::shader_type type);

pikango_internal::shader_impl::~shader_impl()
{
    delete_dangling_program_pipelines(this, type);

    auto func = [](std::vector<std::any> args)
    {
        auto id = std::any_cast<GLuint>(args[0]);
        glDeleteShader(id);
    };

    if (this->id != 0)
        enqueue_task(func, {id}, pikango::queue_type::general);
}

void pikango::compile_shader(shader_handle target, pikango::shader_type type, const std::string& source)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<shader_handle>(args[0]);
        auto type   = std::any_cast<pikango::shader_type>(args[1]);
        auto source = std::any_cast<std::string>(args[2]);
    
        auto si = pikango_internal::obtain_handle_object(handle);
        auto source_ptr = source.c_str();
    
        //Create Shader
        GLuint shader = glCreateShader(get_format_shader(type));
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
    
        //Create Separatable Program
        GLuint program = glCreateProgram();
        glProgramParameteri(program, GL_PROGRAM_SEPARABLE, GL_TRUE);
        glAttachShader(program, shader);
        glLinkProgram(program);
    
        //Delete Shader
        glDetachShader(program, shader);
        glDeleteShader(shader);
    
        si->id = program;
        si->type = type;
    };

    enqueue_task(func, {target, type, source}, pikango::queue_type::general);
}

void pikango::OPENGL_ONLY_link_shader_bindings_info(shader_handle target, OPENGL_ONLY_shader_bindings& bindings)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<shader_handle>(args[0]);
        auto bindings = std::any_cast<pikango::OPENGL_ONLY_shader_bindings>(args[1]);

        auto si = pikango_internal::obtain_handle_object(handle);

        for (auto& [name, d_id, b_id, type] : bindings)
        {
            GLint loc = 0;

            switch (type)
            {
            case pikango::resources_descriptor_binding_type::sampled_texture:
            case pikango::resources_descriptor_binding_type::written_texture:
                loc = glGetUniformLocation(si->id, name.c_str());
            case pikango::resources_descriptor_binding_type::uniform_buffer:
                loc = glGetUniformBlockIndex(si->id, name.c_str());
            case pikango::resources_descriptor_binding_type::storage_buffer:
                loc = 0; //TODO WITH STORAGE BUFFERS
            }
            si->bindings.insert({{d_id, b_id}, {loc, type}});
        }
    };

    enqueue_task(func, {target, bindings}, pikango::queue_type::general);
}
