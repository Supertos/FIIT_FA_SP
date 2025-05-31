#include "gtest/gtest.h"
#include <red_black_tree.h>
#include <logger_builder.h>
#include <client_logger_builder.h>
#include <iostream>
#include <map>


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
    red_black_tree<tkey, tvalue>::node_color color;

    test_data(size_t dep, tkey k, tvalue v, red_black_tree<tkey, tvalue>::node_color c) : depth(dep), key(k), value(v), color(c) {}
};

template<typename tkey, typename tvalue>
bool infix_iterator_test(
        red_black_tree<tkey, tvalue> const &tree,
        std::vector<test_data<tkey, tvalue>> &expected_result)
{
    auto end_infix = tree.cend_infix();
    auto it = tree.cbegin_infix();

    auto vec_begin = expected_result.begin(), vec_end = expected_result.end();

    for (; vec_begin != vec_end; ++vec_begin)
    {
        auto &item = *vec_begin;
        if (it.depth() != item.depth || it->first != item.key || it->second != item.value ||
            it.get_color() != item.color)
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
        red_black_tree<tkey, tvalue> const &tree,
        std::vector<test_data<tkey, tvalue>> &expected_result)
{
    auto end_prefix = tree.cend_prefix();
    auto it = tree.cbegin_prefix();

    for (auto const &item: expected_result)
    {
        if (it.depth() != item.depth || it->first != item.key || it->second != item.value ||
                it.get_color() != item.color)
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
        red_black_tree<tkey, tvalue> const &tree,
        std::vector<test_data<tkey, tvalue>> &expected_result)
{
    std::string line;
    auto end_postfix = tree.cend_postfix();
    auto it = tree.cbegin_postfix();

    for (auto &item: expected_result)
    {
        if (it.depth() != item.depth || it->first != item.key || it->second != item.value ||
                it.get_color() != item.color)
        {
            return false;
        }

        ++it;
    }

    return true;
}

TEST(redBlackTreePositiveTests, test1)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
        {
            {
                "red_black_tree_tests_logs.txt",
                logger::severity::trace
            },
        }));
    
    logger->trace("redBlackTreePositiveTests.test1 started");
    
    auto rb = std::make_unique<red_black_tree<int, std::string>>(std::less<int>(), nullptr, logger.get());
    
    rb->emplace(5, "a");
    rb->emplace(2, "b");
    rb->emplace(15, "c");
    rb->emplace(3, "d");
    rb->emplace(14, "e");
    rb->emplace(1, "l");
    
    std::vector<test_data<int, std::string>> expected_result =
        {
            test_data<int, std::string>(2, 1, "l", red_black_tree<int, std::string>::node_color::RED),
            test_data<int, std::string>(1, 2, "b", red_black_tree<int, std::string>::node_color::BLACK),
            test_data<int, std::string>(2, 3, "d", red_black_tree<int, std::string>::node_color::RED),
            test_data<int, std::string>(0, 5, "a", red_black_tree<int, std::string>::node_color::BLACK),
            test_data<int, std::string>(2, 14, "e", red_black_tree<int, std::string>::node_color::RED),
            test_data<int, std::string>(1, 15, "c", red_black_tree<int, std::string>::node_color::BLACK)
        };
    
    EXPECT_TRUE(infix_iterator_test(*rb, expected_result));
    
    logger->trace("redBlackTreePositiveTests.test1 finished");
}

TEST(redBlackTreePositiveTests, test2)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "red_black_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("redBlackTreePositiveTests.test2 started");

    auto rb = std::make_unique<red_black_tree<int, int>>(std::less<int>(), nullptr, logger.get());
    
    rb->emplace(1, 5);
    rb->emplace(2, 12);
    rb->emplace(15, 1);
    rb->emplace(3, 67);
    rb->emplace(4, 45);
    
    std::vector<test_data<int, int>> expected_result =
        {
                test_data<int, int>(0, 2, 12, red_black_tree<int, int>::node_color::BLACK),
                test_data<int, int>(1, 1, 5, red_black_tree<int, int>::node_color::BLACK),
                test_data<int, int>(1, 4, 45, red_black_tree<int, int>::node_color::BLACK),
                test_data<int, int>(2, 3, 67, red_black_tree<int, int>::node_color::RED),
                test_data<int, int>(2, 15, 1, red_black_tree<int, int>::node_color::RED)
        };
    
    EXPECT_TRUE(prefix_iterator_test(*rb, expected_result));
    
    logger->trace("redBlackTreePositiveTests.test2 finished");
}

