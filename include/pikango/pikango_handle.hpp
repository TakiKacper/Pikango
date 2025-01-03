#pragma once
#include <atomic>
#include <shared_mutex>

namespace pikango_internal
{
    template<class T>
    void implementations_destructor(T*);

    template<class handled_object>
    class handle
    {
        template<class T> friend bool       is_empty(const handle<T>& handle);
        template<class T> friend handle<T>  make_handle (T* object);
        template<class T> friend handle<T>* alloc_handle(T* object);
        template<class T> friend struct object_read_access;
        template<class T> friend struct object_write_access;
        
    private:
        struct meta_block
        {
            bool valid = true;
            std::atomic<uint64_t> refs;
            std::shared_mutex mutex{};
        };

        handled_object* object;
        meta_block* meta;

        handle(handled_object* _object) : object(_object)
        {
            meta = new meta_block;
            meta->refs = 1;
            meta->valid = true;
        }

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
                    implementations_destructor(object);
                    delete meta;
                }
            }
        }
        void operator=(const handle& other)
        {
            this->~handle();

            object = other.object;
            meta = other.meta;

            if (meta != nullptr)
                meta->refs++;
        }

        bool operator==(const handle<handled_object>& other) const
        {
            return meta == other.meta;
        }
    };

    template<class T>
    bool is_empty(const handle<T>& handle)
    {
        return handle.meta == nullptr || handle.object == nullptr;
    }

    template<class T>
    handle<T> make_handle(T* object)
    {
        return handle<T>{object};
    }

    template<class T>
    handle<T>* alloc_handle(T* object)
    {
        return new handle<T>{object};
    }

    template<class T>
    struct object_read_access
    {
    private:
        T* object;
        std::shared_mutex* mutex;

    public:
        object_read_access(const handle<T>& handle) :
            object(handle.object), mutex(&handle.meta->mutex)
        {
            mutex->lock_shared();
        }

        ~object_read_access()
        {
            mutex->unlock_shared();
        }

        const T* operator->()
        {
            return object;
        }

        const T* operator*()
        {
            return object;
        }

        object_read_access(const object_read_access<T>& cpy) = delete; 
        object_read_access(const object_read_access<T>&& cpy) = delete;
        object_read_access& operator=(const object_read_access<T>& cpy) = delete;
    };

    template<class T>
    struct object_write_access
    {
    private:
        T* object;
        std::shared_mutex* mutex;
        
    public:
        object_write_access(const handle<T>& handle) :
            object(handle.object), mutex(&handle.meta->mutex)
        {
            mutex->lock();
        }

        ~object_write_access()
        {
            mutex->unlock();
        }

        T* operator->()
        {
            return object;
        }

        T* operator*()
        {
            return object;
        }

        object_write_access(const object_read_access<T>& cpy) = delete; 
        object_write_access(const object_read_access<T>&& cpy) = delete;
        object_write_access& operator=(const object_read_access<T>& cpy) = delete;
    };
}
