#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <random>
#include <cmath>
#include <chrono>
#include <thread>
#include <stack>
#include <set>
#include <iomanip>
#include <climits>
#include <unordered_set>
#include <numeric>
using namespace std;

class Graph
{
private:
    unordered_map<string, unordered_map<string, int>> adjList;
    unordered_map<string, double> pageRank;

public:
    void addEdge(const string &src, const string &dest)
    {
        adjList[src][dest]++;
    }

    auto getAdjList() const -> const unordered_map<string, unordered_map<string, int>> &
    {
        return adjList;
    }

    auto getBridgeWords(const string &word1, const string &word2) -> vector<string>
    {
        bool word1Exists = adjList.count(word1) != 0u;
        bool word2Exists = adjList.count(word2) != 0u;

        // 检查单词是否在图中，并给出具体的错误信息
        if (!word1Exists || !word2Exists)
        {
            string errorMsg;
            if (!word1Exists && !word2Exists)
            {
                errorMsg = "No \"" + word1 + "\" and \"" + word2 + "\" in the graph!";
            }
            else if (!word1Exists)
            {
                errorMsg = "No \"" + word1 + "\" in the graph!";
            }
            else
            {
                errorMsg = "No \"" + word2 + "\" in the graph!";
            }
            return {errorMsg};
        }

        vector<string> bridges;
        for (auto &[mid, _] : adjList[word1])
        {
            if (adjList[mid].count(word2) != 0u)
            {
                bridges.push_back(mid);
            }
        }

        // 如果没有找到桥接词，返回特殊消息
        if (bridges.empty())
        {
            return {"No bridge words from \"" + word1 + "\" to \"" + word2 + "\"!"};
        }

        return bridges;
    }

    auto shortestPath(const string &src, const string &dest) -> vector<string>
    {
        priority_queue<pair<int, string>, vector<pair<int, string>>, greater<>> pq;
        unordered_map<string, int> dist;
        unordered_map<string, string> prev;

        for (const auto &entry : adjList)
        {                                   // 放弃结构化绑定
            const auto &node = entry.first; // 明确初始化变量
            dist[node] = INT_MAX;
        }
        dist[src] = 0;
        pq.emplace(0, src);

        while (!pq.empty())
        {
            auto [d, u] = pq.top();
            pq.pop();
            if (u == dest)
            {
                break;
            }
            for (auto &[v, w] : adjList[u])
            {
                if (dist[v] > dist[u] + w)
                {
                    dist[v] = dist[u] + w;
                    prev[v] = u;
                    pq.emplace(dist[v], v);
                }
            }
        }

        vector<string> path;
        if (dist[dest] == INT_MAX)
        {
            return path;
        }
        for (string cur = dest; cur != src; cur = prev[cur])
        {
            path.push_back(cur);
        }
        path.push_back(src);
        reverse(path.begin(), path.end());
        return path;
    }

    void calculatePageRank(double damping = 0.85, int iterations = 100)
    {
        // Step 1: 收集所有节点（包括源节点和目标节点）
        unordered_set<string> allNodes;
        for (const auto &[src, edges] : adjList)
        {
            allNodes.insert(src);
            for (const auto &[dest, _] : edges)
            {
                allNodes.insert(dest);
            }
        }
        const double N = allNodes.size();

        // Step 2: 初始化PR值为 1/N
        unordered_map<string, double> pr;
        for (const auto &node : allNodes)
        {
            pr[node] = 1.0 / N;
        }

        // Step 3: 迭代计算
        for (int i = 0; i < iterations; ++i)
        {
            unordered_map<string, double> newPr;
            // 计算悬挂节点的PR总和（出度为0的节点）
            double danglingSum = std::accumulate(
                allNodes.begin(),
                allNodes.end(),
                // 0.0,
                [&pr](double sum, const auto &node)
                {
                    return sum + pr[node];
                });

            // 计算每个节点的新PR值
            for (const auto &node : allNodes)
            {
                double sumIn = 0.0;
                // 遍历所有可能的入边来源节点
                for (const auto &entry : adjList)
                {
                    const auto &src = entry.first;    // 显式初始化
                    const auto &edges = entry.second; // 显式初始化
                    // 使用 src 和 edges
                    if (edges.count(node) != 0u)
                    {                                    // 如果 src 指向当前节点
                        sumIn += pr[src] / edges.size(); // 贡献为 PR(src)/出度
                    }
                }

                // 计算新PR值（包含阻尼因子和悬挂节点均分）
                newPr[node] = (1.0 - damping) / N + damping * (sumIn + danglingSum / N);
            }

            pr = newPr; // 更新PR值
        }

        // 保存结果到成员变量
        pageRank.clear();
        for (const auto &node : allNodes)
        {
            pageRank[node] = pr[node];
        }
    }

    auto getPageRank(const string &word) -> double
    {
        return pageRank.count(word) != 0u ? pageRank[word] : 0.0;
    }

