#pragma once
#ifndef PIKANGO_HPP
#define PIKANGO_HPP

#include <vector>
#include <string>
#include <functional>
#include <any>
#include <variant>

/*
    ENUMERATIONS
*/

namespace pikango
{
    enum class draw_primitive
    {
        points, 
        lines, lines_loop, line_strip,
        traingles, traingles_strip
    };

    enum class buffer_memory_profile
    {
        rare_write_rare_read,   //data is rarely in use
        rare_write_often_read,  //data is often overwritten and often used
        often_write_often_read  //data is rarely overwritten and often used
    };

    enum class buffer_access_profile
    {
        cpu_to_gpu, //application writes data to gpu
        gpu_to_cpu, //gpu data is read by application 
        gpu_to_gpu  //gpu data is read by gpu
    };

    enum class data_type
    {
        int32,
        vec2i32,
        vec3i32,
        vec4i32,

        float32,
        vec2f32,
        vec3f32,
        vec4f32
    };
    size_t size_of(data_type dt);

    enum class texture_filtering
	{
		nearest, linear
	};

    enum class texture_wraping
	{
		repeat, 
        mirror_repeat,
		clamp_coords, 
        clamp_texture
	};

    enum class texture_format
	{
		depth,  depth_stencil,	depth24_stencil8,
		r,		r8, r16,
		rg,		rg8, rg16,
		rgb,	r3_g3_b2, rgb4, rgb5, rgb8, rgb10, rgb12, 
		rgba,	rgba2, rgba4, rgba8, rgba12, rgba16, rgba32f
	};

    enum class rasterization_culling_mode : unsigned char
    {
        none = 0,
        front = 1,
        back = 2,
        front_and_back = 3,
    };

    enum class rasterization_culling_front_face : unsigned char
    {
        couter_clockwise = 0,
        clockwise = 1
    };

    enum class rasterization_polygon_fill_mode : unsigned char
    {
        fill_shape, 
        fill_edges, 
        fill_vertices
    };

    enum class depth_compare_operator : unsigned char
    {
        nerer = 0,
        less = 1,
        equal = 2,
        less_or_equal = 3,
        greater = 4,
        not_equal = 5,
        greater_or_equal = 6,
        always = 7
    };
}

/*
    HANDLES
*/

#include "pikango_handle.hpp"

namespace pikango
{
    template <class handled_object>
    bool is_empty(const pikango_internal::handle<handled_object>& handle)
    {
        return pikango_internal::is_empty(handle);
    }
    
    template <class handled_object>
    size_t handle_hash(const pikango_internal::handle<handled_object>& handle)
    {
        return pikango_internal::handle_hash(handle);
    }
}

/*
This macro for name = xyz would create folowing objects:

    pikango_internal::xyz_impl  (forward)
    
    pikango::xyz_handle         (handle specialization)
    pikango::new_xyz            (function)
*/
#define PIKANGO_HANDLE_FWD(name)     \
    namespace pikango_internal   \
    {                            \
        struct name##_impl;      \
    }                            \
                                 \
    namespace pikango            \
    {                            \
        using name##_handle = pikango_internal::handle<pikango_internal::name##_impl>;    \
        name##_handle new_##name ();            \
    }

PIKANGO_HANDLE_FWD(graphics_pipeline);
//PIKANGO_HANDLE_FWD(compute_pipeline)

PIKANGO_HANDLE_FWD(command_buffer);
PIKANGO_HANDLE_FWD(fence);

PIKANGO_HANDLE_FWD(resources_descriptor);

PIKANGO_HANDLE_FWD(vertex_shader);
PIKANGO_HANDLE_FWD(pixel_shader);
PIKANGO_HANDLE_FWD(geometry_shader);
//PIKANGO_HANDLE_FWD(compute_shader);

PIKANGO_HANDLE_FWD(frame_buffer);

PIKANGO_HANDLE_FWD(buffer);

