#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <queue>
#include <set>
#include <random>
#include <chrono>
#include <windows.h>

using namespace std;

class Graph {
private:
    unordered_map<string, unordered_map<string, int>> adjList;
    unordered_map<string, double> pageRank;
    unordered_map<string, vector<string>> predecessors;

    string toLower(const string& s) {
        string res;
        for (char c : s) res += tolower(c);
        return res;
    }

public:
    void addEdge(const string& from, const string& to) {
        string f = toLower(from);
        string t = toLower(to);
        if (f != t) adjList[f][t]++;
    }

    void buildGraphFromText(const vector<string>& words) {
        for (size_t i = 0; i < words.size() - 1; ++i) {
            addEdge(words[i], words[i + 1]);
        }
        buildPredecessors();
    }

    bool generateDotFile(const string& baseName) {
        // Use backslashes for Windows compatibility
        string dotFileName = "img\\" + baseName + ".dot";
        string pngFileName = "img\\" + baseName + ".dot.png";
        
        ofstream dotFile(dotFileName);
        if (!dotFile.is_open()) {
            cout << "Error: Could not create file " << dotFileName << endl;
            return false;
        }
        dotFile << "digraph G {\n";
        for (const auto& [from, edges] : adjList) {
            for (const auto& [to, weight] : edges) {
                dotFile << "  \"" << from << "\" -> \"" << to 
                        << "\" [label=\"" << weight << "\"];\n";
            }
        }
        dotFile << "}\n";
        dotFile.close();

        // Use backslashes in the command and quote the paths
        string command = "dot -Tpng \"" + dotFileName + "\" -o \"" + pngFileName + "\"";
        int result = system(command.c_str());
        if (result != 0) {
            cout << "Error: Failed to generate PNG file. Ensure 'dot' (Graphviz) is installed and in your PATH." << endl;
            return false;
        }
        return true;
    }

    string getBridgeWords(const string& word1, const string& word2) {
        string w1 = toLower(word1), w2 = toLower(word2);
        vector<string> bridges;
        
        if (adjList.find(w1) == adjList.end() || adjList.find(w2) == adjList.end()) {
            string missing = (adjList.find(w1) == adjList.end() && adjList.find(w2) == adjList.end())
                            ? (w1 + " and " + w2)
                            : (adjList.find(w1) == adjList.end() ? w1 : w2);
            return "No " + missing + " in the graph!";
        }

        for (const auto& [mid, _] : adjList) {
            if (adjList[w1].count(mid) && adjList[mid].count(w2)) {
                bridges.push_back(mid);
            }
        }

        if (bridges.empty()) {
            return "No bridge words from " + w1 + " to " + w2 + "!";
        }

        string result = "The bridge words from " + w1 + " to " + w2 + " are: ";
        for (size_t i = 0; i < bridges.size(); ++i) {
            if (i > 0) {
                if (i == bridges.size() - 1) result += "and ";
                else result += ", ";
            }
            result += "\"" + bridges[i] + "\"";
        }
        result += ".";
        return result;
    }

    string generateNewText(const string& input) {
        vector<string> words;
        string processed;
        for (char c : input) {
            processed += isalpha(c) ? tolower(c) : ' ';
        }
        istringstream iss(processed);
        string word;
        while (iss >> word) words.push_back(word);

        random_device rd;
        mt19937 gen(rd());
        
        string result;
        for (size_t i = 0; i < words.size(); ++i) {
            result += words[i];
            if (i < words.size() - 1) {
                string bridgesStr = getBridgeWords(words[i], words[i + 1]);
                vector<string> bridges;
                if (bridgesStr.find("The bridge words") == 0) {
                    size_t start = bridgesStr.find('"');
                    while (start != string::npos) {
                        size_t end = bridgesStr.find('"', start + 1);
                        if (end != string::npos) {
                            bridges.push_back(bridgesStr.substr(start + 1, end - start - 1));
                            start = bridgesStr.find('"', end + 1);
                        } else break;
                    }
                }
                
                if (!bridges.empty()) {
                    uniform_int_distribution<> dis(0, bridges.size() - 1);
                    result += " " + bridges[dis(gen)] + " ";
                } else {
                    result += " ";
                }
            }
        }
        return result;
    }

