struct pikango_internal::shader_impl
{
    GLuint id = 0;
    pikango::shader_type type;

    ~shader_impl();
};

pikango::shader_handle pikango::new_shader(const shader_create_info& info)
{
    auto si = new pikango_internal::shader_impl;
    si->type = info.type;

    auto handle = pikango_internal::make_handle(si);

    auto func = [](std::vector<std::any>& args)
    {
        auto handle = std::any_cast<shader_handle>(args[0]);
        auto source = std::any_cast<const char*>(args[1]);
    
        auto si = pikango_internal::obtain_handle_object(handle);
    
        //Create And Compile Shader
        GLuint shader = glCreateShader(get_format_shader(si->type));
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);
    
        int success;
        char infoLog[512];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);  
        if (!success)
        {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            log_error(infoLog);
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
    };

    //waiting for execution so the source can be freed
    //waiting is bad, but there is no other way - other api's would just compile shader
    //while opengl require us to execute such task on our execution thread
    enqueue_task_and_wait(func, {handle, info.source}, pikango::queue_type::general);
    return handle;
};

void delete_dangling_program_pipelines(void* impl_ptr, pikango::shader_type type);

pikango_internal::shader_impl::~shader_impl()
{
    delete_dangling_program_pipelines(this, type);

    auto func = [](std::vector<std::any>& args)
    {
        auto id = std::any_cast<GLuint>(args[0]);
        glDeleteShader(id);
    };

    if (this->id != 0)
        enqueue_task(func, {id}, pikango::queue_type::general);
}
