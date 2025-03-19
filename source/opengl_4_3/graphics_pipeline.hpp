struct pikango_internal::graphics_pipeline_impl
{
    pikango::graphics_pipeline_create_info info;
};

pikango::graphics_pipeline_handle pikango::new_graphics_pipeline(const graphics_pipeline_create_info& info)
{
    auto impl   = new pikango_internal::graphics_pipeline_impl;
    impl->info  = info;

    auto handle = pikango_internal::make_handle(impl);
    return handle;
};

void pikango::cmd::bind_graphics_pipeline(graphics_pipeline_handle pipeline)
{
    auto func = [](std::vector<std::any> args)
    {
        auto pipeline = std::any_cast<graphics_pipeline_handle>(args[0]);

        cmd_bindings::graphics_pipeline_changed = true;
        cmd_bindings::graphics_pipeline = pikango_internal::obtain_handle_object(pipeline);
    };

    record_task(func, {pipeline});
}
