#pragma once

namespace {
    pikango::graphics_pipeline_handle       binded_graphics_pipeline;
    bool graphics_pipeline_changed = false;

    pikango::rasterization_pipeline_config  recent_rasterization_config;
    pikango::depth_stencil_pipeline_config  recent_depth_stencil_config;

    std::array<pikango::buffer_handle, 16>  binded_vertex_buffers;
    bool vao_bindings_changed = false;

    pikango::buffer_handle                  binded_index_buffer;
    bool ibo_bindings_changed = false;

    bool using_default_fbo = true;
    bool proper_fbo_binded = true;
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

void pikango::cmd::bind_vertex_buffer(buffer_handle vertex_buffer, size_t binding)
{
    auto func = [](std::vector<std::any> args)
    {
        auto vertex_buffer = std::any_cast<buffer_handle>(args[0]);
        auto binding       = std::any_cast<size_t>(args[1]);

        if (vertex_buffer == binded_vertex_buffers.at(binding)) return;

        binded_vertex_buffers.at(binding) = vertex_buffer;
        vao_bindings_changed = true;
    };

    record_task(func, {vertex_buffer, binding});
}

void pikango::cmd::bind_index_buffer(buffer_handle index_buffer)
{
    auto func = [](std::vector<std::any> args)
    {
        auto index_buffer = std::any_cast<buffer_handle>(args[0]);

        if (index_buffer == binded_index_buffer) return;

        binded_index_buffer = index_buffer;
        ibo_bindings_changed = true;
    };

    record_task(func, {index_buffer});
}

void pikango::cmd::bind_color_render_target(texture_buffer_handle color, size_t slot)
{
    auto func = [](std::vector<std::any> args)
    {
        auto attachment = std::any_cast<texture_buffer_handle>(args[0]);
        auto slot       = std::any_cast<size_t>(args[1]); 

        auto tbi = pikango_internal::obtain_handle_object(attachment);

        if (using_default_fbo && proper_fbo_binded)
            glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        using_default_fbo = false;
        proper_fbo_binded = true;

        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0 + slot, 
            GL_TEXTURE_2D, 
            tbi->id, 
            0
        );
    };

    record_task(func, {color, slot});
}

void pikango::cmd::bind_depth_render_target(texture_buffer_handle depth)
{
    auto func = [](std::vector<std::any> args)
    {
        auto attachment = std::any_cast<texture_buffer_handle>(args[0]);

        auto tbi = pikango_internal::obtain_handle_object(attachment);

        if (using_default_fbo && proper_fbo_binded)
            glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        using_default_fbo = false;
        proper_fbo_binded = true;

        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_DEPTH_ATTACHMENT,
            GL_TEXTURE_2D, 
            tbi->id, 
            0
        );
    };

    record_task(func, {depth});
}

void pikango::cmd::bind_stencil_render_target(texture_buffer_handle stencil)
{
    auto func = [](std::vector<std::any> args)
    {
        auto attachment = std::any_cast<texture_buffer_handle>(args[0]);

        auto tbi = pikango_internal::obtain_handle_object(attachment);

        if (using_default_fbo && proper_fbo_binded)
            glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        using_default_fbo = false;
        proper_fbo_binded = true;

        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_STENCIL_ATTACHMENT,
            GL_TEXTURE_2D, 
            tbi->id, 
            0
        );
    };

    record_task(func, {stencil});
}

void pikango::cmd::OPENGL_ONLY_bind_default_render_target()
{
    auto func = [](std::vector<std::any> args)
    {
        using_default_fbo = true;
        proper_fbo_binded = false;
    };

    record_task(func, {});
}

void pikango::cmd::bind_texture(
    texture_sampler_handle sampler,
    texture_buffer_handle buffer,
    size_t slot        
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto texture_sampler = std::any_cast<texture_sampler_handle>(args[0]);
        auto texture_buffer  = std::any_cast<texture_buffer_handle>(args[1]);
        auto slot   = std::any_cast<size_t>(args[2]);

        auto tsi = pikango_internal::obtain_handle_object(texture_sampler);
        auto tbi = pikango_internal::obtain_handle_object(texture_buffer);

        glBindSampler(slot, tsi->id);
        
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(tbi->texture_type, tbi->id);
    };

    record_task(func, {sampler, buffer, slot});
}

void pikango::cmd::bind_uniform_buffer(
    buffer_handle uniform_buffer,
    size_t slot,
    size_t size,
    size_t offset
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto uniform_buffer = std::any_cast<buffer_handle>(args[0]);
        auto slot   = std::any_cast<size_t>(args[1]);
        auto offset = std::any_cast<size_t>(args[2]);
        auto size   = std::any_cast<size_t>(args[3]);

        auto ubi = pikango_internal::obtain_handle_object(uniform_buffer);

        glBindBufferRange(
            GL_UNIFORM_BUFFER, 
            slot, 
            ubi->id, 
            offset, 
            size
        );
    };

    record_task(func, {uniform_buffer, slot, offset, size});
}

static void apply_vertex_layout();
static void apply_graphics_pipeline_shaders();
static void apply_graphics_pipeline_settings();

//we wait with actual binding until the draw because of openGl desing the bindings
//could be overriden by other non-related stuff like writing to unrelated buffer or something
static void apply_bindings()
{
    if (vao_bindings_changed)
        apply_vertex_layout();
    vao_bindings_changed = false;

    if (ibo_bindings_changed)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pikango_internal::obtain_handle_object(binded_index_buffer)->id);
    ibo_bindings_changed = false;

    if (graphics_pipeline_changed)
        apply_graphics_pipeline_settings();

    if (!proper_fbo_binded)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, using_default_fbo ? 0 : FBO);
        proper_fbo_binded = true;
    }

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
    for (int i = 0; i < 16; i++)
        glDisableVertexAttribArray(i);

    auto& vlc = pikango_internal::obtain_handle_object(binded_graphics_pipeline)->config.vertex_layout_config;

    for (auto& attrib : vlc.attributes)
    {
        auto& buffer    = binded_vertex_buffers.at(attrib.binding);
        auto& buffer_id = pikango_internal::obtain_handle_object(buffer)->id;

        glBindBuffer(
            GL_ARRAY_BUFFER, 
            buffer_id
        );

        glEnableVertexAttribArray(attrib.location);

        glVertexAttribPointer(
            attrib.location, 
            get_elements_in_data_type(attrib.type), 
            get_data_type(attrib.type), 
            GL_FALSE, 
            attrib.stride, 
            (void*)(uintptr_t)(attrib.offset)
        );

        glVertexAttribDivisor(
            attrib.location, 
            attrib.per_instance ? 1 : 0
        );
    }
}

void apply_graphics_pipeline_shaders()
{
    //Apply shaders
    auto gpi = pikango_internal::obtain_handle_object(binded_graphics_pipeline);

    GLuint program_pipeline = get_program_pipeline(gpi->config.shaders_config);
    glUseProgram(0);
    glBindProgramPipeline(program_pipeline);
}

void apply_graphics_pipeline_settings()
{
    auto gpi = pikango_internal::obtain_handle_object(binded_graphics_pipeline);

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
