#pragma once

namespace {
    constexpr int GL_UNSIGNED_INT_size_bytes = 4;
};

void pikango::cmd::draw_vertices(
    draw_primitive  primitive,

    size_t          vertices_count,
    size_t          vertices_buffer_offset_index,

    size_t          instances_count,
    size_t          instances_id_values_offset
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto primitive = std::any_cast<draw_primitive>(args[0]);

        auto vertices_count = std::any_cast<size_t>(args[1]);
        auto vertices_buffer_offset_index = std::any_cast<size_t>(args[2]);

        auto instances_count = std::any_cast<size_t>(args[3]);
        auto instances_id_values_offset = std::any_cast<size_t>(args[4]);

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

    size_t          indices_count,
    size_t          indicies_buffer_offset,
    int32_t         indicies_values_offset,

    size_t          instances_count,
    size_t          instances_id_values_offset
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto primitive = std::any_cast<draw_primitive>(args[0]);

        auto indices_count = std::any_cast<size_t>(args[1]);
        auto indicies_buffer_offset = std::any_cast<size_t>(args[2]);
        auto indicies_values_offset = std::any_cast<int32_t>(args[3]);

        auto instances_count = std::any_cast<size_t>(args[4]);
        auto instances_id_values_offset = std::any_cast<size_t>(args[5]);

        apply_bindings();

        glDrawElementsInstancedBaseVertexBaseInstance(
            get_primitive(primitive),
            indices_count,
            GL_UNSIGNED_INT,
            (void*)(indicies_buffer_offset * GL_UNSIGNED_INT_size_bytes),
            instances_count,
            indicies_values_offset,
            instances_id_values_offset
        );
    };

    record_task(func, {
        primitive, 
        indices_count, 
        indicies_buffer_offset, 
        indicies_values_offset,
        instances_count,
        instances_id_values_offset
    });
}