TEST(redBlackTreePositiveTests, test3)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "red_black_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("redBlackTreePositiveTests.test3 started");

    auto rb = std::make_unique<red_black_tree<std::string, int>>(std::less<std::string>(), nullptr, logger.get());
    
    rb->emplace("a", 1);
    rb->emplace("b", 2);
    rb->emplace("c", 15);
    rb->emplace("d", 3);
    rb->emplace("e", 4);
    
    std::vector<test_data<std::string, int>> expected_result =
        {
                test_data<std::string, int>(1, "a", 1, red_black_tree<std::string, int>::node_color::BLACK),
                test_data<std::string, int>(2, "c", 15, red_black_tree<std::string, int>::node_color::RED),
                test_data<std::string, int>(2, "e", 4, red_black_tree<std::string, int>::node_color::RED),
                test_data<std::string, int>(1, "d", 3, red_black_tree<std::string, int>::node_color::BLACK),
                test_data<std::string, int>(0, "b", 2, red_black_tree<std::string, int>::node_color::BLACK)
        };
    
    EXPECT_TRUE(postfix_iterator_test(*rb, expected_result));
    
    logger->trace("redBlackTreePositiveTests.test3 finished");
}

TEST(redBlackTreePositiveTests, test4)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "red_black_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("redBlackTreePositiveTests.test4 started");

    auto rb1 = std::make_unique<red_black_tree<int, std::string>>(std::less<int>(), nullptr, logger.get());
    
    rb1->emplace(6, "a");
    rb1->emplace(8, "c");
    rb1->emplace(15, "x");
    rb1->emplace(4, "j");
    rb1->emplace(1, "i");
    rb1->emplace(5, "b");
    
    std::vector<test_data<int, std::string>> expected_result =
        {
            test_data<int, std::string>(2, 1, "i", red_black_tree<int, std::string>::node_color::BLACK),
            test_data<int, std::string>(1, 4, "j", red_black_tree<int, std::string>::node_color::RED),
            test_data<int, std::string>(3, 5, "b", red_black_tree<int, std::string>::node_color::RED),
            test_data<int, std::string>(2, 6, "a", red_black_tree<int, std::string>::node_color::BLACK),
            test_data<int, std::string>(0, 8, "c", red_black_tree<int, std::string>::node_color::BLACK),
            test_data<int, std::string>(1, 15, "x", red_black_tree<int, std::string>::node_color::BLACK)
        };
    
    red_black_tree<int, std::string> rb2(
        std::move(*rb1));
    
    EXPECT_TRUE(infix_iterator_test(rb2, expected_result));
    
    logger->trace("redBlackTreePositiveTests.test4 finished");
}

TEST(redBlackTreePositiveTests, test5)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "red_black_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("redBlackTreePositiveTests.test5 started");

    auto rb1 = std::make_unique<red_black_tree<int, std::string>>(std::less<int>(), nullptr, logger.get());
    
    rb1->emplace(6, "a");
    rb1->emplace(8, "c");
    rb1->emplace(15, "x");
    rb1->emplace(4, "j");
    rb1->emplace(1, "i");
    rb1->emplace(5, "b");
    
    std::vector<test_data<int, std::string>> expected_result =
        {
            test_data<int, std::string>(2, 1, "i", red_black_tree<int, std::string>::node_color::BLACK),
            test_data<int, std::string>(1, 4, "j", red_black_tree<int, std::string>::node_color::RED),
            test_data<int, std::string>(3, 5, "b", red_black_tree<int, std::string>::node_color::RED),
            test_data<int, std::string>(2, 6, "a", red_black_tree<int, std::string>::node_color::BLACK),
            test_data<int, std::string>(0, 8, "c", red_black_tree<int, std::string>::node_color::BLACK),
            test_data<int, std::string>(1, 15, "x", red_black_tree<int, std::string>::node_color::BLACK)
        };
    
    red_black_tree<int, std::string> rb2 = std::move(*rb1);
    
    EXPECT_TRUE(infix_iterator_test(rb2, expected_result));
    
    logger->trace("redBlackTreePositiveTests.test5 finished");
}

