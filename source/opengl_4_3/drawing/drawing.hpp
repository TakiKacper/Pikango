#pragma once

void pikango::cmd::draw_vertices(
    draw_primitive  primitive,
    size_t          first_vertex_index,
    size_t          vertices_count
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto primitive = std::any_cast<draw_primitive>(args[0]);
        auto first_vertex_index = std::any_cast<size_t>(args[1]);
        auto vertices_count = std::any_cast<size_t>(args[2]);

        apply_bindings();

        glDrawArrays(
            get_primitive(primitive),
            first_vertex_index,
            vertices_count
        );
    };

    record_task(func, {primitive, first_vertex_index, vertices_count});
}

void pikango::cmd::draw_vertices_instanced(
    draw_primitive  primitive,
    size_t          first_vertex_index,
    size_t          vertices_count,
    size_t          instances_count
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto primitive = std::any_cast<draw_primitive>(args[0]);
        auto first_vertex_index = std::any_cast<size_t>(args[1]);
        auto vertices_count = std::any_cast<size_t>(args[2]);
        auto instances_count = std::any_cast<size_t>(args[3]);

        apply_bindings();

        glDrawArraysInstanced(
            get_primitive(primitive),
            first_vertex_index,
            vertices_count,
            instances_count
        );
    };

    record_task(func, {primitive, first_vertex_index, vertices_count, instances_count});
}

void pikango::cmd::draw_indexed(
    draw_primitive  primitive,
    size_t          indices_count
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto primitive = std::any_cast<draw_primitive>(args[0]);
        auto indices_count = std::any_cast<size_t>(args[1]);

        apply_bindings();

        glDrawElements(
            get_primitive(primitive),
            indices_count,
            GL_UNSIGNED_INT,
            nullptr
        );
    };

    record_task(func, {primitive, indices_count});
}

void pikango::cmd::draw_indexed_instanced(
    draw_primitive  primitive,
    size_t          indices_count,
    size_t          instances_count
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto primitive = std::any_cast<draw_primitive>(args[0]);
        auto indices_count = std::any_cast<size_t>(args[1]);
        auto instances_count = std::any_cast<size_t>(args[2]);

        apply_bindings();

        glDrawElementsInstanced(
            get_primitive(primitive),
            indices_count,
            GL_UNSIGNED_INT,
            nullptr,
            instances_count
        );
    };

    record_task(func, {primitive, indices_count, instances_count});
}
