#include "gtest/gtest.h"
#include <scapegoat_tree.h>
#include <logger_builder.h>
#include <client_logger_builder.h>
#include <iostream>


logger *create_logger(
        std::vector<std::pair<std::string, logger::severity>> const &output_file_streams_setup,
        bool use_console_stream = true,
        logger::severity console_stream_severity = logger::severity::debug)
{
    std::unique_ptr<logger_builder> builder(new client_logger_builder());

    builder->set_format("[%d %t][%s] %m");

    if (use_console_stream)
    {
        builder->add_console_stream(console_stream_severity);
    }

    for (auto &output_file_stream_setup: output_file_streams_setup)
    {
        builder->add_file_stream(output_file_stream_setup.first, output_file_stream_setup.second);
    }

    logger *built_logger = builder->build();


    return built_logger;
}

bool compare_results(
        std::vector<typename binary_search_tree<int, std::string>::value_type> &expected,
        std::vector<typename binary_search_tree<int, std::string>::value_type> &actual)
{

    if (expected.size() != actual.size())
    {
        return false;
    }

    for (auto&& val : std::views::zip(expected, actual))
    {
        if (std::get<0>(val).first != std::get<1>(val).first)
        {
            return false;
        }

        if (std::get<0>(val).second != std::get<1>(val).second)
        {
            return false;
        }
    }

    return true;
}

template<typename tkey, typename tvalue>
struct test_data
{
    tkey key;
    tvalue value;
    size_t depth;

    test_data(size_t dep, tkey k, tvalue v) : depth(dep), key(k), value(v){}
};

template<
    typename tkey,
    typename tvalue>
bool infix_iterator_test(
    scapegoat_tree<tkey, tvalue> const &tree,
    std::vector<test_data<tkey, tvalue>> &expected_result)
{
    
    std::string line;
    auto end_infix = tree.cend_infix();
    auto it = tree.cbegin_infix();
    
    for (auto const &item: expected_result)
    {
        if (it.depth() != item.depth || it->first != item.key || it->second != item.value)
        {
            return false;
        }
        ++it;
    }
    return true;
}

template<
    typename tkey,
    typename tvalue>
bool prefix_iterator_test(
    scapegoat_tree<tkey, tvalue> const &tree,
    std::vector<test_data<tkey, tvalue>> &expected_result)
{
    std::string line;
    auto end_prefix = tree.cend_prefix();
    auto it = tree.cbegin_prefix();
    
    for (auto const &item: expected_result)
    {
        if (it.depth() != item.depth || it->first != item.key || it->second != item.value)
        {
            return false;
        }
        ++it;
    }
    return true;
}

template<
    typename tkey,
    typename tvalue>
bool postfix_iterator_test(
    scapegoat_tree<tkey, tvalue> const &tree,
    std::vector<test_data<tkey, tvalue>> &expected_result)
{
    
    std::string line;
    auto end_postfix = tree.cend_postfix();
    auto it = tree.cbegin_postfix();
    
    for (auto const &item: expected_result)
    {
        if (it.depth() != item.depth || it->first != item.key || it->second != item.value)
        {
            return false;
        }
        ++it;
    }
    return true;
}

TEST(scapegoatTreePositiveTests, test1)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
        {
            {
                "scapegoat_tree_tests_logs.txt",
                logger::severity::trace
            },
        }));
    
    logger->trace("scapegoatTreePositiveTests.test1 started");
    
    auto sg = std::make_unique<scapegoat_tree<int, std::string>>(std::less<int>(), nullptr, logger.get(), 0.7);
    
    sg->emplace(5, "a");
    sg->emplace(2, "b");
    sg->emplace(15, "c");
    sg->emplace(3, "d");
    sg->emplace(14, "e");
    sg->emplace(1, "l");
    
    std::vector<test_data<int, std::string>> expected_result =
        {
            test_data<int, std::string>(2, 1, "l"),
            test_data<int, std::string>(1, 2, "b"),
            test_data<int, std::string>(2, 3, "d"),
            test_data<int, std::string>(0, 5, "a"),
            test_data<int, std::string>(2, 14, "e"),
            test_data<int, std::string>(1, 15, "c")
        };
    
    EXPECT_TRUE(infix_iterator_test(*sg, expected_result));
    
    logger->trace("scapegoatTreePositiveTests.test1 finished");
}