TEST(redBlackTreePositiveTests, test6)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "red_black_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("redBlackTreePositiveTests.test6 started");

    auto rb1 = std::make_unique<red_black_tree<int, std::string>>(std::less<int>(), nullptr, logger.get());
    
    rb1->emplace(6, "a");
    rb1->emplace(8, "c");
    rb1->emplace(15, "x");
    rb1->emplace(4, "j");
    rb1->emplace(1, "i");
    rb1->emplace(5, "b");
    
    rb1->erase(5);
    
    std::vector<test_data<int, std::string>> expected_result =
        {
            test_data<int, std::string>(2, 1, "i", red_black_tree<int, std::string>::node_color::BLACK),
            test_data<int, std::string>(1, 4, "j", red_black_tree<int, std::string>::node_color::RED),
            test_data<int, std::string>(2, 6, "a", red_black_tree<int, std::string>::node_color::BLACK),
            test_data<int, std::string>(0, 8, "c", red_black_tree<int, std::string>::node_color::BLACK),
            test_data<int, std::string>(1, 15, "x", red_black_tree<int, std::string>::node_color::BLACK)
        };
    
    EXPECT_TRUE(infix_iterator_test(*rb1, expected_result));
    
    logger->trace("redBlackTreePositiveTests.test6 finished");
}

TEST(redBlackTreePositiveTests, test7)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "red_black_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("redBlackTreePositiveTests.test7 started");

    auto rb1 = std::make_unique<red_black_tree<int, std::string>>(std::less<int>(), nullptr, logger.get());
    
    rb1->emplace(6, "a");
    rb1->emplace(8, "c");
    rb1->emplace(15, "x");
    rb1->emplace(4, "j");
    rb1->emplace(3, "i");
    rb1->emplace(2, "l");
    rb1->emplace(5, "b");
    
    rb1->erase(3);
    
    std::vector<test_data<int, std::string>> expected_result =
        {
            test_data<int, std::string>(2, 2, "l", red_black_tree<int, std::string>::node_color::BLACK),
            test_data<int, std::string>(1, 4, "j", red_black_tree<int, std::string>::node_color::RED),
            test_data<int, std::string>(3, 5, "b", red_black_tree<int, std::string>::node_color::RED),
            test_data<int, std::string>(2, 6, "a", red_black_tree<int, std::string>::node_color::BLACK),
            test_data<int, std::string>(0, 8, "c", red_black_tree<int, std::string>::node_color::BLACK),
            test_data<int, std::string>(1, 15, "x", red_black_tree<int, std::string>::node_color::BLACK)
        };
    
    EXPECT_TRUE(infix_iterator_test(*rb1, expected_result));
    
    logger->trace("redBlackTreePositiveTests.test7 finished");
}

TEST(redBlackTreePositiveTests, test8)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "red_black_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("redBlackTreePositiveTests.test8 started");

    auto rb1 = std::make_unique<red_black_tree<int, std::string>>(std::less<int>(), nullptr, logger.get());
    
    rb1->emplace(6, "a");
    rb1->emplace(8, "c");
    rb1->emplace(15, "x");
    rb1->emplace(11, "j");
    rb1->emplace(19, "i");
    rb1->emplace(12, "l");
    rb1->emplace(17, "b");
    rb1->emplace(18, "e");
    
    rb1->erase(15);
    
    std::vector<test_data<int, std::string>> expected_result =
        {
                test_data<int, std::string>(1, 6, "a", red_black_tree<int, std::string>::node_color::BLACK),
                test_data<int, std::string>(0, 8, "c", red_black_tree<int, std::string>::node_color::BLACK),
                test_data<int, std::string>(2, 11, "j", red_black_tree<int, std::string>::node_color::BLACK),
                test_data<int, std::string>(1, 12, "l", red_black_tree<int, std::string>::node_color::RED),
                test_data<int, std::string>(3, 17, "b", red_black_tree<int, std::string>::node_color::RED),
                test_data<int, std::string>(2, 18, "e", red_black_tree<int, std::string>::node_color::BLACK),
            test_data<int, std::string>(3, 19, "i", red_black_tree<int, std::string>::node_color::RED)
        };
    
    EXPECT_TRUE(infix_iterator_test(*rb1, expected_result));
    
    logger->trace("redBlackTreePositiveTests.test8 finished");
}

