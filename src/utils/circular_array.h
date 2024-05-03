#pragma once

#include <vector>

namespace nimlib::Sever::Utils
{
    template<typename T>
    struct CircularArray
    {
        CircularArray(int capacity);
        ~CircularArray() = default;

        void push_back(T t);
        T& operator[](int i);
        std::vector<T> copy() const;

    private:
        int capacity;
        std::vector<T> array{};
        int next{};
        int fill_count{};
    };

    template<typename T>
    CircularArray<T>::CircularArray(int capacity) : capacity{ capacity } { array.resize(capacity); }

    template<typename T>
    void CircularArray<T>::push_back(T t)
    {
        array[next] = t;
        next = (next + 1) % capacity;
        if (fill_count < capacity) fill_count++;
    }

    template<typename T>
    T& CircularArray<T>::operator[](int i)
    {
        if (next - fill_count >= 0)
        {
            return array[next - fill_count + i];
        }
        else
        {
            return array[(next - fill_count + capacity + i) % capacity];
        }
    }

    template<typename T>
    std::vector<T> CircularArray<T>::copy() const
    {
        std::vector<T> array_copy{};
        array_copy.resize(fill_count);

        for (int i = 1, j = fill_count - 1; i <= fill_count; i++, j--)
        {
            int pos = next - i;
            array_copy[j] = pos >= 0 ? array[pos] : array[capacity + pos];
        }

        return std::move(array_copy);
    }
};