    auto randomWalk() -> string
    {
        if (adjList.empty())
        {
            return "";
        }
        vector<string> nodes;
        nodes.reserve(adjList.size());
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
        for (const auto &word : path)
        {
            ss << word << " ";
        }

        // 将结果保存到文件
        ofstream outFile("random_walk.txt");
        if (outFile.is_open())
        {
            outFile << ss.str();
            outFile.close();
        }

        return ss.str();
    }
};

class TextProcessor
{
public:
    static auto processText(const string &filename) -> vector<string>
    {
        ifstream file(filename);
        string text;
        string line;
        while (getline(file, line))
        {
            text += line + " ";
        }

        string processed;
        for (char c : text)
        {
            if (isalpha(c) != 0)
            {
                processed += tolower(c);
            }
            else
            {
                processed += ' ';
            }
        }

        stringstream ss(processed);
        vector<string> words;
        string word;
        while (ss >> word)
        {
            words.push_back(word);
        }
        return words;
    }
};

void exportToDot(const Graph &graph, const string &filename = "graph.dot",
                 const vector<string> &highlightPath = vector<string>())
{
    ofstream dotFile(filename);
    dotFile << "digraph G {\n";
    dotFile << "  rankdir=LR;\n"; // 设置从左到右的布局

    // 创建用于检查边是否在最短路径上的辅助函数
    auto isEdgeInPath = [&](const string &src, const string &dest)
    {
        if (highlightPath.empty())
        {
            return false;
        }
        for (size_t i = 0; i < highlightPath.size() - 1; ++i)
        {
            if (highlightPath[i] == src && highlightPath[i + 1] == dest)
            {
                return true;
            }
        }
        return false;
    };

    const auto &adjList = graph.getAdjList();
    for (const auto &[src, edges] : adjList)
    {
        for (const auto &[dest, weight] : edges)
        {
            dotFile << "  \"" << src << "\" -> \"" << dest << "\" [";
            // 如果边在最短路径上，则用红色标注
            if (isEdgeInPath(src, dest))
            {
                dotFile << "color=red, penwidth=2.0, ";
            }
            dotFile << "label=\"" << weight << "\"];\n";
        }
    }
    dotFile << "}\n";
    dotFile.close();
}

// 更新showDirectedGraph函数，添加导出选项
void showDirectedGraph(const Graph &graph, bool exportImage = false)
{
    // 原有命令行展示逻辑
    const auto &adjList = graph.getAdjList();
    cout << "Directed Graph:\n";
    for (const auto &entry : adjList)
    {
        const auto &src = entry.first;
        const auto &edges = entry.second;
        // ...
        cout << src << " -> ";
        for (const auto &edge : edges)
        {
            const auto &dest = edge.first;
            const auto &weight = edge.second;
            // ...
            cout << dest << "(" << weight << ") ";
        }

        cout << "\n";
    }

    // 导出为图像文件（可选功能）
    if (exportImage)
    {
        exportToDot(graph);
        (void)system("dot -Tpng graph.dot -o graph.png");
        cout << "\nGraph image saved to graph.png\n";
    }
}

auto queryBridgeWords(Graph &graph, const string &word1, const string &word2) -> string
{
    auto bridges = graph.getBridgeWords(word1, word2);

    // 如果是错误消息，直接返回
    if (!bridges.empty() && (bridges[0].find("No \"") != string::npos ||
                             bridges[0].find("No bridge words") != string::npos))
    {
        return bridges[0];
    }

    stringstream ss;
    ss << "The bridge words from \"" << word1 << "\" to \"" << word2 << "\" are: ";
    for (size_t i = 0; i < bridges.size(); ++i)
    {
        if (i == bridges.size() - 1)
        {
            ss << "and \"" << bridges[i] << "\".";
        }
        else if (i == bridges.size() - 2)
        {
            ss << "\"" << bridges[i] << "\" ";
        }
        else
        {
            ss << "\"" << bridges[i] << "\", ";
        }
    }
    return ss.str();
}

auto generateNewText(Graph &graph, const string &input) -> string
{
    vector<string> words;
    stringstream ss(input);
    string word;
    while (ss >> word)
    {
        transform(word.begin(), word.end(), word.begin(), ::tolower);
        words.push_back(word);
    }

    vector<string> newText;
    random_device rd;
    mt19937 gen(rd());

    for (size_t i = 0; i < words.size(); ++i)
    {
        newText.push_back(words[i]);
        if (i < words.size() - 1)
        {
            auto bridges = graph.getBridgeWords(words[i], words[i + 1]);
            // 只在找到有效的桥接词时才添加（排除所有错误消息的情况）
            if (!bridges.empty() && bridges[0].find("No ") == string::npos)
            {
                uniform_int_distribution<> dis(0, bridges.size() - 1);
                newText.push_back(bridges[dis(gen)]);
            }
        }
    }

    stringstream result;
    for (size_t i = 0; i < newText.size(); ++i)
    {
        if (i > 0)
        {
            result << " ";
        }
        result << newText[i];
    }
    return result.str();
}

