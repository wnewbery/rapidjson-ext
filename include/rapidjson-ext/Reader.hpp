#pragma once
#include <stdexcept>
#include <stack>
#include <memory>
#include <limits>
#include <string>
#include <cassert>

class ReaderFrame;
void read_json(const std::string &str, std::unique_ptr<ReaderFrame> &&root);

template<class T>
void read_json(const std::string &str, T *p)
{
    read_json(str, make_json_reader(p));
}

class ReaderError : public std::runtime_error
{
public:
    ReaderError(const char *str) : std::runtime_error(str) {}
};
class ReaderFrame
{
public:
    virtual ~ReaderFrame() {}

    virtual bool is_array()const { return false; }

    virtual void value_null() { throw ReaderError("Unexpected null"); }
    virtual void value_bool(bool b) { throw ReaderError("Unexpected bool"); }
    virtual void value_int(int i) { value_int64(i); }
    virtual void value_uint(unsigned i) { value_uint64(i); }
    virtual void value_int64(int64_t i) { throw ReaderError("Unexpected int64"); }
    virtual void value_uint64(uint64_t i) { throw ReaderError("Unexpected uint64"); }
    virtual void value_double(double d) { throw ReaderError("Unexpected double"); }
    virtual void value_string(const std::string &str) { throw ReaderError("Unexpected string"); }
    virtual std::unique_ptr<ReaderFrame> start_array() { throw ReaderError("Unexpected array"); }
    virtual void end_array() { throw ReaderError("Unexpected array end"); }
    virtual std::unique_ptr<ReaderFrame> start_object() { throw ReaderError("Unexpected object"); }
    virtual void end_object() { throw ReaderError("Unexpected object end"); }
    virtual std::unique_ptr<ReaderFrame> key(const std::string &str) { throw ReaderError("Unexpected key"); }
};

class ReaderBool : public ReaderFrame
{
public:
    explicit ReaderBool(bool *out) : out(out) {}
    virtual void value_bool(bool b)override { *out = b; }
private:
    bool *out;
};
template<class T>
class ReaderInt : public ReaderFrame
{
public:
    explicit ReaderInt(T *out) : out(out) {}
    virtual void value_int64(int64_t i)
    {
        if (i < (int64_t)std::numeric_limits<T>::min()) throw ReaderError("Out of range");
        *out = (T)i;
    }
    virtual void value_uint64(uint64_t i)
    {
        if (i > (uint64_t)std::numeric_limits<T>::max()) throw ReaderError("Out of range");
        *out = (T)i;
    }
private:
    T *out;
};
template<class T>
class ReaderUInt : public ReaderFrame
{
public:
    explicit ReaderUInt(T *out) : out(out) {}
    virtual void value_int64(int64_t i) { throw ReaderError("Out of range"); }
    virtual void value_uint64(uint64_t i)
    {
        if (i > (uint64_t)std::numeric_limits<T>::max()) throw ReaderError("Out of range");
        *out = (T)i;
    }
private:
    T *out;
};

template<class T>
class ReaderFloat : public ReaderFrame
{
public:
    explicit ReaderFloat(T *out) : out(out) {}
    virtual void value_int64(int64_t i) { *out = (T)i; }
    virtual void value_uint64(uint64_t i) { *out = (T)i; }
    virtual void value_double(double d) { *out = (T)d; }
private:
    T *out;
};

class ReaderString : public ReaderFrame
{
public:
    explicit ReaderString(std::string *out) : out(out) {}
    virtual void value_string(const std::string &str) { *out = str; }
private:
    std::string *out;
};

class ReaderObject : public ReaderFrame
{
public:
    virtual std::unique_ptr<ReaderFrame> start_object()override { return nullptr; }
    virtual void end_object()override {}
};

template<class T>
class ReaderList : public ReaderFrame
{
public:
    typedef typename T::value_type value_type;
    ReaderList(T *list)
        : list(list), in_array(false), tmp_value(), value_reader(make_json_reader(&tmp_value))
    {
    }

    virtual bool is_array()const override { return true; }
    virtual std::unique_ptr<ReaderFrame> start_array()override
    {
        if (!in_array)
        {
            in_array = true;
            return nullptr;
        }
        else
        {
            list->emplace_back();
            return make_json_reader(&list->back());
        }
    }
    virtual void end_array()override
    {
        if (in_array) in_array = false;
        else throw std::runtime_error("Unexpected array end");
    }
    virtual std::unique_ptr<ReaderFrame> start_object()override
    {
        list->emplace_back();
        return make_json_reader(&list->back());
    }
    virtual void end_object()override
    {
    }


