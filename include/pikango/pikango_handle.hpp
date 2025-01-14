#pragma once
#include <atomic>

namespace pikango_internal
{
    template<class T>
    void implementations_destructor(T*);

    template<class handled_object>
    class handle
    {
        template<class T> friend bool       is_empty(const handle<T>& handle);
        template<class T> friend size_t     handle_hash(const handle<T>& handle);
        template<class T> friend handle<T>  make_handle (T* object);
        template<class T> friend T*         obtain_handle_object(const handle<T>& handle);
        
    private:
        struct meta_block;

        handled_object* object;
        meta_block* meta;

        handle(handled_object* _object) : object(_object)
        {
            meta = new meta_block;
            meta->refs = 1;
        }

    public:
        handle() : meta(nullptr), object(nullptr) {};
        handle(const handle& other);
        ~handle();
        
        void operator=(const handle& other);
        bool operator==(const handle<handled_object>& other) const;
    };

    template<class handled_object>
    struct handle<handled_object>::meta_block
    {
        std::atomic<uint64_t> refs;
    };

    template<class handled_object>
    handle<handled_object>::handle(const handle& other): 
        meta(other.meta), 
        object(other.object)
    {
        if (meta != nullptr)
            meta->refs++;
    }
    
    //this function exist only so the implementation could use meta addresses as hashes
    //do not use it in any other way
    template<class T> void* get_handle_meta_block_address(const handle<T>& handle)
    {
        return handle.meta;
    }

    template<class T> 
    T* get_handle_object_raw(const handle<T>& handle)
    {
        return handle.object;
    }

    template<class handled_object>
    handle<handled_object>::~handle()
    {
        if (meta != nullptr)
        {
            meta->refs--;
            if (meta->refs == 0)
            {
                implementations_destructor(object);
                delete meta;
            }
        }

        meta = nullptr;
        object = nullptr;
    }
    
    template<class handled_object>
    void handle<handled_object>::operator=(const handle& other)
    {
        this->~handle();

        object = other.object;
        meta = other.meta;

        if (meta != nullptr)
            meta->refs++;
    }

    template <class handled_object>
    bool handle<handled_object>::operator==(const handle<handled_object>& other) const
    {
        return meta == other.meta;
    };

    template <class handled_object>
    bool is_empty(const handle<handled_object>& handle)
    {
        return handle.meta == nullptr;
    }

    template <class handled_object>
    size_t handle_hash(const handle<handled_object>& handle)
    {
        //since pointer is the same size as size_t
        //return the meta pointer as a hash of handle
        return (size_t)handle.meta;
    }

    template<class handled_object>
    handle<handled_object> make_handle(handled_object* object)
    {
        return handle<handled_object>{object};
    }

    template<class handled_object> 
    handled_object* obtain_handle_object(const handle<handled_object>& handle)
    {
        return handle.object;
    }
}
