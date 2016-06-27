#include "Writer.hpp"
#include <rapidjson/writer.h>
#include <stdexcept>
#include <limits>

namespace
{
    void check(bool b)
    {
        if (!b) throw std::runtime_error("JsonWriter error");
    }
}
struct JsonWriter::Impl
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer;

    Impl() : buffer(), writer(buffer) {}
};

JsonWriter::JsonWriter()
    : impl(new Impl())
{
}

JsonWriter::~JsonWriter()
{
    delete impl;
}

const char * JsonWriter::data() const
{
    return impl->buffer.GetString();
}

size_t JsonWriter::size() const
{
    return impl->buffer.GetSize();
}

void JsonWriter::start_array()
{
    check(impl->writer.StartArray());
}

void JsonWriter::end_array()
{
    check(impl->writer.EndArray());
}

void JsonWriter::start_object()
{
    check(impl->writer.StartObject());
}

void JsonWriter::end_object()
{
    check(impl->writer.EndObject());
}

void JsonWriter::key(const char * str, size_t len)
{
    if (len > std::numeric_limits<rapidjson::SizeType>::max())
        throw std::runtime_error("Max string length exceeded");
    check(impl->writer.Key(str, (rapidjson::SizeType)len, true));
}

void JsonWriter::value_null()
{
    check(impl->writer.Null());
}

void JsonWriter::value_string(const char * str, size_t len)
{
    if (len > std::numeric_limits<rapidjson::SizeType>::max())
        throw std::runtime_error("Max string length exceeded");
    check(impl->writer.String(str, (rapidjson::SizeType)len, true));
}

void JsonWriter::value_string(const char * str)
{
    check(impl->writer.String(str));
}

void JsonWriter::value_int(int x)
{
    check(impl->writer.Int(x));
}

void JsonWriter::value_uint(unsigned x)
{
    check(impl->writer.Uint(x));
}

void JsonWriter::value_int64(long long x)
{
    check(impl->writer.Int64(x));
}

void JsonWriter::value_uint64(unsigned long long x)
{
    check(impl->writer.Uint64(x));
}

void JsonWriter::value_double(double x)
{
    check(impl->writer.Double(x));
}

void JsonWriter::value_bool(bool x)
{
    check(impl->writer.Bool(x));
}
