#pragma once

namespace {
    pikango::rasterization_pipeline_config  recent_rasterization_config;
    pikango::depth_stencil_pipeline_config  recent_depth_stencil_config;
};

void apply_vertex_layout()
{
    for (int i = 0; i < 16; i++)
        glDisableVertexAttribArray(i);

    auto& vlc = cmd_bindings::graphics_pipeline->config.vertex_layout_config;

    for (auto& attrib : vlc.attributes)
    {
        auto& buffer = cmd_bindings::vertex_buffers.at(attrib.binding);

        glBindBuffer(
            GL_ARRAY_BUFFER, 
            buffer
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
    GLuint program_pipeline = get_program_pipeline(cmd_bindings::graphics_pipeline->config.shaders_config);
    glUseProgram(0);
    glBindProgramPipeline(program_pipeline);
}

void apply_graphics_pipeline_settings()
{
    auto gpi = cmd_bindings::graphics_pipeline;

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

//we wait with actual binding until the draw because of openGl desing the bindings
//could be overriden by other non-related stuff like writing to buffer
static void apply_bindings()
{
    if (cmd_bindings::vertex_buffers_changed)
        apply_vertex_layout();
    cmd_bindings::vertex_buffers_changed = false;

    if (cmd_bindings::index_buffer_changed)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cmd_bindings::index_buffer);
    cmd_bindings::index_buffer_changed = false;

    if (cmd_bindings::graphics_pipeline_changed) apply_graphics_pipeline_settings();
    if (cmd_bindings::frame_buffer_changed)      glBindFramebuffer(GL_FRAMEBUFFER, cmd_bindings::frame_buffer);

    //Always bind the program pipeline because it could have been
    //overwritten by shaders functions
    apply_graphics_pipeline_shaders();
}
