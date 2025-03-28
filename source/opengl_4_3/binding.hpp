#pragma once

void apply_vertex_layout()
{
    for (int i = 0; i < 16; i++)
        glDisableVertexAttribArray(i);

    auto& vlc = cmd_bindings::graphics_pipeline->vertex_layout;

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
    GLuint program_pipeline = get_program_pipeline(cmd_bindings::graphics_pipeline->shaders);
    glUseProgram(0);
    glBindProgramPipeline(program_pipeline);
}

void apply_graphics_pipeline_settings()
{
    auto gpi = cmd_bindings::graphics_pipeline;

    if (gpi->enable_culling) glEnable(GL_CULL_FACE);
    else                     glDisable(GL_CULL_FACE);

    glPolygonMode(GL_FRONT_AND_BACK, gpi->polygon_fill);
    glCullFace(gpi->culling_mode);
    glFrontFace(gpi->culling_front_face);
    glLineWidth(gpi->line_width);

    //Apply depth stencil settings
    auto& ds = gpi->depth_stencil;

    if (ds.enable_depth_test) glEnable(GL_DEPTH_TEST);
    else                      glDisable(GL_DEPTH_TEST);

    if (ds.enable_depth_write) glDepthMask(true);
    else                       glDepthMask(false);
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
