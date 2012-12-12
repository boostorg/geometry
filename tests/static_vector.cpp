#include <boost/aligned_storage.hpp>
#include <boost/assert.hpp>

template <typename Value, size_t Capacity>
class static_vector
{
public:
    typedef Value value_type;
    typedef size_t size_type;
    typedef Value& reference;
    typedef Value const& const_reference;
    typedef Value* iterator;
    typedef const Value * const_iterator;

    static_vector()
        : m_size(0)
    {}

    static_vector(size_type s)
    {
        try
        {
            resize(s);                                                  // may throw
        }
        catch(...)
        {
            this->destroy(0, m_size);
        }
    }

    ~static_vector()
    {
        this->destroy(0, m_size);
    }

    void resize(size_type s)
    {
        if ( s < m_size )
        {
            this->destroy(s, m_size);
            m_size = s;
        }
        else
        {
            for ( size_type i = m_size ; i < s ; ++i )
            {
                this->construct(i);                                     // may throw
                ++m_size;
            }
        }
    }

    void push_back(Value const& value)
    {
        BOOST_ASSERT_MSG(m_size < Capacity, "max capacity reached");
        this->construct(m_size, value);                                 // may throw
        ++m_size;
    }

    void pop_back()
    {
        BOOST_ASSERT_MSG(0 < m_size, "the container is empty");
        --m_size;
        this->destroy(m_size);
    }

    Value & operator[](size_t i)
    {
        BOOST_ASSERT_MSG(i < Capacity, "index out of bounds");
        return *(this->ptr(i));
    }

    Value const& operator[](size_t i) const
    {
        BOOST_ASSERT_MSG(i < Capacity, "index out of bounds");
        return *(this->ptr(i));
    }

    Value & front()
    {
        BOOST_ASSERT_MSG(0 < m_size, "the container is empty");
        return *(this->ptr(0));
    }

    Value const& front() const
    {
        BOOST_ASSERT_MSG(0 < m_size, "the container is empty");
        return *(this->ptr(0));
    }

    Value & back()
    {
        BOOST_ASSERT_MSG(0 < m_size, "the container is empty");
        return *(this->ptr(m_size - 1));
    }

    Value const& back() const
    {
        BOOST_ASSERT_MSG(0 < m_size, "the container is empty");
        return *(this->ptr(m_size - 1));
    }

    size_type capacity() const
    {
        return Capacity;
    }

    size_type size() const
    {
        return m_size;
    }

    bool empty() const
    {
        return 0 == m_size;
    }

private:
    Value * ptr(size_type i)
    {
        return (reinterpret_cast<Value*>(m_storage.address()) + i);
    }

    const Value * ptr(size_type i) const
    {
        return (reinterpret_cast<const Value*>(m_storage.address()) + i);
    }

    void construct(size_type i)
    {
        new (ptr(i)) value_type();                                          // may throw
    }

    void construct(size_type i, Value const& value)
    {
        new (ptr(i)) value_type(value);                                     // may throw
    }

    void destroy(size_type i)
    {
        ptr(i)->~value_type();
    }

    void destroy(size_type first_i, size_type last_i)
    {
        for ( size_type i = first_i ; i < last_i ; ++i )
            this->destroy(i);
    }

    boost::aligned_storage<sizeof(Value) * Capacity> m_storage;
    size_type m_size;
};

// ------------------------------- TEST ----------------------------------

#include <iostream>

struct A
{
    A() : v(0) { std::cout << "A()" << std::endl; }
    A(int i) : v(i) { std::cout << "A(" << i << ")" << std::endl; }
    ~A() { std::cout << "~A" << v << "()" << std::endl; }
    A(A const& a) { std::cout << "A(A(" << a.v << "))" << std::endl; v = a.v; }
    A & operator=(A const& a) { std::cout << "A" << v << " = A" << a.v << std::endl; v = a.v; return *this; }
    int v;
};

int main()
{
    static_vector<A, 4> sv;
    std::cout << "resize(2)" << std::endl;
    sv.resize(2);           // 0 0
    std::cout << "push_back(A(22))" << std::endl;
    sv.push_back(A(22));    // 0 0 22
    std::cout << "push_back(A(23))" << std::endl;
    sv.push_back(A(23));    // 0 0 22 23
    std::cout << "pop_back()" << std::endl;
    sv.pop_back();          // 0 0 22
    sv.front().v = 10;      // 10 0 22
    sv.back().v = 100;      // 10 0 100
    sv[1].v = 50;           // 10 50 100
    std::cout << "resize(1)" << std::endl;
    sv.resize(1);           // 10
    std::cout << "~static_vector()" << std::endl;
    return 0;
}
