#pragma once

void pikango::cmd::set_viewport(const rectangle& rect)
{
    auto func = [](std::vector<std::any> args)
    {
        auto rect = std::any_cast<rectangle>(args[0]);
        glViewport(rect.ax, rect.ay, rect.bx - rect.ax, rect.by - rect.ay);
    };

    record_task(func, {rect});
}

void pikango::cmd::set_scissors(const rectangle& rect)
{
    auto func = [](std::vector<std::any> args)
    {
        auto rect = std::any_cast<rectangle>(args[0]);
        glScissor(rect.ax, rect.ay, rect.bx - rect.ax, rect.by - rect.ay);
    };

    record_task(func, {rect});
}

void pikango::cmd::clear_render_space_color(float r, float g, float b, float a)
{
    auto func = [](std::vector<std::any> args)
    {
        auto r = std::any_cast<float>(args[0]);
        auto g = std::any_cast<float>(args[1]);
        auto b = std::any_cast<float>(args[2]);
        auto a = std::any_cast<float>(args[3]);

        glClearColor(r, g, b, a); 
        glClear(GL_COLOR_BUFFER_BIT);
    };

    record_task(func, {r, g, b, a});
}

void pikango::cmd::clear_render_space_depth(float d)
{
    auto func = [](std::vector<std::any> args)
    {
        auto d = std::any_cast<float>(args[0]);

        glClearDepth(d);
        glClear(GL_DEPTH_BUFFER_BIT);
    };

    record_task(func, {d});
}

void pikango::cmd::clear_render_space_stencil(int s)
{
    auto func = [](std::vector<std::any> args)
    {
        auto s = std::any_cast<int>(args[0]);

        glClearStencil(s);
        glClear(GL_STENCIL_BUFFER_BIT);
    };

    record_task(func, {s});
}
