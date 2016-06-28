#include "Reader.hpp"
#include <rapidjson/reader.h>

using rapidjson::SizeType;

class Reader
{
public:
    std::stack<std::unique_ptr<ReaderFrame>> stack;

    ~Reader() {}

    bool Null()
    {
        stack.top()->value_null();
        if (!stack.top()->is_array()) stack.pop();
        return true;
    }
    bool Bool(bool b)
    {
        stack.top()->value_bool(b);
        if (!stack.top()->is_array()) stack.pop();
        return true;
    }
    bool Int(int i)
    {
        stack.top()->value_int(i);
        if (!stack.top()->is_array()) stack.pop();
        return true;
    }
    bool Uint(unsigned i)
    {
        stack.top()->value_uint(i);
        if (!stack.top()->is_array()) stack.pop();
        return true;
    }
    bool Int64(int64_t i)
    {
        stack.top()->value_int64(i);
        if (!stack.top()->is_array()) stack.pop();
        return true;
    }
    bool Uint64(uint64_t i)
    {
        stack.top()->value_uint64(i);
        if (!stack.top()->is_array()) stack.pop();
        return true;
    }
    bool Double(double d)
    {
        stack.top()->value_double(d);
        if (!stack.top()->is_array()) stack.pop();
        return true;
    }
    bool String(const char* str, SizeType length, bool copy)
    {
        stack.top()->value_string({str, (size_t)length});
        if (!stack.top()->is_array()) stack.pop();
        return true;
    }
    bool StartObject()
    {
        auto next = stack.top()->start_object();
        if (next)
        {
            next->start_object();
            stack.push(std::move(next));
        }
        return true;
    }
    bool Key(const char* str, SizeType length, bool copy)
    {
        stack.emplace(stack.top()->key({str, (size_t)length}));
        return true;
    }
    bool EndObject(SizeType memberCount)
    {
        stack.top()->end_object();
        if (!stack.top()->is_array()) stack.pop();
        return true;
    }
    bool StartArray()
    {
        auto next = stack.top()->start_array();
        if (next)
        {
            next->start_array();
            stack.push(std::move(next));
        }
        return true;
    }
    bool EndArray(SizeType elementCount)
    {
        stack.top()->end_array();
        stack.pop();
        return true;
    }
};

void read_json(const std::string &str, std::unique_ptr<ReaderFrame> &&root)
{
    Reader reader;
    reader.stack.emplace(std::move(root));
    
    rapidjson::Reader json_reader;
    rapidjson::StringStream ss(str.c_str());
    if (!json_reader.Parse(ss, reader)) throw std::runtime_error("Parse error");
}
