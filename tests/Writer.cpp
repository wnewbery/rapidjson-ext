#include <boost/test/unit_test.hpp>
#include "Writer.hpp"
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <list>

BOOST_AUTO_TEST_SUITE(TestWriter)

std::string quotes(std::string str)
{
    std::replace(str.begin(), str.end(), '\'', '"');
    return str;
}
BOOST_AUTO_TEST_CASE(basic)
{
    const char *null_str = nullptr;
    const char *non_null_str = "Hello World";
    int x = 42;
    int *null_int = nullptr;
    int *non_null_int = &x;

    JsonWriter writer;
    writer.start_object();
    writer.prop("a", 5);
    writer.prop("b", true);

    writer.key("c");
    writer.start_array();
    writer.value(null_str);
    writer.value(non_null_str);
    writer.value(null_int);
    writer.value(non_null_int);
    writer.end_array();

    writer.end_object();

    std::string expected =
        "{"
        "'a':5,"
        "'b':true,"
        "'c':[null,'Hello World',null,42]"
        "}";

    BOOST_CHECK_EQUAL(quotes(expected), std::string(writer.data(), writer.size()));
}

BOOST_AUTO_TEST_CASE(arrays)
{
    int arr[] = { 1, 2, 3, 5, 8 };
    std::vector<int> arr2 = { 1, 2, 3, 5, 8 };
    int *arr3[] = { &arr[0], nullptr, &arr[1] };
    std::vector<int> arr4 = { 1, 2, 3, 5, 8 };

    JsonWriter writer;
    writer.start_object();

    writer.prop("a", arr);
    writer.prop("b", arr2);
    writer.prop("c", arr3);
    writer.prop("d", arr4);

    writer.end_object();

    std::string expected =
        "{"
        "'a':[1,2,3,5,8],"
        "'b':[1,2,3,5,8],"
        "'c':[1,null,2],"
        "'d':[1,2,3,5,8]"
        "}";

    BOOST_CHECK_EQUAL(quotes(expected), std::string(writer.data(), writer.size()));
}

struct MyObject
{
    int x;
    std::string str;
    std::vector<std::string> words;
};
void write_json(JsonWriter &writer, const MyObject &x)
{
    writer.start_object();
    writer.prop("x", x.x);
    writer.prop("str", x.str);
    writer.prop("words", x.words);
    writer.end_object();
}
BOOST_AUTO_TEST_CASE(custom_type)
{
    MyObject a = { 55, "Hello World", { "Apple", "Orange"} };
    MyObject b[] = {
        { 10, "foo", { "Red", "Blue" }},
        { 15, "bar", { "Black", "White" }}
    };
    JsonWriter writer;
    writer.start_object();
    writer.prop("a", a);
    writer.prop("b", b);
    writer.end_object();

    std::string expected =
        "{"
        "'a':{'x':55,'str':'Hello World','words':['Apple','Orange']},"
        "'b':["
        "{'x':10,'str':'foo','words':['Red','Blue']},"
        "{'x':15,'str':'bar','words':['Black','White']}"
        "]"
        "}";

    BOOST_CHECK_EQUAL(quotes(expected), std::string(writer.data(), writer.size()));
}
BOOST_AUTO_TEST_SUITE_END()