    vector<string> shortestPath(const string& src, const string& dest) {
        string s = toLower(src), d = toLower(dest);
        priority_queue<pair<int, string>, vector<pair<int, string>>, greater<>> pq;
        unordered_map<string, int> dist;
        unordered_map<string, string> prev;

        for (const auto& [node, _] : adjList) dist[node] = INT_MAX;
        dist[s] = 0;
        pq.push({0, s});

        while (!pq.empty()) {
            auto [cost, u] = pq.top(); pq.pop();
            if (u == d) break;
            if (cost > dist[u]) continue;

            for (const auto& [v, w] : adjList[u]) {
                if (dist[v] > dist[u] + w) {
                    dist[v] = dist[u] + w;
                    prev[v] = u;
                    pq.push({dist[v], v});
                }
            }
        }

        vector<string> path;
        if (dist[d] == INT_MAX) return path;
        for (string at = d; !at.empty(); at = prev[at]) {
            path.push_back(at);
        }
        reverse(path.begin(), path.end());
        return path;
    }

    unordered_map<string, pair<vector<string>, int>> shortestPathsToAll(const string& src) {
        string s = toLower(src);
        
        if (adjList.find(s) == adjList.end()) {
            return {};
        }
        
        priority_queue<pair<int, string>, vector<pair<int, string>>, greater<>> pq;
        unordered_map<string, int> dist;
        unordered_map<string, string> prev;

        for (const auto& [node, _] : adjList) dist[node] = INT_MAX;
        dist[s] = 0;
        pq.push({0, s});

        while (!pq.empty()) {
            auto [cost, u] = pq.top(); pq.pop();
            if (cost > dist[u]) continue;

            for (const auto& [v, w] : adjList[u]) {
                if (dist[v] > dist[u] + w) {
                    dist[v] = dist[u] + w;
                    prev[v] = u;
                    pq.push({dist[v], v});
                }
            }
        }

        unordered_map<string, pair<vector<string>, int>> paths;
        for (const auto& [node, distance] : dist) {
            if (node == s || distance == INT_MAX) continue;
            
            vector<string> path;
            for (string at = node; !at.empty(); at = prev[at]) {
                path.push_back(at);
            }
            reverse(path.begin(), path.end());
            
            paths[node] = {path, distance};
        }

        return paths;
    }

    unordered_map<string, double> computeTFInitialPR(const vector<string>& words) {
        unordered_map<string, double> tf;
        double totalWords = words.size();
        
        for (const string& word : words) {
            string w = toLower(word);
            tf[w]++;
        }
        
        for (auto& [word, count] : tf) {
            count /= totalWords;
        }
        
        unordered_map<string, double> initialPR;
        for (const auto& [node, edges] : adjList) {
            double tfVal = tf.count(node) ? tf[node] : 0.0;
            double indegree = predecessors.count(node) ? predecessors[node].size() : 0;
            double sinkBoost = 0.0;
            
            for (const auto& [target, _] : edges) {
                if (adjList[target].empty()) {
                    sinkBoost += 1.0;
                }
            }
            
            double boostFactor = 1.0 + (indegree * 0.3) + (sinkBoost * 0.5);
            initialPR[node] = tfVal * boostFactor;
        }
        
        double sumPR = 0.0;
        for (const auto& [_, pr] : initialPR) {
            sumPR += pr;
        }
        
        if (sumPR > 0) {
            for (auto& [_, pr] : initialPR) {
                pr /= sumPR;
            }
        }
        
        return initialPR;
    }

    void computePageRank(double d = 0.85, int maxIter = 100, const unordered_map<string, double>* initialPR = nullptr) {
        int N = adjList.size();
        
        if (initialPR) {
            pageRank = *initialPR;
            for (const auto& [node, _] : adjList) {
                if (pageRank.find(node) == pageRank.end()) {
                    pageRank[node] = 0.0;
                }
            }
        } else {
            for (const auto& [node, _] : adjList) {
                pageRank[node] = 1.0 / N;
            }
        }

        for (int iter = 0; iter < maxIter; ++iter) {
            unordered_map<string, double> newPR;
            double zeroOutDegreeSum = 0.0;
            
            for (const auto& [node, edges] : adjList) {
                if (edges.empty()) {
                    zeroOutDegreeSum += pageRank[node];
                }
            }
            
            // double zeroOutDegreePortion = 0.0;
            // if (N > 1) {
            //     zeroOutDegreePortion = zeroOutDegreeSum / (N - 1);
            // }
            
            for (const auto& [u, _] : adjList) {
                double sum = 0.0;
                if (predecessors.find(u) != predecessors.end()) {
                    for (const string& v : predecessors[u]) {
                        int L_v = adjList[v].size();
                        if (L_v > 0) {
                            sum += pageRank[v] / L_v;
                        }
                    }
                }
                
                newPR[u] = (1.0 - d) / N + d * sum;
                
                if (zeroOutDegreeSum > 0 && adjList.size() > 1) {
                    if (adjList[u].empty()) {
                        newPR[u] += d * (zeroOutDegreeSum - pageRank[u]) / (N - 1);
                    } else {
                        newPR[u] += d * zeroOutDegreeSum / (N - 1);
                    }
                }
            }
            pageRank = newPR;
        }
    }