PIKANGO_HANDLE_FWD(texture_1d);
PIKANGO_HANDLE_FWD(texture_2d);
PIKANGO_HANDLE_FWD(texture_3d);
PIKANGO_HANDLE_FWD(texture_cube);
PIKANGO_HANDLE_FWD(texture_1d_array);
PIKANGO_HANDLE_FWD(texture_2d_array);
//PIKANGO_HANDLE_FWD(renderbuffer);

#undef PIKANGO_HANDLE_FWD

/*
    STRUCTS
*/

namespace pikango
{
    //Struct for representing rectangles
    //it describes them at two points in a coordinate system: a and b
    //a being the lower-left point  (offset)
    //b being the upper-right point (extend)
    //  |----------B
    //  |----------|
    //  A----------|
    struct rectangle
    {
        int ax;
        int ay;

        int bx;
        int by;

        rectangle() : ax(0), ay(0), bx(400), by(400) {};
        rectangle(int _ax, int _ay, int _bx, int _by)
            : ax(_ax), ay(_ay), bx(_bx), by(_by) {};
    };

    struct vertex_layout_pipeline_config
    {
        std::vector<data_type> vertex_attributes;
        std::vector<data_type> instance_attributes;
        size_t                  stride;
    };

    struct graphics_shaders_pipeline_config
    {
        vertex_shader_handle    vertex_shader;
        pixel_shader_handle     pixel_shader;
        geometry_shader_handle  geometry_shader;
    };

    struct rasterization_pipeline_config
    {
        bool enable_culling = false;

        rasterization_polygon_fill_mode   polygon_fill = rasterization_polygon_fill_mode::fill_shape;
        rasterization_culling_mode        culling_mode = rasterization_culling_mode::none;
        rasterization_culling_front_face  culling_front_face = rasterization_culling_front_face::couter_clockwise;

        float line_width = 1.0f;
    };

    struct depth_stencil_pipeline_config
    {
        bool enable_depth_test = false;
        bool enable_depth_write = false;
    };

    struct graphics_pipeline_config
    {
        vertex_layout_pipeline_config     vertex_layout_config;
        graphics_shaders_pipeline_config  shaders_config;
        rasterization_pipeline_config     rasterization_config;
        depth_stencil_pipeline_config     depth_stencil_config;
    };

    enum class resources_descriptor_binding_type
    {
        sampled_texture, written_texture, uniform_buffer, storage_buffer
    };

    using resources_descriptor_resource_handle = std::variant<
        buffer_handle,
        //storage_buffer_handle

        texture_1d_handle,
        texture_2d_handle,
        texture_3d_handle,
        texture_cube_handle
        //texture_1d_array_handle,
        //texture_2d_array_handle
    >;
}

/*
    FUNCTIONS
*/

//Library
namespace pikango
{
    using error_notification_callback = void(*)(const char* notification);

    std::string initialize_library_cpu();
    std::string initialize_library_gpu();
    std::string terminate();

    void set_error_notification_callback(error_notification_callback callback);
}

#ifdef PIKANGO_OPENGL_4_3
namespace pikango
{
    using opengl_thread_task = void(*)(std::vector<std::any>);
    void OPENGL_ONLY_execute_on_context_thread(opengl_thread_task task, std::vector<std::any> args);
}
#endif

//Queues
namespace pikango
{
    enum class queue_type
    {
        general, compute, transfer
    };

    size_t get_queues_max_amount(queue_type type);
    void enable_queues(queue_type type, size_t amount);

    void wait_queue_empty(queue_type type, size_t queue_index);
    void wait_all_queues_empty();
    void submit_command_buffer(command_buffer_handle target, queue_type type, size_t queue_index);
    void submit_command_buffer_with_fence(command_buffer_handle target, queue_type type, size_t queue_index, fence_handle wait_fence);
}

//Getters
namespace pikango
{
    //Shaders
    const char* get_used_shading_language_name();

    //Descriptors
    size_t get_max_resources_descriptors_bindings();

    //Framebuffers
    size_t get_max_framebuffer_color_buffers_attachments();
}

/*
    COMMNANDS AND UTILITY METHODS
*/

//Pipelines
namespace pikango
{
    void configure_graphics_pipeline(graphics_pipeline_handle target, graphics_pipeline_config& config);
}