TEST(scapegoatTreePositiveTests, test2)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "scapegoat_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("scapegoatTreePositiveTests.test2 started");

    auto sg = std::make_unique<scapegoat_tree<int, int>>(std::less<int>(), nullptr, logger.get(), 0.5);
    
    sg->emplace(1, 5);
    sg->emplace(2, 12);
    sg->emplace(15, 1);
    sg->emplace(3, 67);
    sg->emplace(4, 45);
    
    std::vector<test_data<int, int>> expected_result =
        {
                test_data<int, int>(0, 3, 67),
                test_data<int, int>(1, 2, 12),
                test_data<int, int>(2, 1, 5),
                test_data<int, int>(1, 15, 1),
                test_data<int, int>(2, 4, 45)
        };
    
    EXPECT_TRUE(prefix_iterator_test(*sg, expected_result));
    
    logger->trace("scapegoatTreePositiveTests.test2 finished");
}

TEST(scapegoatTreePositiveTests, test3)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "scapegoat_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("scapegoatTreePositiveTests.test3 started");

    auto sg = std::make_unique<scapegoat_tree<std::string, int>>(std::less<std::string>(), nullptr, logger.get(), 0.9);
    
    sg->emplace("a", 1);
    sg->emplace("b", 2);
    sg->emplace("c", 15);
    sg->emplace("d", 3);
    sg->emplace("e", 4);
    
    std::vector<test_data<std::string, int>> expected_result =
        {
                test_data<std::string, int>(4, "e", 4),
                test_data<std::string, int>(3, "d", 3),
                test_data<std::string, int>(2, "c", 15),
                test_data<std::string, int>(1, "b", 2),
                test_data<std::string, int>(0, "a", 1)
        };
    
    EXPECT_TRUE(postfix_iterator_test(*sg, expected_result));
    
    logger->trace("scapegoatTreePositiveTests.test3 finished");
}

TEST(scapegoatTreePositiveTests, test4)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "scapegoat_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("scapegoatTreePositiveTests.test4 started");

    auto sg1 = std::make_unique<scapegoat_tree<int, std::string>>(std::less<int>(), nullptr, logger.get(), 0.65);
    
    sg1->emplace(6, "a");
    sg1->emplace(8, "c");
    sg1->emplace(15, "x");
    sg1->emplace(4, "j");
    sg1->emplace(1, "i");
    sg1->emplace(5, "b");
    
    std::vector<test_data<int, std::string>> expected_result =
        {
            test_data<int, std::string>(2, 1, "i"),
            test_data<int, std::string>(1, 4, "j"),
            test_data<int, std::string>(2, 5, "b"),
            test_data<int, std::string>(0, 6, "a"),
            test_data<int, std::string>(2, 8, "c"),
            test_data<int, std::string>(1, 15, "x"),
        };
    
    scapegoat_tree<int, std::string> sg2(std::move(*sg1));
    
    EXPECT_TRUE(infix_iterator_test(sg2, expected_result));
    
    logger->trace("scapegoatTreePositiveTests.test4 finished");
}

TEST(scapegoatTreePositiveTests, test5)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "scapegoat_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("scapegoatTreePositiveTests.test5 started");

    auto sg1 = std::make_unique<scapegoat_tree<int, std::string>>(std::less<int>(), nullptr, logger.get(), 0.65);
    
    sg1->emplace(6, "a");
    sg1->emplace(8, "c");
    sg1->emplace(15, "x");
    sg1->emplace(4, "j");
    sg1->emplace(1, "i");
    sg1->emplace(5, "b");
    
    std::vector<test_data<int, std::string>> expected_result =
        {
            test_data<int, std::string>(2, 1, "i"),
            test_data<int, std::string>(1, 4, "j"),
            test_data<int, std::string>(2, 5, "b"),
            test_data<int, std::string>(0, 6, "a"),
            test_data<int, std::string>(2, 8, "c"),
            test_data<int, std::string>(1, 15, "x"),
        };
    
    scapegoat_tree<int, std::string> sg2 = std::move(
        *sg1);
    
    EXPECT_TRUE(infix_iterator_test(sg2, expected_result));
    
    logger->trace("scapegoatTreePositiveTests.test5 finished");
}

TEST(scapegoatTreePositiveTests, test6)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "scapegoat_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("scapegoatTreePositiveTests.test6 started");

    auto sg1 = std::make_unique<scapegoat_tree<int, std::string>>(std::less<int>(), nullptr, logger.get(), 0.5);
    
    sg1->emplace(6, "a");
    sg1->emplace(8, "c");
    sg1->emplace(15, "x");
    sg1->emplace(4, "j");
    sg1->emplace(1, "i");
    sg1->emplace(5, "b");
    
    sg1->erase(5);
    
    std::vector<test_data<int, std::string>> expected_result =
        {
            test_data<int, std::string>(2, 1, "i"),
            test_data<int, std::string>(1, 4, "j"),
            test_data<int, std::string>(0, 6, "a"),
            test_data<int, std::string>(2, 8, "c"),
            test_data<int, std::string>(1, 15, "x"),
        };
    
    EXPECT_TRUE(infix_iterator_test(*sg1, expected_result));
    
    logger->trace("scapegoatTreePositiveTests.test6 finished");
}