    string randomWalk(const string& baseName) {
    if (adjList.empty()) return "";
    
    // Read the last used starting node from a file
    string lastNodeFile = "img\\" + baseName + "_last_node.txt";
    string lastUsedNode = "";
    
    ifstream inFile(lastNodeFile);
    if (inFile.is_open()) {
        getline(inFile, lastUsedNode);
        inFile.close();
    }
    
    // Seed the random generator with current time
    // auto seed = chrono::high_resolution_clock::now().time_since_epoch().count();
    std::random_device rd;
    auto time_seed = chrono::high_resolution_clock::now().time_since_epoch().count();
    mt19937 gen(rd() ^ static_cast<unsigned int>(time_seed));
    
    // Collect all nodes except the last used one
    vector<string> nodes;
    for (const auto& [node, _] : adjList) {
        if (node != lastUsedNode) {
            nodes.push_back(node);
        }
    }
    
    // If we have no available nodes (only had one node), reset
    if (nodes.empty()) {
        for (const auto& [node, _] : adjList) {
            nodes.push_back(node);
        }
        cout << "Only one node in graph, will reuse it" << endl;
    }
    
    // Select a random node from available nodes
    uniform_int_distribution<> dis(0, nodes.size() - 1);
    string current = nodes[dis(gen)];
    
    // Save this node as the last used node
    ofstream outFile(lastNodeFile);
    if (outFile.is_open()) {
        outFile << current;
        outFile.close();
    }
    
    set<pair<string, string>> visitedEdges;
    vector<string> walk = {current};
    
    cout << "Starting random walk from node: " << current << endl;

    while (true) {
        if (adjList[current].empty()) {
            cout << "Reached node with no outgoing edges." << endl;
            break;
        }
        
        vector<string> candidates;
        for (const auto& [next, _] : adjList[current]) 
            candidates.push_back(next);
        
        uniform_int_distribution<> nextDis(0, candidates.size() - 1);
        string next = candidates[nextDis(gen)];
        
        if (visitedEdges.count({current, next})) break;
        visitedEdges.insert({current, next});
        
        walk.push_back(next);
        current = next;
    }
    
    string outputFile = "img\\" + baseName + "_random_walk.txt";
    ofstream out(outputFile);
    string result;
    for (size_t i = 0; i < walk.size(); ++i) {
        result += walk[i];
        if (i != walk.size() - 1) result += " ";
    }
    out << result;
    out.close();
    return result;
}

    const unordered_map<string, double>& getPageRank() const { return pageRank; }
    
    bool hasNode(const string& word) {
        string w = toLower(word);
        return adjList.find(w) != adjList.end();
    }

private:
    void buildPredecessors() {
        predecessors.clear();
        for (const auto& [from, edges] : adjList) {
            for (const auto& [to, _] : edges) {
                predecessors[to].push_back(from);
            }
        }
    }
};

