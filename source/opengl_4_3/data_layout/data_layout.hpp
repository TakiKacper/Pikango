/*
    Implementation
*/

PIKANGO_IMPL(data_layout)
{
    std::vector<pikango::data_types> layout{};
    size_t layouts_stride = 0;
};

PIKANGO_NEW(data_layout)
{
    return pikango_internal::make_handle(new pikango_internal::data_layout_impl);
};

PIKANGO_DELETE(data_layout)
{

};

/*
    Methods
*/

size_t pikango::get_layout_size(data_layout_handle target)
{
    auto dli = pikango_internal::object_read_access(target);

    size_t size = 0;
    for (auto& e : dli->layout)
        size += size_of(e);
    
    return size;
}

size_t pikango::get_layout_size_with_stride(data_layout_handle target)
{
    auto dli = pikango_internal::object_read_access(target);

    size_t size = 0;
    for (auto& e : dli->layout)
        size += size_of(e);

    size += dli->layouts_stride;
    
    return size;
}

void pikango::assign_data_layout(data_layout_handle target, std::vector<data_types> layout, size_t layouts_stride)
{
    auto dli = pikango_internal::object_write_access(target);
    dli->layout = std::move(layout);
    dli->layouts_stride = layouts_stride;
}
