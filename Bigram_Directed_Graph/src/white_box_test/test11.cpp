#include <gtest/gtest.h>
#include <regex>
#include <vector>
#include <string>
#include <fstream>
#include <unordered_map>
#include <queue>
#include <random>
#include <sstream>


#define INT_MAX 2147483647

using namespace std;
class Graph {
private:
    unordered_map<string, unordered_map<string, int>> adjList;

public:
    void addEdge(const string& src, const string& dest) {
        adjList[src][dest]++;
    }

    auto getAdjList() const -> const unordered_map<string, unordered_map<string, int>>& {
        return adjList;
    }

    auto randomWalk() -> string {
        if (adjList.empty()) { return "";
}
        vector<string> nodes;
        for (auto& [node, _] : adjList) { nodes.push_back(node);
}
        static random_device rd;
        static mt19937 gen(rd());
        uniform_int_distribution<> dis(0, nodes.size()-1);
        
        string current = nodes[dis(gen)];
        set<pair<string, string>> visitedEdges;
        vector<string> path;
        path.push_back(current);

        while (true) {
            auto& edges = adjList[current];
            if (edges.empty()) { break;
}
            
            vector<string> targets;
            for (auto& [t, _] : edges) { targets.push_back(t);
}
            uniform_int_distribution<> edgeDis(0, targets.size()-1);
            string next = targets[edgeDis(gen)];
            
            if (visitedEdges.count({current, next}) != 0u) { break;
}
            visitedEdges.insert({current, next});
            current = next;
            path.push_back(current);
        }

        stringstream ss;
        for (auto& word : path) { ss << word << " ";
}
        
        // 将结果保存到文件
        ofstream outFile("random_walk.txt");
        if (outFile.is_open()) {
            outFile << ss.str();
            outFile.close();
        }
        
        return ss.str();
    }
};





class RandomWalkTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 在每个测试前删除可能存在的随机游走结果文件
        std::remove("random_walk.txt");
    }

    void TearDown() override {
        // 在每个测试后清理文件
        std::remove("random_walk.txt");
    }

    // 辅助函数：验证路径合法性
    bool isValidPath(const Graph& graph, const std::string& result) {
        std::istringstream iss(result);
        std::vector<std::string> path;
        std::string word;
        
        while (iss >> word) {
            path.push_back(word);
        }

        if (path.empty()) return false;

        const auto& adjList = graph.getAdjList();
        for (size_t i = 0; i < path.size() - 1; ++i) {
            if (adjList.find(path[i]) == adjList.end() ||
                adjList.at(path[i]).find(path[i + 1]) == adjList.at(path[i]).end()) {
                return false;
            }
        }
        return true;
    }
};

// 测试用例1：空图测试
TEST_F(RandomWalkTest, EmptyGraphTest) {
    Graph graph;
    std::string result = graph.randomWalk();
    
    EXPECT_EQ(result, "");
    
    // 验证不会创建文件
    std::ifstream file("random_walk.txt");
    EXPECT_FALSE(file.good());
}


int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}