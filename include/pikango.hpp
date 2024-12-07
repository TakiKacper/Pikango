#include <vector>
#include <atomic>
#include <string>

/*
    Enumerations
*/

namespace pikango
{
    enum class draw_primitive
    {
        points, 
        lines, lines_loop, line_strip,
        traingles, traingles_strip
    };

    enum class data_types
    {
        int32,
        float32,
        vec2f32,
        vec3f32,
        vec4f32
    };
}

/*
    Handles
*/

namespace pikango_internal
{
    template<class handled_object>
    class handle
    {
    private:
        struct meta_block
        {
            std::atomic<uint64_t> refs;
        };

        handled_object* object;
        meta_block* meta;

    public:
        handle() : meta(nullptr), object(nullptr) {};
        handle(const handle& other) : 
            meta(other.meta), 
            object(other.object)
        {
            if (meta != nullptr)
                meta->refs++;
        }
        ~handle()
        {
            if (meta != nullptr)
            {
                meta->refs--;
                if (meta->refs == 0)
                {
                    delete object;
                    delete meta;
                }
            }
        }
    };
}

#define PIKANGO_HANDLE(name)     \
    namespace pikango_internal   \
    {                            \
        struct name##_impl;      \
    }                            \
                                 \
    namespace pikango            \
    {                            \
        using name##_handle = pikango_internal::handle<pikango_internal::name##_impl>;    \
        name##_handle new_##name ();            \
        void delete_##name (name##_handle);     \
    }


PIKANGO_HANDLE(vertex_buffer);
PIKANGO_HANDLE(index_buffer);
PIKANGO_HANDLE(instances_buffer);
PIKANGO_HANDLE(uniform_buffer);
PIKANGO_HANDLE(data_layout);
PIKANGO_HANDLE(graphics_shader);
PIKANGO_HANDLE(texture); 
PIKANGO_HANDLE(frame_buffer);

#undef PIKANGO_HANDLE

/*
    Library Functions
*/

namespace pikango
{
    using error_notification_callback = void(*)(const std::string& notification);

    std::string initialize();
    std::string terminate();
    void set_error_notification_callback(error_notification_callback callback);
}

/*
    Buffers Methods
*/

#define BUFFER_METHODS(buffer_name) \
    void write_buffer(buffer_name##_handle target, size_t data_size_bytes, void* data);   \
    void overwrite_buffer(buffer_name##_handle target, size_t data_size_bytes, void* data);   \
    void overwrite_buffer_region(buffer_name##_handle target, size_t data_size_bytes, void* data, size_t data_offset_bytes);   \

namespace pikango
{
    BUFFER_METHODS(vertex_buffer);
    BUFFER_METHODS(index_buffer);
    BUFFER_METHODS(instances_buffer);
    BUFFER_METHODS(uniform_buffer);
}

#undef BUFFER_METHODS

/*
    Data Layout Methods
*/

namespace pikango
{
    void assign_data_layout(data_layout_handle target, std::vector<data_types> layout, size_t layouts_stride);
}

/*
    Shaders Methods
*/

namespace pikango
{
    void link_vertex_shader  (graphics_shader_handle target, const std::string& shader);
    void link_geometry_shader(graphics_shader_handle target, const std::string& shader);
    void link_pixel_shader   (graphics_shader_handle target, const std::string& shader);
}

/*
    Textures Methods
*/


/*
    Frame Buffer Methods
*/


/*
    Drawing
*/

namespace pikango
{
    struct draw_vertices_args
    {
        draw_primitive primitive = draw_primitive::traingles;
        size_t first_vertex_index = 0;
        size_t primitives_count = 0;

        frame_buffer_handle     target;

        vertex_buffer_handle    vertex_buffer;
        graphics_shader_handle  graphics_shader;
        data_layout_handle      data_layout;
        
    };

    void draw_vertices(const draw_vertices_args& args);
}

#ifdef PIKANGO_IMPLEMENTATION

namespace pikango_internal
{
    pikango::error_notification_callback error_callback = nullptr;
}

void pikango::set_error_notification_callback(error_notification_callback callback)
{
    pikango_internal::error_callback = callback;
}

#endif