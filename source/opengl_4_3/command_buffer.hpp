struct pikango_internal::command_buffer_impl
{
    pikango::queue_type target_queue_type;
    std::vector<recorded_task> tasks;
};

pikango::command_buffer_handle pikango::new_command_buffer(const command_buffer_create_info& info)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::command_buffer_impl);
    return handle;
}

void pikango::configure_command_buffer(command_buffer_handle target, queue_type target_queue_type)
{
    auto cbi = pikango_internal::obtain_handle_object(target);
    cbi->target_queue_type = target_queue_type;
}

void pikango::begin_command_buffer_recording(command_buffer_handle target)
{
    auto cbi = pikango_internal::obtain_handle_object(target);
    cbi->tasks.clear();
    recorded_command_buffer = target;
}

void pikango::end_command_buffer_recording(command_buffer_handle target)
{
    recorded_command_buffer = {};
}
