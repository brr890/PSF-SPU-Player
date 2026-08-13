#ifndef SPU2LOG_AUDACIOUS_COMPAT_INDEX_H
#define SPU2LOG_AUDACIOUS_COMPAT_INDEX_H

#include <stddef.h>
#include <string.h>

#include <vector>

template <typename T>
class Index {
public:
    Index() = default;

    size_t len() const
    {
        return data_.size();
    }

    T *begin()
    {
        return data_.empty() ? nullptr : data_.data();
    }

    const T *begin() const
    {
        return data_.empty() ? nullptr : data_.data();
    }

    void clear()
    {
        data_.clear();
    }

    void append(const T *data, size_t count)
    {
        if (data != nullptr && count != 0) {
            data_.insert(data_.end(), data, data + count);
        }
    }

private:
    std::vector<T> data_;
};

#endif
