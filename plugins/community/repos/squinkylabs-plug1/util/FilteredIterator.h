#pragma once

#include <iterator>
#include <functional>

template <typename T, typename UnderlyingIterator>
class filtered_iterator
{
public:

    typedef typename UnderlyingIterator::value_type value_type;
    typedef typename UnderlyingIterator::difference_type difference_type;
    typedef typename UnderlyingIterator::reference reference;
    typedef typename UnderlyingIterator::pointer * pointer;

    // we are not bidirectional, even if the underlying iterator is
    typedef std::forward_iterator_tag iterator_category;

    using filter_func = std::function<bool(UnderlyingIterator)>;
    filtered_iterator(UnderlyingIterator ibegin,
        UnderlyingIterator iend,
        filter_func f)
        : _it(ibegin), _end(iend), _filter_func(f)
    {
        searchIfNeeded();
    }
    bool operator != (const filtered_iterator& z) const
    {
        return z._it != this->_it;
    }
    bool operator == (const filtered_iterator& z) const
    {
        return z._it == this->_it;
    }

    filtered_iterator& operator ++ ()
    {
        _it++;
        searchIfNeeded();
        return *this;
    }

    filtered_iterator operator ++ (int)
    {
        filtered_iterator returnValue = *this;
        _it++;
        searchIfNeeded();
        return returnValue;
    }

    void searchIfNeeded()
    {
        for (bool done = false; !done; ) {
            if (_it == _end) {
                done = true;
            } else if (_filter_func(_it)) {
                done = true;
            } else {
                ++_it;
            }
        }
    }

    value_type operator * () const
    {
        return *_it;
    }
    const value_type * operator ->() const
    {
        return &*_it;
    }
private:
    UnderlyingIterator _it;
    UnderlyingIterator _end;
    filter_func _filter_func;
};
