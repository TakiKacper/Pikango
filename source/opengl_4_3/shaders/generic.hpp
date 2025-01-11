template <size_t identifier_offset>
void delete_dangling_program_pipelines(void* impl_ptr)
{
    std::vector<GLuint> pipelines_to_delete;

    program_pipelines_registry_mutex.lock();

    auto itr = program_pipelines_registry.begin();
    while (itr != program_pipelines_registry.end())
    {
        auto& key = itr->first;
        auto& id = itr->second;

        //get base address of identifier
        //then offset it by identifier_offset so the pointer
        //it now points to an struct member: vertex_shader_impl_ptr, pixel_shader_impl_ptr etc.
        void** shader_obj_member_pos = reinterpret_cast<void**>((uint8_t*)(&key) + identifier_offset);

        //now we compare the member
        if (*shader_obj_member_pos == impl_ptr)
        {
            pipelines_to_delete.push_back(id);
            itr = program_pipelines_registry.erase(itr);
        }
        else
            ++itr;
    }

    program_pipelines_registry_mutex.unlock();

    auto func = [](std::vector<std::any> args)
    {
        auto ids = std::any_cast<std::vector<GLuint>>(args[0]);
        glDeleteProgramPipelines(ids.size(), &ids[0]);
    };

    if (pipelines_to_delete.size() != 0)
        enqueue_task(func, {std::move(pipelines_to_delete)}, pikango::queue_type::general);
}

template <class impl_type>
void delete_shader_generic(impl_type* _this)
{
    auto func = [](std::vector<std::any> args)
    {
        auto id = std::any_cast<GLuint>(args[0]);
        glDeleteShader(id);
    };

    if (_this->id != 0)
        enqueue_task(func, {_this->id}, pikango::queue_type::general);
}

template<class shader_handle_type, GLuint ShaderTypeFlag>
void compile_shader_task(std::vector<std::any> args)
{
    shader_handle_type handle = std::any_cast<shader_handle_type>(args[0]);
    std::string source = std::any_cast<std::string>(args[1]);

    auto si = pikango_internal::obtain_handle_object(handle);

    auto source_ptr = source.c_str();

    //Create Shader
    GLuint shader = glCreateShader(ShaderTypeFlag);
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
}

template<class handle_type>
void OPENGL_ONLY_link_shader_bindings_info_generic(handle_type handle, pikango::OPENGL_ONLY_shader_bindings& bindings)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<handle_type>(args[0]);
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

    enqueue_task(func, {handle, bindings}, pikango::queue_type::general);
}
