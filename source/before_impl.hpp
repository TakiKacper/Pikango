static bool operator==(const pikango::rectangle& a, const pikango::rectangle& b)
{
    return (
        a.ax == b.ax && 
        a.ay == b.ay && 
        a.bx == b.bx && 
        a.by == b.by
    );
}

static bool operator!=(const pikango::rectangle& a, const pikango::rectangle& b)
{
    return !(a == b);
}

size_t pikango::size_of(data_type dt)
{
    switch (dt)
    {
    case data_type::float32: return 4;
    case data_type::int32:   return 4;
    case data_type::vec2f32: return 4 * 2;
    case data_type::vec3f32: return 4 * 3;
    case data_type::vec4f32: return 4 * 4;
    }

    //will never be reached
    return 0;
}
