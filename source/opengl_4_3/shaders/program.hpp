#pragma once

GLuint get_program(pikango::graphics_shaders_pipeline_config& config)
{
    auto it = program_registry.find(config);
    if (it != program_registry.end())
        return it->second;
    
    GLuint program = glCreateProgram();

    glAttachShader(program, pikango_internal::object_read_access(config.vertex_shader)->id);
    glAttachShader(program, pikango_internal::object_read_access(config.pixel_shader)->id);

    if (!pikango_internal::is_empty(config.geometry_shader))
        glAttachShader(program, pikango_internal::object_read_access(config.geometry_shader)->id);

    int success;
    char infoLog[512];

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) 
    {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        pikango_internal::log_error(infoLog);
    }
    
    return program;
};