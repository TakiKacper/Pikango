#pragma once
#include <set>

namespace {
    pikango::graphics_pipeline_handle   binded_graphics_pipeline;
    bool graphics_pipeline_changed = true;

    pikango::rasterization_pipeline_config recent_rasterization_config;
    pikango::depth_stencil_pipeline_config recent_depth_stencil_config;

    pikango::vertex_buffer_handle       binded_vertex_buffer;
    pikango::instance_buffer_handle     binded_instance_buffer;
    bool vao_bindings_changed = true;

    pikango::frame_buffer_handle        binded_frame_buffer;
    bool fbo_bindings_changed = true;

    pikango::index_buffer_handle        binded_index_buffer;
    bool ibo_bindings_changed = true;

    std::array<pikango::resources_descriptor_handle, max_resources_descriptors> binded_resources_descriptors;
    std::set<size_t> changed_resources_descriptors;

    std::unordered_map<pikango_internal::size_t_pair, GLint, pikango_internal::size_t_pair_hash> descriptors_to_opengl_pools_mapping;
};

void pikango::cmd::bind_graphics_pipeline(graphics_pipeline_handle pipeline)
{
    auto func = [](std::vector<std::any> args)
    {
        auto pipeline = std::any_cast<graphics_pipeline_handle>(args[0]);

        if (pipeline == binded_graphics_pipeline) return;

        binded_graphics_pipeline = pipeline;
        graphics_pipeline_changed = true;
        vao_bindings_changed = true;
    };

    record_task(func, {pipeline});
}

void pikango::cmd::bind_resources_descriptor(resources_descriptor_handle descriptor, size_t descriptor_index)
{
    auto func = [](std::vector<std::any> args)
    {
        auto descriptor = std::any_cast<resources_descriptor_handle>(args[0]);
        auto descriptor_index = std::any_cast<size_t>(args[1]);

        if (binded_resources_descriptors.at(descriptor_index) == descriptor) return;

        binded_resources_descriptors.at(descriptor_index) = descriptor;
        changed_resources_descriptors.insert(descriptor_index);
    };

    record_task(func, {descriptor, descriptor_index});
}

void pikango::cmd::bind_frame_buffer(frame_buffer_handle frame_buffer)
{
    auto func = [](std::vector<std::any> args)
    {
        auto frame_buffer = std::any_cast<frame_buffer_handle>(args[0]);

        if (frame_buffer == binded_frame_buffer) return;

        binded_frame_buffer = frame_buffer;
        fbo_bindings_changed = true;
    };

    record_task(func, {frame_buffer});
}

void pikango::cmd::bind_vertex_buffer(vertex_buffer_handle vertex_buffer)
{
    auto func = [](std::vector<std::any> args)
    {
        auto vertex_buffer = std::any_cast<vertex_buffer_handle>(args[0]);

        if (vertex_buffer == binded_vertex_buffer) return;

        binded_vertex_buffer = vertex_buffer;
        vao_bindings_changed = true;
    };

    record_task(func, {vertex_buffer});
}

void pikango::cmd::bind_instance_buffer(instance_buffer_handle instance_buffer)
{
    auto func = [](std::vector<std::any> args)
    {
        auto instance_buffer = std::any_cast<instance_buffer_handle>(args[0]);

        if (instance_buffer == binded_instance_buffer) return;

        binded_instance_buffer = instance_buffer;
        vao_bindings_changed = true;
    };

    record_task(func, {instance_buffer});
}

void pikango::cmd::bind_index_buffer(index_buffer_handle index_buffer)
{
    auto func = [](std::vector<std::any> args)
    {
        auto index_buffer = std::any_cast<index_buffer_handle>(args[0]);

        if (index_buffer == binded_index_buffer) return;

        binded_index_buffer = index_buffer;
        ibo_bindings_changed = true;
    };

    record_task(func, {index_buffer});
}

static void apply_vertex_layout();
static void apply_graphics_pipeline_shaders();
static void apply_graphics_pipeline_settings();
static void apply_resources_descriptors_and_shaders_uniforms();

//we wait with actual binding until the draw because of openGl desing the bindings
//could be overriden by other non-related stuff like writing to unrelated buffer or something
static void apply_bindings()
{
    if (vao_bindings_changed)
        apply_vertex_layout();
    vao_bindings_changed = false;

    if (ibo_bindings_changed)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pikango_internal::object_read_access(binded_index_buffer)->id);
    ibo_bindings_changed = false;

    if (fbo_bindings_changed)
        glBindFramebuffer(GL_FRAMEBUFFER, pikango_internal::object_read_access(binded_frame_buffer)->id);
    fbo_bindings_changed = false;

    if (graphics_pipeline_changed)
        apply_graphics_pipeline_settings();
    graphics_pipeline_changed = false;

    if (changed_resources_descriptors.size() != 0 || graphics_pipeline_changed)
        apply_resources_descriptors_and_shaders_uniforms();
    changed_resources_descriptors.clear();

    //Always bind the program pipeline because it could have been
    //overwritten by shaders functions
    apply_graphics_pipeline_shaders();
}

