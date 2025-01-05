#pragma once
#include <set>

namespace {
    pikango::graphics_pipeline_handle   binded_graphics_pipeline;
    bool graphics_pipeline_changed = true;

    pikango::vertex_buffer_handle       binded_vertex_buffer;
    pikango::instance_buffer_handle     binded_instance_buffer;
    bool vao_bindings_changed = true;

    pikango::frame_buffer_handle        binded_frame_buffer;
    bool fbo_bindings_changed = true;

    pikango::index_buffer_handle        binded_index_buffer;
    bool ibo_bindings_changed = true;

    std::array<pikango::resources_descriptor_handle, max_resources_descriptors> binded_resources_descriptors;
    std::set<size_t> changed_resources_descriptors;
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
static void apply_resources_descriptors();

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

    if (changed_resources_descriptors.size() != 0)
        apply_resources_descriptors();
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
}

void apply_resources_descriptors()
{
    auto gpi = pikango_internal::object_write_access(binded_graphics_pipeline);
    auto& shaders = gpi->config.shaders_config;

    auto set_shader_uniforms = [](shader_uniforms_to_descriptors_maping& desc_mapping)
    {
        for (auto& d_id : changed_resources_descriptors)
        {
            auto descriptor = pikango_internal::object_read_access(binded_resources_descriptors.at(d_id));
            auto& desc_bindings = descriptor->bindings;
            auto& desc_layout = descriptor->layout;

            for (size_t b_id = 0; b_id < desc_bindings.size(); b_id++)
            {
                auto itr = desc_mapping.find({d_id, b_id});
                if (itr == desc_mapping.end()) continue;
                auto addr = itr->second;

                auto& type = desc_layout.at(d_id);
                auto& binding = desc_bindings.at(d_id);

                switch (type)
                {
                case pikango::resources_descriptor_binding_type::sampled_texture:
                    glActiveTexture(GL_TEXTURE0 + addr);

                    if (std::holds_alternative<pikango::texture_2d_handle>(binding))
                    {
                        auto txt = std::get<pikango::texture_2d_handle>(binding);
                        glBindTexture(GL_TEXTURE_2D, pikango_internal::get_handle_object_raw(txt)->id);
                    }     

                    break;
                }
            };
        }
    };

    auto vsi = pikango_internal::object_write_access(shaders.vertex_shader);
    set_shader_uniforms(vsi->desc_mapping);

    auto psi = pikango_internal::object_write_access(shaders.pixel_shader);
    set_shader_uniforms(psi->desc_mapping);

    if (!pikango_internal::is_empty(shaders.geometry_shader)) 
    {
        auto gsi = pikango_internal::object_write_access(shaders.geometry_shader);
        set_shader_uniforms(gsi->desc_mapping);
    }
}
