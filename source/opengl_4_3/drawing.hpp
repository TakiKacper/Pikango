#pragma once

namespace {
    constexpr int GL_UNSIGNED_INT_size_bytes = 4;
};

void pikango::cmd::draw_vertices(
    draw_primitive  primitive,

    uint32_t        vertices_count,
    uint32_t        instances_count,

    uint32_t        vertices_buffer_offset_index,
    uint32_t        instances_id_values_offset
)
{
    auto func = [](std::vector<std::any>& args)
    {
        auto primitive = std::any_cast<draw_primitive>(args[0]);

        auto vertices_count                 = std::any_cast<uint32_t>(args[1]);
        auto vertices_buffer_offset_index   = std::any_cast<uint32_t>(args[2]);

        auto instances_count                = std::any_cast<uint32_t>(args[3]);
        auto instances_id_values_offset     = std::any_cast<uint32_t>(args[4]);

        apply_bindings();

        glDrawArraysInstancedBaseInstance(
            get_primitive(primitive),
            vertices_buffer_offset_index,
            vertices_count,
            instances_count,
            instances_id_values_offset
        );
    };

    record_task(func, {
        primitive, 
        vertices_count, 
        vertices_buffer_offset_index,
        instances_count,
        instances_id_values_offset
    });
}

void pikango::cmd::draw_indexed(
    draw_primitive  primitive,

    uint32_t        indices_count,
    uint32_t        instances_count,

    uint32_t        indicies_buffer_offset_index,
    int32_t         indicies_values_offset,
    uint32_t        instances_id_values_offset
)
{
    auto func = [](std::vector<std::any>& args)
    {
        auto primitive = std::any_cast<draw_primitive>(args[0]);

        auto indices_count              = std::any_cast<uint32_t>(args[1]);
        auto indicies_buffer_offset     = std::any_cast<uint32_t>(args[2]);
        auto indicies_values_offset     = std::any_cast<int32_t>(args[3]);

        auto instances_count            = std::any_cast<uint32_t>(args[4]);
        auto instances_id_values_offset = std::any_cast<uint32_t>(args[5]);

        apply_bindings();

        size_t size = (indicies_buffer_offset * GL_UNSIGNED_INT_size_bytes);

        glDrawElementsInstancedBaseVertexBaseInstance(
            get_primitive(primitive),
            indices_count,
            GL_UNSIGNED_INT,
            (void*)size,
            instances_count,
            indicies_values_offset,
            instances_id_values_offset
        );
    };

    record_task(func, {
        primitive, 
        indices_count, 
        indicies_buffer_offset_index, 
        indicies_values_offset,
        instances_count,
        instances_id_values_offset
    });
}

void pikango::cmd::draw_vertieces_indirect(
    buffer_handle   indirect_buffer,
    uint32_t        draw_count,
    uint32_t        draw_params_buffer_offset,
    uint32_t        draw_params_stride
)
{
    auto func = [](std::vector<std::any>& args)
    {
        auto indirect_buffer            = std::any_cast<pikango::buffer_handle>(args[0]);
        auto draw_count                 = std::any_cast<uint32_t>(args[1]);
        auto draw_params_buffer_offset  = std::any_cast<uint32_t>(args[2]);
        auto draw_params_stride         = std::any_cast<uint32_t>(args[3]);

        auto bi = pikango_internal::obtain_handle_object(indirect_buffer);

        apply_bindings();

        size_t offset = draw_params_buffer_offset;

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, bi->id);
        glMultiDrawArraysIndirect(
            GL_TRIANGLE_STRIP,
            (void*)offset, 
            draw_count, 
            draw_params_stride
        );
    };

    record_task(func, {
        indirect_buffer,
        draw_count,
        draw_params_buffer_offset,
        draw_params_stride
    });
}

void pikango::cmd::draw_indexed_indirect(
    buffer_handle   indirect_buffer,
    uint32_t        draw_count,
    uint32_t        draw_params_buffer_offset,
    uint32_t        draw_params_stride
)
{
    auto func = [](std::vector<std::any>& args)
    {
        auto indirect_buffer            = std::any_cast<pikango::buffer_handle>(args[0]);
        auto draw_count                 = std::any_cast<uint32_t>(args[1]);
        auto draw_params_buffer_offset  = std::any_cast<uint32_t>(args[2]);
        auto draw_params_stride         = std::any_cast<uint32_t>(args[3]);

        auto bi = pikango_internal::obtain_handle_object(indirect_buffer);

        apply_bindings();

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, bi->id);
        glMultiDrawElementsIndirect(
            GL_TRIANGLES,
            GL_UNSIGNED_INT,
            (void*)(size_t)(draw_params_buffer_offset),
            draw_count,
            draw_params_stride
        );
    };

    record_task(func, {
        indirect_buffer,
        draw_count,
        draw_params_buffer_offset,
        draw_params_stride
    });
}
