struct pikango_internal::fence_impl
{
    bool subbmitted = false;
    bool is_signaled = false;
    std::mutex mutex;
    std::condition_variable condition;
};

pikango::fence_handle pikango::new_fence(const fence_create_info& info)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::fence_impl);
    return handle;
}