//Resources Descriptor
namespace pikango
{
    void configure_resources_descriptor(resources_descriptor_handle target, std::vector<resources_descriptor_binding_type>& layout);
    void bind_to_resources_descriptor(resources_descriptor_handle target, std::vector<resources_descriptor_resource_handle>& bindings);
}

//Command Buffer
namespace pikango
{
    void configure_command_buffer(command_buffer_handle target, queue_type target_queue_type);
    void begin_command_buffer_recording(command_buffer_handle target);
    void end_command_buffer_recording(command_buffer_handle target);
    void clear_command_buffer(command_buffer_handle target);
}

//Fences
namespace pikango
{
    void wait_fence(fence_handle target);
    void wait_multiple_fences(std::vector<fence_handle> targets);
}

//Buffer
namespace pikango
{
    size_t get_buffer_size(buffer_handle target);
}

namespace pikango::cmd
{
    void assign_buffer_memory(
        buffer_handle target, 
        size_t memory_block_size_bytes, 
        buffer_memory_profile memory_profile, 
        buffer_access_profile access_profile
    );

    void write_buffer(
        buffer_handle target, 
        size_t data_size_bytes, 
        void* data
    );

    void write_buffer_region(
        buffer_handle target, 
        size_t data_size_bytes, 
        void* data, 
        size_t data_offset_bytes
    );

    void copy_buffer_to_buffer(
        buffer_handle source, 
        buffer_handle destination,
        size_t read_offset, 
        size_t read_size, 
        size_t write_offset
    );
}

//Shaders Compilation
namespace pikango
{
    void compile_vertex_shader   (vertex_shader_handle target, const std::string& source);
    void compile_pixel_shader    (pixel_shader_handle target, const std::string& source);
    void compile_geometry_shader (geometry_shader_handle target, const std::string& source);
    //void compile_compute_shader(compute_shader_handle target, const std::string& source);
}

#ifdef PIKANGO_OPENGL_4_3

namespace pikango
{
    // {binding name, descriptor id, binding id, binding type}
    using OPENGL_ONLY_shader_bindings = std::vector<std::tuple<std::string, size_t, size_t, resources_descriptor_binding_type>>;

    void OPENGL_ONLY_link_shader_bindings_info(vertex_shader_handle target, OPENGL_ONLY_shader_bindings& bindings);
    void OPENGL_ONLY_link_shader_bindings_info(pixel_shader_handle target, OPENGL_ONLY_shader_bindings& bindings);
    void OPENGL_ONLY_link_shader_bindings_info(geometry_shader_handle target, OPENGL_ONLY_shader_bindings& bindings);
    //void OPENGL_ONLY_link_shader_bindings_info(compute_shader_handle target, OPENGL_ONLY_shader_bindings& bindings);
}
#endif

//Textures
namespace pikango
{
    void set_texture_wraping(texture_1d_handle target, texture_wraping x);
    void set_texture_wraping(texture_2d_handle target, texture_wraping x, texture_wraping y);
    void set_texture_wraping(texture_3d_handle target, texture_wraping x, texture_wraping y, texture_wraping z);

    void set_texture_wraping(texture_cube_handle target, texture_wraping x, texture_wraping y, texture_wraping z);

    void set_texture_wraping(texture_1d_array_handle target, texture_wraping x);
    void set_texture_wraping(texture_2d_array_handle target, texture_wraping x, texture_wraping y);


    void set_texture_filtering(
        texture_1d_handle target, 
        texture_filtering magnifying, 
        texture_filtering minifying,
        texture_filtering mipmap
    );

    void set_texture_filtering(
        texture_2d_handle target, 
        texture_filtering magnifying, 
        texture_filtering minifying, 
        texture_filtering mipmap
    );

    void set_texture_filtering(
        texture_3d_handle target, 
        texture_filtering magnifying, 
        texture_filtering minifying, 
        texture_filtering mipmap
    );

    void set_texture_filtering(
        texture_cube_handle target, 
        texture_filtering magnifying, 
        texture_filtering minifying, 
        texture_filtering mipmap
    );

