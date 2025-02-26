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