    virtual void value_null()override
    {
        if (!in_array) throw std::runtime_error("Expected array");
        value_reader->value_null();
        list->push_back(tmp_value);
    }
    virtual void value_bool(bool b)
    {
        if (!in_array) throw std::runtime_error("Expected array");
        value_reader->value_bool(b);
        list->push_back(tmp_value);
    }
    virtual void value_int(int i)
    {
        if (!in_array) throw std::runtime_error("Expected array");
        value_reader->value_int(i);
        list->push_back(tmp_value);
    }
    virtual void value_uint(unsigned i)
    {
        if (!in_array) throw std::runtime_error("Expected array");
        value_reader->value_uint(i);
        list->push_back(tmp_value);
    }
    virtual void value_int64(int64_t i)
    {
        if (!in_array) throw std::runtime_error("Expected array");
        value_reader->value_int64(i);
        list->push_back(tmp_value);
    }
    virtual void value_uint64(uint64_t i)
    {
        if (!in_array) throw std::runtime_error("Expected array");
        value_reader->value_uint64(i);
        list->push_back(tmp_value);
    }
    virtual void value_double(double d)
    {
        if (!in_array) throw std::runtime_error("Expected array");
        value_reader->value_double(d);
        list->push_back(tmp_value);
    }
    virtual void value_string(const std::string &str)
    {
        if (!in_array) throw std::runtime_error("Expected array");
        value_reader->value_string(str);
        list->push_back(tmp_value);
    }
private:
    T *list;
    bool in_array;
    value_type tmp_value;
    decltype(make_json_reader(std::add_pointer<value_type>::type())) value_reader;
};


namespace rapidjson_ext_detail
{
    // Enable argument dependent lookup
    using std::begin;
    using std::end;

    /**Implementation for is_iterable*/
    template<class T>
    auto is_list_impl(int) -> decltype(
        std::declval<T>().push_back(std::declval<typename T::value_type>()),
        std::true_type{}
    );
    /**Not iterable. ... is less specific than (int)*/
    template<class T>
    std::false_type is_list_impl(...);

    /**Consider an object a list if it has a push_back(value_type)*/
    template <class T> struct is_list : public decltype(is_list_impl<T>(0)) {};
}

inline std::unique_ptr<ReaderFrame> make_json_reader(char *p) { return std::make_unique<ReaderInt<char>>(p); }
inline std::unique_ptr<ReaderFrame> make_json_reader(unsigned char *p) { return std::make_unique<ReaderInt<unsigned char>>(p); }
inline std::unique_ptr<ReaderFrame> make_json_reader(short *p) { return std::make_unique<ReaderInt<short>>(p); }
inline std::unique_ptr<ReaderFrame> make_json_reader(unsigned short *p) { return std::make_unique<ReaderInt<unsigned short>>(p); }
inline std::unique_ptr<ReaderFrame> make_json_reader(int *p) { return std::make_unique<ReaderInt<int>>(p); }
inline std::unique_ptr<ReaderFrame> make_json_reader(unsigned *p) { return std::make_unique<ReaderInt<unsigned>>(p); }
inline std::unique_ptr<ReaderFrame> make_json_reader(long *p) { return std::make_unique<ReaderInt<long>>(p); }
inline std::unique_ptr<ReaderFrame> make_json_reader(unsigned long *p) { return std::make_unique<ReaderInt<unsigned long>>(p); }
inline std::unique_ptr<ReaderFrame> make_json_reader(long long *p) { return std::make_unique<ReaderInt<long long>>(p); }
inline std::unique_ptr<ReaderFrame> make_json_reader(unsigned long long *p) { return std::make_unique<ReaderInt<unsigned long long>>(p); }

inline std::unique_ptr<ReaderFrame> make_json_reader(float *p) { return std::make_unique<ReaderFloat<float>>(p); }
inline std::unique_ptr<ReaderFrame> make_json_reader(double *p) { return std::make_unique<ReaderFloat<double>>(p); }

inline std::unique_ptr<ReaderFrame> make_json_reader(bool *p) { return std::make_unique<ReaderBool>(p); }

inline std::unique_ptr<ReaderFrame> make_json_reader(std::string *p) { return std::make_unique<ReaderString>(p); }

template<class T, typename std::enable_if<rapidjson_ext_detail::is_list<T>::value>::type * = nullptr>
std::unique_ptr<ReaderFrame> make_json_reader(T *list)
{
    return std::make_unique<ReaderList<T>>(list);
}

