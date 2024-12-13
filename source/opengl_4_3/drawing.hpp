#pragma once

void bind_vertex_layout(const pikango::data_layout_handle& handle)
{
    auto dli = pikango_internal::object_read_access(handle);

    int i = 0;

    size_t size = pikango::get_layout_size_with_stride(handle);
    size_t offset = 0;

    for (; i < dli->layout.size(); i++)
    {
        auto data_type = dli->layout.at(i);

        auto dt = dli->layout.at(i);

        glVertexAttribPointer(i, get_elements_in_data_type(dt), get_data_type(dt), GL_FALSE, size, (void*)(uintptr_t)(offset));
		glEnableVertexAttribArray(i);
    }

    for (; i < 16; i++)
    {
        glDisableVertexAttribArray(i);
    }
}

void pikango::draw_vertices(draw_vertices_args& args)
{
    auto func = [](std::vector<std::any> args)
    {
        auto draw_args = std::any_cast<pikango::draw_vertices_args>(args.front());

        //bind vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, pikango_internal::object_read_access(draw_args.vertex_buffer)->id);
        
        //bind layout
        bind_vertex_layout(draw_args.data_layout);

        //bind shader
        glUseProgram(pikango_internal::object_read_access(draw_args.graphics_shader)->id);

        glDrawArrays(
            get_primitive(draw_args.primitive), 
            draw_args.first_vertex_index, 
            draw_args.vertices_count
        );
    };

    enqueue_task(func, {args});
}

void pikango::draw_indexed(draw_indexed_args& args)
{
    auto func = [](std::vector<std::any> args)
    {
        auto draw_args = std::any_cast<pikango::draw_indexed_args>(args.front());

        //bind vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, pikango_internal::object_read_access(draw_args.vertex_buffer)->id);

        //bind layout
        bind_vertex_layout(draw_args.data_layout);
        
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

    enqueue_task(func, {args});
}