    void set_texture_filtering(
        texture_1d_array_handle target, 
        texture_filtering magnifying, 
        texture_filtering minifying, 
        texture_filtering mipmap
    );

    void set_texture_filtering(
        texture_2d_array_handle target, 
        texture_filtering magnifying, 
        texture_filtering minifying, 
        texture_filtering mipmap
    );
}

namespace pikango::cmd
{
    void write_texture(
        texture_1d_handle target, 
        texture_format source_format, 
        texture_format inner_format, 
        size_t width, 
        void* pixel_data
    );

    void write_texture(
        texture_2d_handle target, 
        texture_format source_format, 
        texture_format inner_format,
        size_t width, 
        size_t height, 
        void* pixel_data
    );

    void write_texture(
        texture_3d_handle target, 
        texture_format source_format, 
        texture_format inner_format, 
        size_t width, 
        size_t height, 
        size_t depth, 
        void* pixel_data
    );

    void write_texture(
        texture_cube_handle target, 
        texture_format source_format, 
        texture_format inner_format, 
        size_t width, 
        void* top, 
        void* bottom, 
        void* left, 
        void* right, 
        void* front,
        void* back
    );

    void write_texture(
        texture_1d_array_handle target,
        texture_format source_format, 
        texture_format inner_format, 
        size_t width, 
        std::vector<void*> pixel_data
    );
    void write_texture(
        texture_2d_array_handle target,
        texture_format source_format, 
        texture_format inner_format, 
        size_t width, 
        size_t height, 
        std::vector<void*> pixel_data
    );
}

//Framebuffers
namespace pikango
{
    void attach_framebuffer_color_buffer(
        frame_buffer_handle target,
        texture_2d_handle attachment,
        unsigned int slot
    );

    void attach_framebuffer_depth_buffer(
        frame_buffer_handle target,
        texture_2d_handle attachment
    );

    void attach_framebuffer_depth_buffer(
        frame_buffer_handle target,
        texture_cube_handle attachment
    );

    void attach_framebuffer_stencil_buffer(
        frame_buffer_handle target,
        texture_2d_handle attachment
    );
    

    void detach_framebuffer_color_buffer(
        frame_buffer_handle target,
        unsigned int slot
    );

    void detach_framebuffer_depth_buffer(frame_buffer_handle target);
    void detach_framebuffer_stencil_buffer(frame_buffer_handle target);


    texture_2d_handle get_framebuffer_color_buffer(
        frame_buffer_handle target,
        unsigned int slot
    );

    std::variant<texture_2d_handle, texture_cube_handle> get_framebuffer_depth_buffer(frame_buffer_handle target);
    texture_2d_handle get_framebuffer_stencil_buffer(frame_buffer_handle target);

#ifdef PIKANGO_OPENGL_4_3
    frame_buffer_handle OPENGL_ONLY_get_default_frame_buffer();
#endif
};

/*
    DRAWING AND STATE
*/

//Binding
namespace pikango::cmd
{
    void bind_graphics_pipeline(graphics_pipeline_handle pipeline);
    void bind_resources_descriptor(resources_descriptor_handle descriptor, size_t descriptor_index);
    
    void bind_frame_buffer(frame_buffer_handle frame_buffer);

    void bind_vertex_buffer(buffer_handle vertex_buffer);
    void bind_index_buffer(buffer_handle index_buffer);
    void bind_instance_buffer(buffer_handle instance_buffer);
}

//Drawing Related Commands
namespace pikango::cmd
{
    void set_viewport(const rectangle& rect);
    void set_scissors(const rectangle& rect);
}

//Drawing
namespace pikango::cmd
{
    void draw_vertices(
        draw_primitive  primitive,

        size_t          vertices_count,
        size_t          vertices_buffer_offset_index,

        size_t          instances_count,
        size_t          instances_id_values_offset
    );

    void draw_indexed(
        draw_primitive  primitive,

        size_t          indices_count,
        size_t          indicies_buffer_offset_index,
        int32_t         indicies_values_offset,

        size_t          instances_count,
        size_t          instances_id_values_offset
    );
}

#endif
