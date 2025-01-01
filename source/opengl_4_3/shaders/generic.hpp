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

    auto si = pikango_internal::object_write_access(handle);

    auto source_ptr = source.c_str();

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

    si->id = shader;
}
