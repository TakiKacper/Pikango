PIKANGO_IMPL(graphics_pipeline)
{
    pikango::graphics_pipeline_config config;
};

PIKANGO_NEW(graphics_pipeline)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::graphics_pipeline_impl);
    return handle;
};

void pikango::configure_graphics_pipeline(graphics_pipeline_handle target, graphics_pipeline_config& configuration)
{
    auto gpi = pikango_internal::object_write_access(target);
    gpi->config = configuration;
}