static size_t calc_attributes_size(std::vector<pikango::data_type>& attributes)
{
    size_t size = 0;
    for (auto& dt : attributes)
        size += pikango::size_of(dt);
    return size;
}

void apply_vertex_layout()
{
    auto& vlc = pikango_internal::object_write_access(binded_graphics_pipeline)->config.vertex_layout_config;

    int attrib_id = 0;
    size_t offset = 0;

    auto bind_layout = [&](bool instance)
    {
        auto& attributes = instance ? vlc.instance_attributes : vlc.vertex_attributes;
        size_t size = calc_attributes_size(attributes);

        GLuint target_buffer_id;
        if (instance) target_buffer_id = pikango_internal::object_read_access(binded_instance_buffer)->id;
        else          target_buffer_id = pikango_internal::object_read_access(binded_vertex_buffer)->id;

        glBindBuffer(GL_ARRAY_BUFFER, target_buffer_id);

        int i = 0;

        for (; i < attributes.size(); i++)
        {
            auto dt = attributes.at(i);

            glEnableVertexAttribArray(attrib_id + i);
            glVertexAttribPointer(
                attrib_id + i, 
                get_elements_in_data_type(dt), 
                get_data_type(dt), 
                GL_FALSE, 
                size, 
                (void*)(uintptr_t)(offset)
            );
            glVertexAttribDivisor(attrib_id + i, instance ? 1 : 0);

            offset += pikango::size_of(dt);
        }

        attrib_id += i;
    };

    bind_layout(false);
    if (!pikango_internal::is_empty(binded_instance_buffer)) 
        bind_layout(true);

    for (; attrib_id < 16; attrib_id++)
        glDisableVertexAttribArray(attrib_id);
}

void apply_graphics_pipeline_shaders()
{
    //Apply shaders
    auto gpi = pikango_internal::object_write_access(binded_graphics_pipeline);

    GLuint program_pipeline = get_program_pipeline(gpi->config.shaders_config);
    glUseProgram(0);
    glBindProgramPipeline(program_pipeline);
}

void apply_graphics_pipeline_settings()
{
    auto gpi = pikango_internal::object_write_access(binded_graphics_pipeline);

    //Apply rasterization settings  
    auto& rast = gpi->config.rasterization_config;
    auto& r_rast = recent_rasterization_config;

    if (rast.enable_culling != r_rast.enable_culling)
    {
        if (rast.enable_culling) glEnable(GL_CULL_FACE);
        else                     glDisable(GL_CULL_FACE);  
    }

    if (rast.polygon_fill != r_rast.polygon_fill)
        glPolygonMode(GL_FRONT_AND_BACK, get_rasterization_fill(rast.polygon_fill));

    if (rast.culling_mode != r_rast.culling_mode)
        glCullFace(get_culling_mode(rast.culling_mode));

    if (rast.culling_front_face != r_rast.culling_front_face)
        glFrontFace(get_front_face(rast.culling_front_face));

    if (rast.line_width != r_rast.line_width)
        glLineWidth(rast.line_width);

    r_rast = rast;

    //Apply depth stencil settings
    auto& ds = gpi->config.depth_stencil_config;
    auto& r_ds = recent_depth_stencil_config;

    if (ds.enable_depth_test != r_ds.enable_depth_test)
    {
        if (ds.enable_depth_test) glEnable(GL_DEPTH_TEST);
        else                      glDisable(GL_DEPTH_TEST);  
    }

    if (ds.enable_depth_write != r_ds.enable_depth_write)
    {
        if (ds.enable_depth_write) glDepthMask(true);
        else                       glDepthMask(false);  
    }

    r_ds = ds;
}

template<class handle_type>
static void set_shaders_uniforms(handle_type handle)
{
    if (pikango_internal::is_empty(handle)) return;
    auto si = pikango_internal::object_read_access(handle);

    glUseProgram(si->id);

    for (auto& binding : si->bindings)
    {
        GLint pool_id = 0;
        
        auto itr = descriptors_to_opengl_pools_mapping.find(binding.first);

        if (itr == descriptors_to_opengl_pools_mapping.end())
        {
            pikango_internal::log_error(
                (
                    std::string("Invalid descriptor mapping: desc: ") + 
                    std::to_string(binding.first.first) +
                    ", binding: " + 
                    std::to_string(binding.first.second)
                ).c_str()
            );
            pool_id = 0;
        }
        else 
            pool_id = (*itr).second;

        switch (binding.second.second)
        {
        case pikango::resources_descriptor_binding_type::sampled_texture:
        case pikango::resources_descriptor_binding_type::written_texture:
            glUniform1i(binding.second.first, pool_id);
            break;
        case pikango::resources_descriptor_binding_type::uniform_buffer:
            glUniformBlockBinding(si->id, binding.second.first, pool_id);
            break;
        case pikango::resources_descriptor_binding_type::storage_buffer:
            //TODO
            break;
        }
    }
}

