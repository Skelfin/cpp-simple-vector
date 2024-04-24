#pragma once

#include "array_ptr.h"
#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <utility>

struct ReserveProxyObj {
    size_t capacity;
    explicit ReserveProxyObj(size_t new_capacity) : capacity(new_capacity) {}
};

template <typename Type>
class SimpleVector {
private:
    ArrayPtr<Type> array_;
    size_t size_ = 0;
    size_t capacity_ = 0;
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    explicit SimpleVector(size_t size)
        : array_(size), size_(size), capacity_(size) {
        std::fill(begin(), end(), Type());
    }

    explicit SimpleVector(ReserveProxyObj reserve)
        : array_(reserve.capacity), size_(0), capacity_(reserve.capacity) {}

    SimpleVector(size_t size, const Type& value)
        : array_(size), size_(size), capacity_(size) {
        std::fill(begin(), end(), value);
    }

    SimpleVector(std::initializer_list<Type> init)
        : array_(init.size()), size_(init.size()), capacity_(init.size()) {
        std::copy(init.begin(), init.end(), begin());
    }

    SimpleVector(const SimpleVector& other)
        : array_(other.size_), size_(other.size_), capacity_(other.size_) {
        std::copy(other.begin(), other.end(), begin());
    }

    SimpleVector(SimpleVector&& other) noexcept
        : array_(std::move(other.array_)), size_(other.size_), capacity_(other.capacity_) {
        other.size_ = 0;
        other.capacity_ = 0;
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector copy(rhs);
            swap(copy);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) noexcept {
        if (this != &rhs) {
            array_ = std::move(rhs.array_);
            size_ = std::exchange(rhs.size_, 0);
            capacity_ = std::exchange(rhs.capacity_, 0);
        }
        return *this;
    }


    void PushBack(Type item) {
        if (size_ == capacity_) {
            size_t new_capacity = capacity_ == 0 ? 1 : 2 * capacity_;
            ArrayPtr<Type> new_array(new_capacity);
            std::move(begin(), end(), new_array.Get());
            array_.swap(new_array);
            capacity_ = new_capacity;
        }
        new(array_.Get() + size_) Type(std::move(item));
        size_++;
    }


    void PopBack() noexcept {
        assert(size_ > 0);
        --size_;
    }

    template <typename Container>
    Iterator Insert(ConstIterator pos, Container&& value) {
        size_t index = pos - begin();
        if (size_ == capacity_) {
            size_t new_capacity = capacity_ == 0 ? 1 : 2 * capacity_;
            ArrayPtr<Type> new_array(new_capacity);
            std::move(begin(), begin() + index, new_array.Get());
            new(new_array.Get() + index) Type(std::forward<U>(value));
            std::move(begin() + index, end(), new_array.Get() + index + 1);
            array_.swap(new_array);
            capacity_ = new_capacity;
        }
        else {
            std::move_backward(begin() + index, end(), end() + 1);
            new(array_.Get() + index) Type(std::forward<Container>(value));
        }
        ++size_;
        return begin() + index;
    }


    Iterator Erase(ConstIterator pos) {
        size_t index = pos - begin();
        if (index < size_ - 1) {
            std::move(begin() + index + 1, end(), begin() + index);
        }
        --size_;
        return begin() + index;
    }


    void swap(SimpleVector& other) noexcept {
        array_.swap(other.array_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> new_array(new_capacity);
            std::move(begin(), end(), new_array.Get());
            array_.swap(new_array);
            capacity_ = new_capacity;
        }
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }


    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return array_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return array_[index];
    }

    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return array_[index];
    }

    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return array_[index];
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (new_size > capacity_) {
            ArrayPtr<Type> new_array(new_size);
            std::move(std::make_move_iterator(begin()), std::make_move_iterator(end()), new_array.Get());
            for (size_t i = size_; i < new_size; ++i) {
                new(new_array.Get() + i) Type();
            }
            array_.swap(new_array);
            capacity_ = new_size;
        }
        else {
            if (new_size > size_) {
                for (size_t i = size_; i < new_size; ++i) {
                    new(array_.Get() + i) Type();
                }
            }
        }
        size_ = new_size;
    }

    Iterator begin() noexcept {
        return array_.Get();
    }

    Iterator end() noexcept {
        return array_.Get() + size_;
    }

    ConstIterator begin() const noexcept {
        return array_.Get();
    }

    ConstIterator end() const noexcept {
        return array_.Get() + size_;
    }

    ConstIterator cbegin() const noexcept {
        return begin();
    }

    ConstIterator cend() const noexcept {
        return end();
    }
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs.GetSize() == rhs.GetSize() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}