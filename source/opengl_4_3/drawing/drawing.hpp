#pragma once

void bind_draw_target(const pikango::draw_target_args& dta)
{
    auto fbi = pikango_internal::object_read_access(dta.frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, fbi->id);
}

void bind_vertex_layout(const pikango::data_layout_handle& layout, const pikango::vertex_buffer_handle& vertex)
{
    auto dli = pikango_internal::object_read_access(layout);
    auto vbi = pikango_internal::object_read_access(vertex);

    int i = 0;

    size_t size = pikango::get_layout_size_with_stride(layout);
    size_t offset = 0;

    glBindBuffer(GL_ARRAY_BUFFER, vbi->id);

    for (; i < dli->layout.size(); i++)
    {
        auto dt = dli->layout.at(i);

        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, get_elements_in_data_type(dt), get_data_type(dt), GL_FALSE, size, (void*)(uintptr_t)(offset));
        glVertexAttribDivisor(i, 0);

        offset += pikango::size_of(dt);
    }

    for (; i < 16; i++)
        glDisableVertexAttribArray(i);
}

void bind_vertex_and_instance_layout(
    const pikango::data_layout_handle& vertex, 
    const pikango::data_layout_handle& instance,
    const pikango::vertex_buffer_handle& vertex_buffer,
    const pikango::instance_buffer_handle& instance_buffer
)
{
    int i = 0;

    auto bind_layout = [&](const pikango::data_layout_handle& dlh, bool instance)
    {
        size_t offset = 0;
        size_t size = pikango::get_layout_size_with_stride(dlh);
        
        auto dli = pikango_internal::object_read_access(dlh);

        GLuint target_buffer_id;
        if (instance) target_buffer_id = pikango_internal::object_read_access(instance_buffer)->id;
        else          target_buffer_id = pikango_internal::object_read_access(vertex_buffer)->id;

        glBindBuffer(GL_ARRAY_BUFFER, target_buffer_id);

        for (int j = 0; j < dli->layout.size(); j++)
        {
            auto dt = dli->layout.at(j);

            glEnableVertexAttribArray(i + j);
            glVertexAttribPointer(i + j, get_elements_in_data_type(dt), get_data_type(dt), GL_FALSE, size, (void*)(uintptr_t)(offset));
            glVertexAttribDivisor(i + j, instance ? 1 : 0);

            offset += pikango::size_of(dt);
        }

        i += dli->layout.size();
    };

    bind_layout(vertex, false);
    bind_layout(instance, true);

    for (; i < 16; i++)
        glDisableVertexAttribArray(i);
}

void pikango::draw_vertices(draw_target_args& dta, draw_vertices_args& dva)
{
    auto func = [](std::vector<std::any> args)
    {
        auto targ_args = std::any_cast<pikango::draw_target_args>(args[0]);
        auto draw_args = std::any_cast<pikango::draw_vertices_args>(args[1]);

        //bind framebuffer
        bind_draw_target(targ_args);

        //bind layout
        bind_vertex_layout(draw_args.vertex_layout, draw_args.vertex_buffer);

        //bind shader
        glUseProgram(pikango_internal::object_read_access(draw_args.graphics_shader)->id);

        glDrawArrays(
            get_primitive(draw_args.primitive), 
            draw_args.first_vertex_index, 
            draw_args.vertices_count
        );
    };

    record_task(func, {dta, dva});
}

void pikango::draw_indexed(draw_target_args& dta, draw_indexed_args& dia)
{
    auto func = [](std::vector<std::any> args)
    {
        auto targ_args = std::any_cast<pikango::draw_target_args>(args[0]);
        auto draw_args = std::any_cast<pikango::draw_indexed_args>(args[1]);

        //bind framebuffer
        bind_draw_target(targ_args);

        //bind layout
        bind_vertex_layout(draw_args.vertex_layout, draw_args.vertex_buffer);
        
        //enable index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pikango_internal::object_read_access(draw_args.index_buffer)->id);

        //bind shader
        glUseProgram(pikango_internal::object_read_access(draw_args.graphics_shader)->id);

        glDrawElements(
            get_primitive(draw_args.primitive), 
            draw_args.indicies_count, 
            GL_UNSIGNED_INT, 
            0
        );
    };

    record_task(func, {dta, dia});
}

void pikango::draw_vertices_instanced(draw_target_args& dta, draw_vertices_args& dva, draw_instanced_args dia)
{
    auto func = [](std::vector<std::any> args)
    {
        auto targ_args = std::any_cast<pikango::draw_target_args>(args[0]);
        auto draw_args = std::any_cast<pikango::draw_vertices_args>(args[1]);
        auto inst_args = std::any_cast<pikango::draw_instanced_args>(args[2]);

        //bind framebuffer
        bind_draw_target(targ_args);

        //bind layout
        bind_vertex_and_instance_layout(
            draw_args.vertex_layout, 
            inst_args.instance_layout, 
            draw_args.vertex_buffer, 
            inst_args.instance_buffer
        );

        //bind shader
        glUseProgram(pikango_internal::object_read_access(draw_args.graphics_shader)->id);

        glDrawArraysInstanced(
            get_primitive(draw_args.primitive), 
            draw_args.first_vertex_index, 
            draw_args.vertices_count,
            inst_args.instances_count
        );
    };

    record_task(func, {dta, dva, dia});
}

void pikango::draw_indexed_instanced(draw_target_args& dta, draw_indexed_args& dia, draw_instanced_args dia2)
{
    auto func = [](std::vector<std::any> args)
    {
        auto targ_args = std::any_cast<pikango::draw_target_args>(args[0]);
        auto draw_args = std::any_cast<pikango::draw_indexed_args>(args[1]);
        auto inst_args = std::any_cast<pikango::draw_instanced_args>(args[2]);

        //bind framebuffer
        bind_draw_target(targ_args);

        //bind layout
        bind_vertex_and_instance_layout(
            draw_args.vertex_layout, 
            inst_args.instance_layout, 
            draw_args.vertex_buffer, 
            inst_args.instance_buffer
        );
        
        //enable index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pikango_internal::object_read_access(draw_args.index_buffer)->id);

        //bind shader
        glUseProgram(pikango_internal::object_read_access(draw_args.graphics_shader)->id);

        glDrawElementsInstanced(
            get_primitive(draw_args.primitive), 
            draw_args.indicies_count, 
            GL_UNSIGNED_INT, 
            0,
            inst_args.instances_count
        );
    };

    record_task(func, {dta, dia, dia2});
}
