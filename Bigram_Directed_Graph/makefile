all:
	g++ -Wall -O2 -std=c++17 src\Bigram_Directed_Graph.cpp -o bin\Bigram_Directed_Graph.exe

test1:
	bin\Bigram_Directed_Graph.exe  "test\Cursed Be The Treasure.txt"

test2:
	bin\Bigram_Directed_Graph.exe  "test\Easy Test.txt"

test3:
	bin\Bigram_Directed_Graph.exe  "test\Input.txt"

clean:
	del /Q bin\Bigram_Directed_Graph.exe graph.dot graph.dot.png random_walk.txt 2>nul