auto calcShortestPath(Graph &graph, const string &word1, const string &word2) -> string
{
    auto path = graph.shortestPath(word1, word2);
    if (path.empty())
    {
        return "No path from " + word1 + " to " + word2 + "!";
    }

    // 导出带有红色标注的路径图
    exportToDot(graph, "graph.dot", path);
    system("dot -Tpng graph.dot -o graph.png");

    stringstream ss;
    ss << "Shortest path: ";
    for (size_t i = 0; i < path.size(); ++i)
    {
        if (i > 0)
        {
            ss << " -> ";
        }
        ss << path[i];
    }
    ss << "\nGraph with highlighted path has been saved to graph.png";
    return ss.str();
}

// 修改计算单源最短路径的函数
auto calcShortestPathToAll(Graph &graph, const string &source) -> string
{
    const auto &adjList = graph.getAdjList();
    if (adjList.count(source) == 0u)
    {
        return "输入的单词 '" + source + "' 不在图中！";
    }

    stringstream result;
    result << "从单词 '" << source << "' 到其他所有单词的最短路径：\n";

    vector<string> destinations;
    for (const auto &[dest, _] : adjList)
    {
        if (dest != source)
        {
            destinations.push_back(dest);
        }
    }
    sort(destinations.begin(), destinations.end());

    for (const string &dest : destinations)
    {
        auto path = graph.shortestPath(source, dest);
        result << "\n到 '" << dest << "' 的最短路径: ";
        if (path.empty())
        {
            result << "不存在路径";
        }
        else
        {
            // 为每个路径生成单独的图片
            std::string dotFile("path_");
            dotFile.reserve(32); // 预分配合理空间（按实际需要调整）
            dotFile.append(source)
                .append("_to_")
                .append(dest)
                .append(".dot");
            std::string pngFile("path_");
            pngFile.reserve(32); // 预分配合理空间
            pngFile.append(source)
                .append("_to_")
                .append(dest)
                .append(".png");
            exportToDot(graph, dotFile, path);
            std::string cmd("dot -Tpng ");
            cmd.reserve(128); // 预分配合理空间
            cmd.append(dotFile)
                .append(" -o ")
                .append(pngFile);
            system(cmd.c_str());

            for (size_t i = 0; i < path.size(); ++i)
            {
                if (i > 0)
                {
                    result << " -> ";
                }
                result << path[i];
            }
            result << " (查看 " << pngFile << ")";
        }
    }

    return result.str();
}

auto calcPageRank(Graph &graph, const string &word) -> double
{
    return graph.getPageRank(word);
}

auto randomWalk(Graph &graph) -> string
{
    return graph.randomWalk();
}

auto main(int argc, const char *argv[]) -> int
{
    if (argc < 2)
    {
        cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    auto words = TextProcessor::processText(argv[1]);
    Graph graph;
    for (size_t i = 0; i < words.size() - 1; ++i)
    {
        graph.addEdge(words[i], words[i + 1]);
    }

    graph.calculatePageRank();

    while (true)
    {
        cout << "\nOptions:\n"
             << "1. Show directed graph\n"
             << "2. Export graph to image (PNG)\n"
             << "3. Query bridge words\n"
             << "4. Generate new text\n"
             << "5. Calculate shortest path\n"
             << "6. Calculate PageRank\n"
             << "7. Random walk\n"
             << "8. Exit\n"
             << "Choice: ";

        int choice;
        cin >> choice;
        cin.ignore();

        switch (choice)
        {
        case 1:
            showDirectedGraph(graph);
            break;
        case 2:
            showDirectedGraph(graph, true); // 导出为图像
            break;
        case 3:
        {
            string word1;
            string word2;
            cout << "Enter two words: ";
            cin >> word1 >> word2;
            transform(word1.begin(), word1.end(), word1.begin(), ::tolower);
            transform(word2.begin(), word2.end(), word2.begin(), ::tolower);
            cout << queryBridgeWords(graph, word1, word2) << '\n';
            break;
        }
        case 4:
        {
            string input;
            cout << "Enter text: ";
            getline(cin, input);
            cout << generateNewText(graph, input) << '\n';
            break;
        }
        case 5:
        {
            string word1;
            string word2;
            cout << "Enter one or two words: ";
            cin >> word1;
            transform(word1.begin(), word1.end(), word1.begin(), ::tolower);

            // 检查是否有第二个单词输入
            string line;
            getline(cin, line);
            stringstream ss(line);
            if (ss >> word2)
            {
                // 如果输入了两个单词，计算它们之间的最短路径
                transform(word2.begin(), word2.end(), word2.begin(), ::tolower);
                cout << calcShortestPath(graph, word1, word2) << '\n';
            }
            else
            {
                // 如果只输入了一个单词，计算到所有其他单词的最短路径
                cout << calcShortestPathToAll(graph, word1) << '\n';
            }
            break;
        }
        case 6:
        {
            string word;
            cout << "Enter word: ";
            cin >> word;
            transform(word.begin(), word.end(), word.begin(), ::tolower);
            cout << "PageRank: " << fixed << setprecision(4) << calcPageRank(graph, word) << '\n';
            break;
        }
        case 7:
            cout << "Random walk: " << randomWalk(graph) << '\n';
            break;
        case 8:
            return 0;
        default:
            cout << "Invalid choice\n";
        }
    }
}