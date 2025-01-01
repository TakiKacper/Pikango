#pragma once

GLuint get_program_pipeline(pikango::graphics_shaders_pipeline_config& config)
{
    program_pipelines_registry_mutex.lock();

    //Search for existing pipeline
    auto it = program_pipelines_registry.find(config);
    if (it != program_pipelines_registry.end())
        return it->second;

    program_pipelines_registry_mutex.unlock();
    
    //Pipeline does not exist, create it
    GLuint pipeline;
    glGenProgramPipelines(1, &pipeline);
    
    glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, pikango_internal::object_read_access(config.vertex_shader)->id);
    glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, pikango_internal::object_read_access(config.pixel_shader)->id);

    if (!pikango_internal::is_empty(config.geometry_shader))
        glUseProgramStages(pipeline, GL_GEOMETRY_SHADER_BIT, pikango_internal::object_read_access(config.geometry_shader)->id);

    //Insert pipeline for future use
    program_pipelines_registry_mutex.lock();
    program_pipelines_registry.insert({config, pipeline});
    program_pipelines_registry_mutex.unlock();

    //Todo: Handle Errors
    
    return pipeline;
};
