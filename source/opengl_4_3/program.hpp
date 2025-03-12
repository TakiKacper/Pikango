#pragma once

//Utilities enabling use of graphics shaders configuration as key for
//program pipeline registry
namespace{
    struct graphics_shaders_pipeline_config_impl_ptr_identifier
    {
        void* vertex_shader_impl_ptr;
        void* pixel_shader_impl_ptr;
        void* geometry_shader_impl_ptr;
    };

    struct graphics_shaders_pipeline_config_impl_ptr_identifier_hash
    {
        std::size_t operator()(const graphics_shaders_pipeline_config_impl_ptr_identifier& config) const
        {
            std::size_t seed = 0;
            seed = seed ^ (size_t)config.vertex_shader_impl_ptr;
            seed = seed ^ (size_t)config.pixel_shader_impl_ptr;
            seed = seed ^ (size_t)config.geometry_shader_impl_ptr;
            return seed;
        }
    };

    struct graphics_shaders_pipeline_impl_ptr_identifier_equal
    {
        bool operator()(
            const graphics_shaders_pipeline_config_impl_ptr_identifier& a, 
            const graphics_shaders_pipeline_config_impl_ptr_identifier& b
        ) const
        {
            return  a.vertex_shader_impl_ptr == b.vertex_shader_impl_ptr && 
                    a.pixel_shader_impl_ptr == b.pixel_shader_impl_ptr && 
                    a.geometry_shader_impl_ptr == b.geometry_shader_impl_ptr;
        }
    };
}

//program pipelines reigstry needs to be mutexed since it can be accessed 
//by both exection thread when applying bindings and other threads in shaders deconstructors
std::mutex program_pipelines_registry_mutex;
std::unordered_map<
    graphics_shaders_pipeline_config_impl_ptr_identifier, 
    GLuint,
    graphics_shaders_pipeline_config_impl_ptr_identifier_hash,
    graphics_shaders_pipeline_impl_ptr_identifier_equal> 
program_pipelines_registry;

GLuint get_program_pipeline(pikango::graphics_shaders_pipeline_config& config)
{
    graphics_shaders_pipeline_config_impl_ptr_identifier idtf;

    idtf.vertex_shader_impl_ptr     = pikango_internal::obtain_handle_object(config.vertex_shader);
    idtf.pixel_shader_impl_ptr      = pikango_internal::obtain_handle_object(config.pixel_shader);
    idtf.geometry_shader_impl_ptr   = pikango_internal::obtain_handle_object(config.geometry_shader);

    program_pipelines_registry_mutex.lock();

    //Search for existing pipeline
    auto it = program_pipelines_registry.find(idtf);
    if (it != program_pipelines_registry.end())
    {
        GLuint id = it->second;
        program_pipelines_registry_mutex.unlock();
        return id;
    }

    program_pipelines_registry_mutex.unlock();
    
    //Pipeline does not exist, create it
    GLuint pipeline;
    glGenProgramPipelines(1, &pipeline);
    
    if (!pikango_internal::is_empty(config.vertex_shader))
        glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT,   pikango_internal::obtain_handle_object(config.vertex_shader)->id);

    if (!pikango_internal::is_empty(config.pixel_shader))
        glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, pikango_internal::obtain_handle_object(config.pixel_shader)->id);

    if (!pikango_internal::is_empty(config.geometry_shader))
        glUseProgramStages(pipeline, GL_GEOMETRY_SHADER_BIT, pikango_internal::obtain_handle_object(config.geometry_shader)->id);

    //Insert pipeline for future use
    program_pipelines_registry_mutex.lock();
    program_pipelines_registry.insert({idtf, pipeline});
    program_pipelines_registry_mutex.unlock();

    //Todo: Handle Errors
    
    return pipeline;
};

void delete_dangling_program_pipelines(void* impl_ptr, pikango::shader_type type)
{
    size_t identifier_offset = 0;
    
    switch (type)
    {
        case pikango::shader_type::vertex:
            identifier_offset = offsetof(graphics_shaders_pipeline_config_impl_ptr_identifier, vertex_shader_impl_ptr);
            break;
        case pikango::shader_type::pixel:
            identifier_offset = offsetof(graphics_shaders_pipeline_config_impl_ptr_identifier, pixel_shader_impl_ptr);
            break;
        case pikango::shader_type::geometry:
            identifier_offset = offsetof(graphics_shaders_pipeline_config_impl_ptr_identifier, geometry_shader_impl_ptr);
            break;
    }

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

void delete_all_program_pipelines()
{
    program_pipelines_registry_mutex.lock();

    for (auto& [_, id] : program_pipelines_registry)
        glDeleteProgramPipelines(1, &id);
    program_pipelines_registry.clear();

    program_pipelines_registry_mutex.unlock();
}