void bind_any_texture(const pikango::resources_descriptor_resource_handle& vhandle)
{
    GLenum type;
    GLint id;

    if (std::holds_alternative<pikango::texture_1d_handle>(vhandle)) {
        auto handle = std::get<pikango::texture_1d_handle>(vhandle);
        auto ti = pikango_internal::object_read_access(handle);
        type = GL_TEXTURE_1D;
        id = ti->id;
    } 
    else if (std::holds_alternative<pikango::texture_2d_handle>(vhandle)) {
        auto handle = std::get<pikango::texture_2d_handle>(vhandle);
        auto ti = pikango_internal::object_read_access(handle);
        type = GL_TEXTURE_2D;
        id = ti->id;
    } 
    else if (std::holds_alternative<pikango::texture_3d_handle>(vhandle)) {
        auto handle = std::get<pikango::texture_3d_handle>(vhandle);
        auto ti = pikango_internal::object_read_access(handle);
        type = GL_TEXTURE_3D;
        id = ti->id;
    } 
    else if (std::holds_alternative<pikango::texture_cube_handle>(vhandle)) {
        auto handle = std::get<pikango::texture_cube_handle>(vhandle);
        auto ti = pikango_internal::object_read_access(handle);
        type = GL_TEXTURE_CUBE_MAP;
        id = ti->id;
    } 
    /*else if (std::holds_alternative<pikango::texture_array_1d_handle>(vhandle)) {
        auto handle = std::get<pikango::texture_array_1d_handle>(vhandle);
        auto ti = pikango_internal::object_read_access(handle);
        type = 
        id = ti->id;
    } 
    else if (std::holds_alternative<pikango::texture_array_2d_handle>(vhandle)) {
        auto handle = std::get<pikango::texture_array_2d_handle>(vhandle);
        auto ti = pikango_internal::object_read_access(handle);
        type = 
        id = ti->id;
    } */

   glBindTexture(type, id);
}

void apply_resources_descriptors_and_shaders_uniforms()
{
    //1: bind resources to their pools
    descriptors_to_opengl_pools_mapping.clear();

    size_t d_id = 0;
    size_t b_id = 0;

    GLint texture_pool_itr = 0;
    GLint uniform_pool_itr = 0;

    auto bind_texture = [&](const pikango::resources_descriptor_resource_handle& handle)
    {
        glActiveTexture(GL_TEXTURE0 + texture_pool_itr);
        bind_any_texture(handle);

        descriptors_to_opengl_pools_mapping.insert({{d_id, b_id}, texture_pool_itr});
        texture_pool_itr++;
    };

    auto bind_uniform = [&](const pikango::resources_descriptor_resource_handle& handle)
    {
        auto ub_handle = std::get<pikango::uniform_buffer_handle>(handle);
        auto ubi = pikango_internal::object_read_access(ub_handle);

        glBindBufferBase(GL_UNIFORM_BUFFER, uniform_pool_itr, ubi->id);

        descriptors_to_opengl_pools_mapping.insert({{d_id, b_id}, uniform_pool_itr});
        uniform_pool_itr++;
    };

    auto bind_storage = [&](const pikango::resources_descriptor_resource_handle& handle)
    {

    };

    for (auto& res_desc : binded_resources_descriptors)
    {
        b_id = 0;

        if (pikango_internal::is_empty(res_desc)) {d_id++; continue;};
        auto rdi = pikango_internal::object_read_access(res_desc);

        for (; b_id < rdi->layout.size(); b_id++)
        {
            switch (rdi->layout[b_id])
            {
            case pikango::resources_descriptor_binding_type::sampled_texture:
                bind_texture(rdi->bindings[b_id]); break;
            case pikango::resources_descriptor_binding_type::written_texture:
                bind_texture(rdi->bindings[b_id]); break;
            case pikango::resources_descriptor_binding_type::uniform_buffer:
                bind_uniform(rdi->bindings[b_id]); break;
            case pikango::resources_descriptor_binding_type::storage_buffer:
                bind_storage(rdi->bindings[b_id]); break;
            }
        }

        d_id++;
    }

    //2. set shaders uniforms to point to those resources 
    auto gpi = pikango_internal::object_write_access(binded_graphics_pipeline);
    auto& shaders = gpi->config.shaders_config;

    set_shaders_uniforms(shaders.vertex_shader);
    set_shaders_uniforms(shaders.pixel_shader);
    set_shaders_uniforms(shaders.geometry_shader);
}
