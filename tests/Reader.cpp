#include <boost/test/unit_test.hpp>
#include "Reader.hpp"
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <list>

BOOST_AUTO_TEST_SUITE(TestReader)

std::string quotes(std::string str)
{
    std::replace(str.begin(), str.end(), '\'', '"');
    return str;
}

struct MyObject
{
    int x;
    std::string str;
    std::vector<std::string> words;
};
class MyObjectReader : public ReaderObject
{
public:
    MyObjectReader(MyObject *out) : out(out) {}

    //TODO: Make fields mandatory
    virtual std::unique_ptr<ReaderFrame> key(const std::string &str)override
    {
        typedef std::vector<std::string> T;

        if (str == "x") return make_json_reader(&out->x);
        else if (str == "str") return make_json_reader(&out->str);
        else if (str == "words") return make_json_reader(&out->words);
        else throw std::runtime_error("Unknown key " + str);
    }
private:
    MyObject *out;
};
inline std::unique_ptr<ReaderFrame> make_json_reader(MyObject *p)
{
    return std::make_unique<MyObjectReader>(p);
}

// An object containing other objects, and re-using the reader
struct MyObject2
{
    MyObject a, b;
};
class MyObjectReader2 : public ReaderObject
{
public:
    MyObjectReader2(MyObject2 *out) : out(out) {}

    virtual std::unique_ptr<ReaderFrame> key(const std::string &str)override
    {
        if (str == "a") return make_json_reader(&out->a);
        else if (str == "b") return make_json_reader(&out->b);
        else throw std::runtime_error("Unknown key " + str);
    }
private:
    MyObject2 *out;
};
inline std::unique_ptr<ReaderFrame> make_json_reader(MyObject2 *p)
{
    return std::make_unique<MyObjectReader2>(p);
}


BOOST_AUTO_TEST_CASE(object)
{
    MyObject a;
    std::string json =
        "{'x':55,'str':'Hello World','words':['Apple','Orange']}"
        ;
    std::string expected_words[] = { "Apple", "Orange" };

    read_json(quotes(json), &a);
    BOOST_CHECK_EQUAL(55, a.x);
    BOOST_CHECK_EQUAL("Hello World", a.str);
    BOOST_CHECK_EQUAL_COLLECTIONS(expected_words, expected_words + 2, a.words.begin(), a.words.end());
}

BOOST_AUTO_TEST_CASE(object_nested)
{
    MyObject2 a;
    std::string json =
        "{"
        "'a':{'x':55,'str':'Hello World'},"
        "'b':{'x':10,'str':'Red'}"
        "}"
        ;

    read_json(quotes(json), &a);
    BOOST_CHECK_EQUAL(55, a.a.x);
    BOOST_CHECK_EQUAL("Hello World", a.a.str);
    BOOST_CHECK_EQUAL(10, a.b.x);
    BOOST_CHECK_EQUAL("Red", a.b.str);
}

BOOST_AUTO_TEST_CASE(array)
{
    std::string expected_words1[] = { "Red", "Blue" };
    std::string expected_words2[] = { "Black", "White" };
    std::vector<MyObject> objects;
    std::string json =
        "["
        "{'x':10,'str':'foo','words':['Red','Blue']},"
        "{'x':15,'str':'bar','words':['Black','White']}"
        "]";
    read_json(quotes(json), &objects);
    BOOST_CHECK_EQUAL(2, objects.size());
    if (objects.size() == 2)
    {
        BOOST_CHECK_EQUAL(10, objects[0].x);
        BOOST_CHECK_EQUAL(15, objects[1].x);
        BOOST_CHECK_EQUAL_COLLECTIONS(expected_words1, expected_words1 + 2, objects[0].words.begin(), objects[0].words.end());
        BOOST_CHECK_EQUAL_COLLECTIONS(expected_words2, expected_words2 + 2, objects[1].words.begin(), objects[1].words.end());
    }
}

BOOST_AUTO_TEST_CASE(array_nested)
{
    std::string json = "[[1,2,3,5,8],[1,2,3,5,7,11]]";
    std::vector<std::vector<int>> arrays;
    read_json(quotes(json), &arrays);
}
BOOST_AUTO_TEST_SUITE_END()
