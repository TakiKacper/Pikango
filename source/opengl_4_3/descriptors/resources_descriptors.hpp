#pragma once

PIKANGO_IMPL(resources_descriptor)
{
    bool layout_set = false;
    std::vector<pikango::resources_descriptor_binding_type> layout;
    std::vector<pikango::resources_descriptor_resource_handle> bindings;
};

PIKANGO_NEW(resources_descriptor)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::resources_descriptor_impl);
    return handle;
};

void pikango::configure_resources_descriptor(resources_descriptor_handle target, std::vector<resources_descriptor_binding_type>& layout)
{
    auto rdi = pikango_internal::object_write_access(target);
    
    if (rdi->layout_set)
    {
        pikango_internal::log_error("Resources descriptor layout already set");
        return;
    }

    rdi->layout = layout;
    rdi->layout_set = true;
}

void pikango::bind_to_resources_descriptor(resources_descriptor_handle target, std::vector<resources_descriptor_resource_handle>& bindings)
{
    auto rdi = pikango_internal::object_write_access(target);

    if (!rdi->layout_set)
    {
        pikango_internal::log_error("Binding resources to resources descriptor before seting it's layout");
        return;
    }

    rdi->bindings = bindings;
}