TEST(scapegoatTreePositiveTests, test7)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "scapegoat_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("scapegoatTreePositiveTests.test7 started");

    auto sg1 = std::make_unique<scapegoat_tree<int, std::string>>(std::less<int>(), nullptr, logger.get(), 0.7);
    
    sg1->emplace(6, "a");
    sg1->emplace(8, "c");
    sg1->emplace(15, "x");
    sg1->emplace(4, "j");
    sg1->emplace(3, "i");
    sg1->emplace(2, "l");
    sg1->emplace(5, "b");
    
    sg1->erase(3);
    
    std::vector<test_data<int, std::string>> expected_result =
        {
            test_data<int, std::string>(2, 2, "l"),
            test_data<int, std::string>(1, 4, "j"),
            test_data<int, std::string>(2, 5, "b"),
            test_data<int, std::string>(0, 6, "a"),
            test_data<int, std::string>(1, 8, "c"),
            test_data<int, std::string>(2, 15, "x"),
        };
    
    EXPECT_TRUE(infix_iterator_test(*sg1, expected_result));
    
    logger->trace("scapegoatTreePositiveTests.test7 finished");
}

TEST(scapegoatTreePositiveTests, test8)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "scapegoat_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("scapegoatTreePositiveTests.test8 started");

    auto sg1 = std::make_unique<scapegoat_tree<int, std::string>>(std::less<int>(), nullptr, logger.get(), 0.7);
    
    sg1->emplace(6, "a");
    sg1->emplace(8, "c");
    sg1->emplace(15, "x");
    sg1->emplace(11, "j");
    sg1->emplace(19, "i");
    sg1->emplace(12, "l");
    sg1->emplace(17, "b");
    sg1->emplace(18, "e");
    
    sg1->erase(12);
    sg1->erase(15);
    
    std::vector<test_data<int, std::string>> expected_result =
        {
            test_data<int, std::string>(2, 6, "a"),
            test_data<int, std::string>(1, 8, "c"),
            test_data<int, std::string>(0, 11, "j"),
            test_data<int, std::string>(2, 17, "b"),
            test_data<int, std::string>(1, 18, "e"),
            test_data<int, std::string>(2, 19, "i"),
        };
    
    EXPECT_TRUE(infix_iterator_test(*sg1, expected_result));
    
    logger->trace("scapegoatTreePositiveTests.test8 finished");
}

TEST(scapegoatTreePositiveTests, test9)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "scapegoat_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("scapegoatTreePositiveTests.test9 started");

    auto sg1 = std::make_unique<scapegoat_tree<int, std::string>>(std::less<int>(), nullptr, logger.get(), 0.7);
    
    sg1->emplace(6, "l");
    sg1->emplace(8, "c");
    sg1->emplace(15, "l");
    sg1->emplace(11, "o");
    sg1->emplace(9, "h");
    sg1->emplace(2, "e");
    sg1->emplace(4, "b");
    sg1->emplace(18, "e");
    
    std::vector<std::string> vector;
    
    vector.push_back(sg1->at(9));
    vector.push_back(sg1->at(2));
    vector.push_back(sg1->at(15));
    vector.push_back(sg1->at(6));
    vector.push_back(sg1->at(11));
    
    std::string actual_result;
    
    for (auto const &item: vector)
    {
        actual_result.append(item + " ");
    }
    
    EXPECT_EQ(actual_result, "h e l l o ");
    
    logger->trace("scapegoatTreePositiveTests.test9 finished");
}

TEST(scapegoatTreePositiveTests, test10)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "scapegoat_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("scapegoatTreePositiveTests.test10 started");

    auto sg = std::make_unique<scapegoat_tree<int, std::string>>(std::less<int>(), nullptr, logger.get(), 0.7);
    
    sg->emplace(6, "l");
    sg->emplace(8, "c");
    sg->emplace(15, "l");
    sg->emplace(11, "o");
    sg->emplace(9, "h");
    sg->emplace(2, "e");
    sg->emplace(4, "b");
    sg->emplace(18, "e");


    auto b = sg->lower_bound(2);
    auto e = sg->upper_bound(10);
    std::vector<typename scapegoat_tree<int, std::string>::value_type> actual_result(b, e);
    
    std::vector<typename scapegoat_tree<int, std::string>::value_type> expected_result =
        {
            { 2, "e" },
            { 4, "b" },
            { 6, "l" },
            { 8, "c" },
            { 9, "h" }
        };
    
    EXPECT_TRUE(compare_results(expected_result, actual_result));
    
    logger->trace("scapegoatTreePositiveTests.test10 finished");
}

int main(
    int argc,
    char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    
    return RUN_ALL_TESTS();
}