TEST(redBlackTreePositiveTests, test9)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "red_black_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("redBlackTreePositiveTests.test9 started");

    auto rb1 = std::make_unique<red_black_tree<int, std::string>>(std::less<int>(), nullptr, logger.get());
    
    rb1->emplace(6, "a");
    rb1->emplace(8, "c");
    rb1->emplace(15, "x");
    rb1->emplace(11, "j");
    rb1->emplace(19, "i");
    rb1->emplace(12, "l");
    rb1->emplace(17, "b");
    rb1->emplace(18, "e");
    
    rb1->erase(11);
    
    std::vector<test_data<int, std::string>> expected_result =
        {
                test_data<int, std::string>(1, 6, "a", red_black_tree<int, std::string>::node_color::BLACK),
                test_data<int, std::string>(0, 8, "c", red_black_tree<int, std::string>::node_color::BLACK),
                test_data<int, std::string>(2, 12, "l", red_black_tree<int, std::string>::node_color::BLACK),
                test_data<int, std::string>(1, 15, "x", red_black_tree<int, std::string>::node_color::RED),
                test_data<int, std::string>(3, 17, "b", red_black_tree<int, std::string>::node_color::RED),
                test_data<int, std::string>(2, 18, "e", red_black_tree<int, std::string>::node_color::BLACK),
                test_data<int, std::string>(3, 19, "i", red_black_tree<int, std::string>::node_color::RED)
        };
    
    EXPECT_TRUE(infix_iterator_test(*rb1, expected_result));
    
    logger->trace("redBlackTreePositiveTests.test9 finished");
}

TEST(redBlackTreePositiveTests, test10)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "red_black_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("redBlackTreePositiveTests.test10 started");

    auto rb1 = std::make_unique<red_black_tree<int, std::string>>(std::less<int>(), nullptr, logger.get());
    
    rb1->emplace(6, "l");
    rb1->emplace(8, "c");
    rb1->emplace(15, "l");
    rb1->emplace(11, "o");
    rb1->emplace(9, "h");
    rb1->emplace(2, "e");
    rb1->emplace(4, "b");
    rb1->emplace(18, "e");
    
    std::vector<std::string> vector;
    
    vector.push_back(rb1->at(9));
    vector.push_back(rb1->at(2));
    vector.push_back(rb1->at(15));
    vector.push_back(rb1->at(6));
    vector.push_back(rb1->at(11));
    
    std::string actual_result;
    
    for (auto const &item: vector)
    {
        actual_result.append(item + " ");
    }
    
    EXPECT_EQ(actual_result, "h e l l o ");
    
    logger->trace("redBlackTreePositiveTests.test10 finished");
}

TEST(redBlackTreePositiveTests, test11)
{
    std::unique_ptr<logger> logger(create_logger(std::vector<std::pair<std::string, logger::severity>>
                                                         {
                                                                 {
                                                                         "red_black_tree_tests_logs.txt",
                                                                         logger::severity::trace
                                                                 },
                                                         }));
    
    logger->trace("redBlackTreePositiveTests.test11 started");

    auto rb = std::make_unique<red_black_tree<int, std::string>>(std::less<int>(), nullptr, logger.get());
    
    rb->emplace(6, "l");
    rb->emplace(8, "c");
    rb->emplace(15, "l");
    rb->emplace(11, "o");
    rb->emplace(9, "h");
    rb->emplace(2, "e");
    rb->emplace(4, "b");
    rb->emplace(18, "e");

    auto b = rb->lower_bound(2);
    auto e = rb->upper_bound(10);

    std::vector<typename red_black_tree<int, std::string>::value_type> actual_result(b, e);
    
    std::vector<typename red_black_tree<int, std::string>::value_type> expected_result =
        {
            { 2, "e" },
            { 4, "b" },
            { 6, "l" },
            { 8, "c" },
            { 9, "h" }
        };
    
    EXPECT_TRUE(compare_results(expected_result, actual_result));
    
    logger->trace("redBlackTreePositiveTests.test11 finished");
}

TEST(redBlackTreePositiveTests, test17)
{
	red_black_tree<int, int> tree{std::less<int>()};
	std::map<int, int> map;

	size_t iterations = 100'000;

	srand(time(nullptr));

	for(size_t i = 0; i < iterations; ++i)
	{
		switch(rand() % 3)
		{
			case 0:
			case 1:
			{
				int tmp = rand();
				try
				{

					if (map.find(tmp) == map.end())
					{
						map.insert(std::make_pair(tmp, 1));
						tree.emplace(tmp, 1);
					}
				} catch (std::logic_error& er)
				{
					std::cout << er.what() << std::endl;
				}
				break;
			}
			case 2:
			{
				if (!map.empty())
				{
					auto it = map.begin();

					//std::advance(it, rand() % map.size());

					tree.erase(it->first);
					map.erase(it);
				} else
				{
					std::cout << "Empty" << std::endl;
				}
			}
				break;
		}
	}

	while(!map.empty())
	{
		auto it = map.begin();
		//std::advance(it, rand() % map.size());

		tree.erase(it->first);
		map.erase(it);
	}

}


int main(
    int argc,
    char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    
    return RUN_ALL_TESTS();
}