vector<string> processTextFile(const string& filename) {
    ifstream file(filename);
    string text((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    
    transform(text.begin(), text.end(), text.begin(),
        [](char c) { return isalpha(c) ? tolower(c) : ' '; });
    
    vector<string> words;
    istringstream iss(text);
    string word;
    while (iss >> word) words.push_back(word);
    return words;
}

void printPath(const vector<string>& path) {
    if (path.empty()) {
        cout << "No path exists!" << endl;
        return;
    }
    for (size_t i = 0; i < path.size(); ++i) {
        cout << path[i];
        if (i != path.size() - 1) cout << " -> ";
    }
    cout << endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <input_file>\n";
        return 1;
    }

    // Extract the base name of the input file (without path or extension)
    string inputFile = argv[1];
    string baseName = inputFile;
    size_t lastSlash = inputFile.find_last_of("\\/");
    if (lastSlash != string::npos) {
        baseName = inputFile.substr(lastSlash + 1);
    }
    size_t lastDot = baseName.find_last_of(".");
    if (lastDot != string::npos) {
        baseName = baseName.substr(0, lastDot);
    }

    // Create the img directory if it doesn't exist
    if (!CreateDirectoryA("img", NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
        cout << "Error: Could not create 'img' directory." << endl;
        return 1;
    }

    vector<string> words = processTextFile(argv[1]);
    Graph g;
    g.buildGraphFromText(words);
    if (!g.generateDotFile(baseName)) {
        cout << "Failed to generate graph visualization." << endl;
    }

    unordered_map<string, double> initialPR = g.computeTFInitialPR(words);
    g.computePageRank(0.85, 100, &initialPR);

    while (true) {
        cout << "\n1. Show Graph\n2. Query Bridge Words\n3. Generate New Text\n"
                "4. Shortest Path\n5. Show PageRank\n6. Random Walk\n7. Exit\n> ";
        int choice;
        cin >> choice;
        
        if (choice == 7) break;
        
        switch(choice) {
            case 1: {
                string pngFile = "img\\" + baseName + ".dot.png";
                // Check if the file exists
                ifstream fileCheck(pngFile);
                if (!fileCheck.good()) {
                    cout << "Error: The graph image file " << pngFile << " does not exist." << endl;
                    cout << "Ensure Graphviz 'dot' is installed and the program has write permissions in the 'img' directory." << endl;
                } else {
                    ShellExecuteA(NULL, "open", pngFile.c_str(), NULL, NULL, SW_SHOW);
                }
                fileCheck.close();
                break;
            }
            case 2: {
                string w1, w2;
                cout << "Enter two words: ";
                cin >> w1 >> w2;
                cout << g.getBridgeWords(w1, w2) << endl;
                break;
            }
            case 3: {
                cin.ignore();
                cout << "Enter text: ";
                string input;
                getline(cin, input);
                cout << "Generated text: " << g.generateNewText(input) << endl;
                break;
            }
            case 4: {
                string src, dest;
                cout << "Enter source word: ";
                cin >> src;
                
                if (!g.hasNode(src)) {
                    cout << "The word \"" << src << "\" is not in the graph!" << endl;
                    break;
                }
                
                cout << "Enter destination word (or press Enter to find paths to all words): ";
                cin.ignore();
                getline(cin, dest);
                
                if (dest.empty()) {
                    auto allPaths = g.shortestPathsToAll(src);
                    if (allPaths.empty()) {
                        cout << "No paths found from \"" << src << "\" to any other word." << endl;
                    } else {
                        vector<pair<string, pair<vector<string>, int>>> sortedPaths(allPaths.begin(), allPaths.end());
                        sort(sortedPaths.begin(), sortedPaths.end(), 
                             [](const auto& a, const auto& b) { return a.second.second < b.second.second; });
                        
                        cout << "Shortest paths from \"" << src << "\" to all other words:" << endl;
                        for (const auto& [dest, pathInfo] : sortedPaths) {
                            cout << "To \"" << dest << "\" (Distance: " << pathInfo.second << "): ";
                            printPath(pathInfo.first);
                        }
                    }
                } else {
                    if (!g.hasNode(dest)) {
                        cout << "The word \"" << dest << "\" is not in the graph!" << endl;
                        break;
                    }
                    
                    auto path = g.shortestPath(src, dest);
                    cout << "Shortest path from \"" << src << "\" to \"" << dest << "\": ";
                    printPath(path);
                    
                    if (!path.empty()) {
                        cout << "Path length: " << path.size() - 1 << endl;
                    }
                }
                break;
            }
            case 5: {
                cout << "PageRank values:\n";
                const auto& pr = g.getPageRank();
                vector<pair<string, double>> sortedPR(pr.begin(), pr.end());
                sort(sortedPR.begin(), sortedPR.end(),
                    [](const auto& a, const auto& b) { return b.second < a.second; });
                for (const auto& [node, rank] : sortedPR) {
                    cout << node << ": " << rank << endl;
                }
                break;
            }
            case 6: {
                // Ensure the img directory exists before calling randomWalk
                if (!CreateDirectoryA("img", NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
                    cout << "Error: Could not create 'img' directory." << endl;
                    break;
                }
                
                string walk = g.randomWalk(baseName);
                cout << "Random walk: " << walk << endl;
                cout << "Saved to img\\" << baseName << "_random_walk.txt" << endl;
                break;
            }
            default:
                cout << "Invalid choice!" << endl;
        }
    }
    return 0;
}