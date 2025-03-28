struct pikango_internal::graphics_pipeline_impl
{
    pikango::vertex_layout_pipeline_info vertex_layout;

    GLenum  primitive;

    pikango::graphics_shaders_pipeline_info shaders;

    bool    enable_culling;
    GLenum  polygon_fill;
    GLenum  culling_mode;
    GLenum  culling_front_face;
    float   line_width;

    pikango::depth_stencil_pipeline_info depth_stencil;
};

pikango::graphics_pipeline_handle pikango::new_graphics_pipeline(const graphics_pipeline_create_info& info)
{
    auto impl   = new pikango_internal::graphics_pipeline_impl;

    impl->vertex_layout = info.vertex_layout_info;

    impl->primitive     = get_primitive(info.input_assembly_info.primitive);

    impl->shaders = info.graphics_shaders_info;

    impl->enable_culling     = info.rasterization_info.enable_culling;
    impl->polygon_fill       = get_rasterization_fill(info.rasterization_info.polygon_fill);
    impl->culling_mode       = get_culling_mode(info.rasterization_info.culling_mode);
    impl->culling_front_face = get_front_face(info.rasterization_info.culling_front_face);
    impl->line_width         = info.rasterization_info.line_width;
    impl->depth_stencil      = info.depth_stencil_info;

    auto handle = pikango_internal::make_handle(impl);
    return handle;
};

void pikango::cmd::bind_graphics_pipeline(graphics_pipeline_handle pipeline)
{
    auto func = [](std::vector<std::any>& args)
    {
        auto pipeline = std::any_cast<graphics_pipeline_handle>(args[0]);

        cmd_bindings::graphics_pipeline_changed = true;
        cmd_bindings::graphics_pipeline = pikango_internal::obtain_handle_object(pipeline);
    };

    record_task(func, {pipeline});
}
