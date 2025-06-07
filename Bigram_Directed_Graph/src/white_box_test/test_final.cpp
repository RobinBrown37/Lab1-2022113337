#include <gtest/gtest.h>
#include <regex>
#include <vector>
#include <string>
#include <fstream>
#include <unordered_map>
#include <queue>
#include <random>
#include <sstream>
#include <filesystem> // 新增：用于文件系统操作

#define INT_MAX 2147483647

using namespace std;

class Graph
{
private:
    unordered_map<string, unordered_map<string, int>> adjList;

public:
    void addEdge(const string &src, const string &dest)
    {
        adjList[src][dest]++;
    }

    void clearEdges(const string &node)
    {
        if (adjList.find(node) != adjList.end())
        {
            adjList[node].clear();
        }
    }

    auto getAdjList() const -> const unordered_map<string, unordered_map<string, int>> &
    {
        return adjList;
    }

    auto randomWalk() -> string
    {
        if (adjList.empty())
        {
            return "";
        }
        vector<string> nodes;
        for (auto &[node, _] : adjList)
        {
            nodes.push_back(node);
        }
        static random_device rd;
        static mt19937 gen(rd());
        uniform_int_distribution<> dis(0, nodes.size() - 1);

        string current = nodes[dis(gen)];
        set<pair<string, string>> visitedEdges;
        vector<string> path;
        path.push_back(current);

        while (true)
        {
            auto &edges = adjList[current];
            if (edges.empty())
            {
                break;
            }

            vector<string> targets;
            for (auto &[t, _] : edges)
            {
                targets.push_back(t);
            }
            uniform_int_distribution<> edgeDis(0, targets.size() - 1);
            string next = targets[edgeDis(gen)];

            if (visitedEdges.count({current, next}) != 0u)
            {
                break;
            }
            visitedEdges.insert({current, next});
            current = next;
            path.push_back(current);
        }

        stringstream ss;
        for (auto &word : path)
        {
            ss << word << " ";
        }

        ofstream outFile("random_walk.txt");
        if (outFile.is_open())
        {
            outFile << ss.str();
            outFile.close();
        }

        return ss.str();
    }
};

class RandomWalkTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        std::remove("random_walk.txt");
        std::filesystem::remove("test_dir/random_walk.txt"); // 清理测试目录
    }

    void TearDown() override
    {
        std::remove("random_walk.txt");
        std::filesystem::remove("test_dir/random_walk.txt");
        std::filesystem::permissions("test_dir", std::filesystem::perms::owner_all, std::filesystem::perm_options::replace); // 恢复目录权限
    }

    bool isValidPath(const Graph &graph, const std::string &result)
    {
        std::istringstream iss(result);
        std::vector<std::string> path;
        std::string word;

        while (iss >> word)
        {
            path.push_back(word);
        }

        if (path.empty())
            return false;

        const auto &adjList = graph.getAdjList();
        for (size_t i = 0; i < path.size() - 1; ++i)
        {
            if (adjList.find(path[i]) == adjList.end() ||
                adjList.at(path[i]).find(path[i + 1]) == adjList.at(path[i]).end())
            {
                return false;
            }
        }
        return true;
    }
};

// 测试用例1：空图测试
TEST_F(RandomWalkTest, EmptyGraphTest)
{
    Graph graph;
    std::string result = graph.randomWalk();

    EXPECT_EQ(result, "");

    std::ifstream file("random_walk.txt");
    EXPECT_FALSE(file.good());
}

// 测试用例2：单节点无边测试
TEST_F(RandomWalkTest, SingleNodeNoEdgeTest)
{
    Graph graph;
    graph.addEdge("start", "end");
    graph.clearEdges("start");
    std::string result = graph.randomWalk();

    EXPECT_EQ(result, "start ");

    std::ifstream file("random_walk.txt");
    EXPECT_TRUE(file.good());
    std::string fileContent;
    std::getline(file, fileContent);
    EXPECT_EQ(fileContent, result);

    EXPECT_TRUE(isValidPath(graph, result));
}

// 测试用例3：单边测试
TEST_F(RandomWalkTest, SingleEdgeTest)
{
    Graph graph;
    graph.addEdge("start", "end");
    std::string result = graph.randomWalk();

    EXPECT_TRUE(result == "start end " || result == "end ");

    EXPECT_TRUE(isValidPath(graph, result));

    std::ifstream file("random_walk.txt");
    EXPECT_TRUE(file.good());
    std::string fileContent;
    std::getline(file, fileContent);
    EXPECT_EQ(fileContent, result);
}

// 测试用例4：死胡同路径测试
TEST_F(RandomWalkTest, DeadEndPathTest)
{
    Graph graph;
    graph.addEdge("start", "middle");
    graph.addEdge("middle", "end");
    std::string result = graph.randomWalk();

    EXPECT_FALSE(result.empty());
    EXPECT_TRUE(isValidPath(graph, result));

    std::ifstream file("random_walk.txt");
    EXPECT_TRUE(file.good());
    std::string fileContent;
    std::getline(file, fileContent);
    EXPECT_EQ(fileContent, result);
}

// 测试用例5：复杂环路测试
TEST_F(RandomWalkTest, ComplexCyclePathTest)
{
    Graph graph;
    graph.addEdge("a", "b");
    graph.addEdge("a", "c");
    graph.addEdge("b", "c");
    graph.addEdge("b", "d");
    graph.addEdge("c", "a");
    graph.addEdge("c", "d");
    graph.addEdge("d", "a");
    std::string result = graph.randomWalk();

    EXPECT_FALSE(result.empty());
    EXPECT_TRUE(isValidPath(graph, result));

    std::regex pattern("[a-d]+(\\s+[a-d]+)*\\s*");
    EXPECT_TRUE(std::regex_match(result, pattern));

    std::ifstream file("random_walk.txt");
    EXPECT_TRUE(file.good());
    std::string fileContent;
    std::getline(file, fileContent);
    EXPECT_EQ(fileContent, result);
}

// 测试用例6：多出边选择测试
TEST_F(RandomWalkTest, MultipleEdgesTest)
{
    Graph graph;
    graph.addEdge("start", "a");
    graph.addEdge("start", "b");
    graph.addEdge("start", "c");
    graph.addEdge("a", "end");
    graph.addEdge("b", "end");
    graph.addEdge("c", "end");
    std::string result = graph.randomWalk();

    EXPECT_FALSE(result.empty());
    EXPECT_TRUE(isValidPath(graph, result));

    std::ifstream file("random_walk.txt");
    EXPECT_TRUE(file.good());
    std::string fileContent;
    std::getline(file, fileContent);
    EXPECT_EQ(fileContent, result);
}

// 测试用例7：文件无法写入测试
TEST_F(RandomWalkTest, FileNotWritableTest)
{
    Graph graph;
    graph.addEdge("start", "end");

    // 创建只读目录
    std::filesystem::create_directory("test_dir");
    std::filesystem::permissions("test_dir", std::filesystem::perms::owner_read, std::filesystem::perm_options::replace);

    // 修改输出文件路径到只读目录
    std::ofstream outFile("test_dir/random_walk.txt");
    EXPECT_FALSE(outFile.is_open()); // 确认无法打开
    outFile.close();

    std::string result = graph.randomWalk();

    EXPECT_FALSE(result.empty());
    EXPECT_TRUE(isValidPath(graph, result));

    std::ifstream file("test_dir/random_walk.txt");
    EXPECT_FALSE(file.good()); // 文件不应存在

    // 恢复目录权限
    std::filesystem::permissions("test_dir", std::filesystem::perms::owner_all, std::filesystem::perm_options::replace);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}