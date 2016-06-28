#pragma once
#include "Detail.hpp"
#include <string>

/**JSON string writer.
 * This implementation uses RapidJSON internally.
 * 
 * A global "write_json(JsonWriter &writer, val)" is generally used for writing values.
 * write_json has overloads and templates for the primitive types, pointers and arrays.
 * 
 * Custom objects can provide a suitable write_json overload. If there is no such overload, then
 * write_json will attempt to use to_string to convert the value to a string.
 */
class JsonWriter
{
public:
    JsonWriter();
    ~JsonWriter();

    JsonWriter(const JsonWriter &) = delete;
    JsonWriter& operator = (const JsonWriter &) = delete;

    /**Get the written data buffer.*/
    const char *data()const;
    /**Get the length of the data buffer in bytes.*/
    size_t size()const;

    // Basic outputs
    void start_array();
    void end_array();
    void start_object();
    void end_object();

    void key(const char *str, size_t len);
    template<size_t N> void key(const char (&str)[N]) { key(str, N - 1); }

    void value_null();
    void value_string(const char *str, size_t len);
    void value_string(const char *str);
    void value_string(const std::string &str) { value_string(str.data(), str.size()); }
    void value_int(int x);
    void value_uint(unsigned x);
    void value_int64(long long x);
    void value_uint64(unsigned long long x);
    void value_double(double x);
    void value_bool(bool x);

    /** Generic write value helper. Calls global write_json.*/
    template<class T> void value(const T &val)
    {
        write_json(*this, val);
    }
    template<class T, size_t N> void value(const T (&val)[N])
    {
        write_json<T, N>(*this, val);
    }

    /**Object property helper. Calls key and value.*/
    template<size_t N, class T>
    void prop(const char(&str)[N], const T &val)
    {
        key(str, N - 1);
        value(val);
    }
private:
    struct Impl;
    Impl *impl;
};

// Basic type overloads
inline void write_json(JsonWriter &writer, const char *str)
{
    if (str) writer.value_string(str);
    else writer.value_null();
}
inline void write_json(JsonWriter &writer, const std::string &str) { writer.value_string(str); }
inline void write_json(JsonWriter &writer, char x) { writer.value_int(x); }
inline void write_json(JsonWriter &writer, unsigned char x) { writer.value_int(x); }
inline void write_json(JsonWriter &writer, short x) { writer.value_int(x); }
inline void write_json(JsonWriter &writer, unsigned short x) { writer.value_uint(x); }
inline void write_json(JsonWriter &writer, int x) { writer.value_int(x); }
inline void write_json(JsonWriter &writer, unsigned x) { writer.value_uint(x); }
inline void write_json(JsonWriter &writer, long long x) { writer.value_int64(x); }
inline void write_json(JsonWriter &writer, unsigned long long x) { writer.value_uint64(x); }
inline void write_json(JsonWriter &writer, float x) { writer.value_double(x); }
inline void write_json(JsonWriter &writer, double x) { writer.value_double(x); }
inline void write_json(JsonWriter &writer, bool x) { writer.value_bool(x); }

inline void write_json(JsonWriter &writer, long x)
{
    sizeof(long) == sizeof(int) ? writer.value_int(x) : writer.value_int64(x);
}
inline void write_json(JsonWriter &writer, unsigned long x)
{
    sizeof(long) == sizeof(int) ? writer.value_uint(x) : writer.value_uint64(x);
}

/**Template overload for pointers.*/
template<class T> void write_json(JsonWriter &writer, const T *val)
{
    if (val) write_json(writer, *val);
    else writer.value_null();
}
template<class T> void write_json(JsonWriter &writer, T *val)
{
    write_json<T>(writer, (const T*)val);
}
/**Basic template JSON writer. This default template converts the value to a string
 * via to_string, then writes that as a JSON string.
 */
template<class T, typename std::enable_if<rapidjson_ext_detail::has_to_string<T>::value>::type * = nullptr>
void write_json(JsonWriter &writer, const T &val)
{
    using std::to_string;
    writer.value_string(to_string(val));
}

/**Template overload for arrays. Calls write_json_array. */
template<class T, size_t N> void write_json(JsonWriter &writer, const T(&arr)[N])
{
    write_json_array(writer, arr);
}
template<class T, size_t N> void write_json(JsonWriter &writer, T(&arr)[N])
{
    write_json_array(writer, arr);
}
template<class T, typename std::enable_if<rapidjson_ext_detail::is_iterable<T>::value>::type * = nullptr>
void write_json(JsonWriter &writer, const T &iterable)
{
    write_json_array(writer, iterable);
}

/**Generic template for arrays. Writes a JSON array, using write_json for each element.
 * 
 * iterable may be any value that can be used with the C++11 range-based for loop.
 */
template<class T> void write_json_array(JsonWriter &writer,  const T &iterable)
{
    writer.start_array();
    for (auto &i : iterable)
    {
        write_json(writer, i);
    }
    writer.end_array();
}

