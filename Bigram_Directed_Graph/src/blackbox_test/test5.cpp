#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <fstream>
#include <unordered_map>
#include <queue>
#include <random>
#include <sstream>
#define INT_MAX 2147483647

using namespace std;

// -------------------- Graph 类定义 --------------------
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

    auto shortestPath(const string& src, const string& dest) -> vector<string> {
        // 最短路径算法实现（与 llm.cpp 中的代码一致）
        priority_queue<pair<int, string>, vector<pair<int, string>>, greater<>> pq;
        unordered_map<string, int> dist;
        unordered_map<string, string> prev;

        for (const auto& entry : adjList) {
            const auto& node = entry.first;
            dist[node] = INT_MAX;
        }
        dist[src] = 0;
        pq.emplace(0, src);

        while (!pq.empty()) {
            auto [d, u] = pq.top(); pq.pop();
            if (u == dest) break;
            for (auto& [v, w] : adjList[u]) {
                if (dist[v] > dist[u] + w) {
                    dist[v] = dist[u] + w;
                    prev[v] = u;
                    pq.emplace(dist[v], v);
                }
            }
        }

        vector<string> path;
        if (dist[dest] == INT_MAX) return path;
        for (string cur = dest; cur != src; cur = prev[cur]) {
            path.push_back(cur);
        }
        path.push_back(src);
        reverse(path.begin(), path.end());
        return path;
    }
};

// -------------------- TextProcessor 类定义 --------------------
class TextProcessor {
public:
    static auto processText(const string& filename) -> vector<string> {
        ifstream file(filename);
        string text;
        string line;
        while (getline(file, line)) {
            text += line + " ";
        }

        string processed;
        for (char c : text) {
            if (isalpha(c)) processed += tolower(c);
            else processed += ' ';
        }

        stringstream ss(processed);
        vector<string> words;
        string word;
        while (ss >> word) {
            words.push_back(word);
        }
        return words;
    }
};

// -------------------- 测试代码 --------------------
class ShortestPathTest : public ::testing::Test {
protected:
    Graph graph;

    void SetUp() override {
        vector<string> words = TextProcessor::processText("Easy_Test_2.txt");
        for (size_t i = 0; i < words.size() - 1; ++i) {
            graph.addEdge(words[i], words[i + 1]);
        }
    }
};


// 测试用例5：两节点均不存在
TEST_F(ShortestPathTest, BothNodesNotExist) {
    vector<string> actual = graph.shortestPath("at", "but");
    ASSERT_TRUE(actual.empty());
}



int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}