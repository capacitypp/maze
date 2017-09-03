#include <iostream>
#include <ctime>
#include <cmath>
#include <vector>
#include <random>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

//壁の厚さ
#define BLOCK_WIDTH	1

using namespace std;
using namespace cv;

class Position {
	int x, y;
public:
	inline Position(int px, int py) : x(px), y(py) { }
	inline Position(const Position& src) : x(src.x), y(src.y) { }
	inline int getX() const { return x; }
	inline int getY() const { return y; }
	inline Position& operator=(const Position& src) { x = src.x; y = src.y; return *this; }
	inline bool operator==(const Position& src) const { return (x == src.x) && (y == src.y); }
};

class Node {
	Node* parent;
	vector<Node*> childs;
public:
	inline Node(Node* parent_) : parent(parent_), childs(NULL) { }
	inline ~Node() { cout << "~Node\n"; }
	inline Node* getParent() const { return parent; }
	inline void addChild(Node* child) { childs.push_back(child); }
	inline void removeChild(Node* child) {
		for (int i = 0; i < childs.size(); i++) {
			if (childs[i] == child) {
				childs.erase(childs.begin() + i);
				break;
			}
		}
	}
	inline vector<Node*> getChilds() const { return childs; }
};

void destroy_map(char** map, int height)
{
	while (--height > 0)
		delete[] map[height];
	delete[] map;
}

char** generate_map(int width, int height)
{
	char** map;
	height += 4;
	width += 4;
	map = new char*[height];
	if (map == NULL)
		return NULL;
	for (int i = 0; i < height; i++) {
		map[i] = new char[width];
		if (map[i] == NULL)
			destroy_map(map, i);
		for (int j = 0; j < width; j++)
			map[i][j] = 0;
	}
	return map;
}

bool is_available(char** map, const Position& position)
{
	int x = position.getX();
	int y = position.getY();
	if (!map[y][x - 2])	return true;
	if (!map[y][x + 2])	return true;
	if (!map[y - 2][x])	return true;
	if (!map[y + 2][x])	return true;
	return false;
}

vector<Position>& list_available(char** map, const Position& position)
{
	int x = position.getX();
	int y = position.getY();
	vector<Position> positions;
	if (!map[y][x - 2] && !map[y][x - 1])	positions.push_back(Position(x - 2, y));
	if (!map[y][x + 2] && !map[y][x + 1])	positions.push_back(Position(x + 2, y));
	if (!map[y - 2][x] && !map[y - 1][x])	positions.push_back(Position(x, y - 2));
	if (!map[y + 2][x] && !map[y + 1][x])	positions.push_back(Position(x, y + 2));
	return *new vector<Position>(positions);
}

void set_value(char** map, const Position& position, char value)
{
	map[position.getY()][position.getX()] = value;
}

Position center_position(const Position& p1, const Position& p2)
{
	return Position((p1.getX() + p2.getX()) / 2, (p1.getY() + p2.getY()) / 2);
}

vector<Position> get_gateway(char** map, int width, int height)
{
	vector<Position> positions;
	for (int i = 3; i < width + 1; i++) {
		if (!map[2][i])
			positions.push_back(Position(i, 2));
		if (!map[height + 1][i])
			positions.push_back(Position(i, height + 1));
	}
	for (int i = 3; i < height + 1; i++) {
		if (!map[i][2])
			positions.push_back(Position(2, i));
		if (!map[i][width + 1])
			positions.push_back(Position(width + 1, i));
	}
	return vector<Position>(positions);
}

void generate_maze(char** map, int width, int height)
{
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < width + 4; j++) {
			map[i][j] = 1;
			map[height + 4 - i - 1][j] = 1;
		}
		for (int j = 0; j < height + 4; j++) {
			map[j][i] = 1;
			map[j][width + 4 - i - 1] = 1;
		}
	}

	//壁を作り始める場所
	vector<Position> positions;
	//最初は外周の壁を登録
	for (int i = 4; i < height; i += 2) {
		positions.push_back(Position(2, i));
		positions.push_back(Position(width + 1, i));
	}
	for (int i = 4; i < width; i += 2) {
		positions.push_back(Position(i, 2));
		positions.push_back(Position(i, height + 1));
	}
	random_device rnd;
	mt19937 mt(rnd());
	while (positions.size()) {
		int size = positions.size();
		uniform_int_distribution<> rand(0, size - 1);
		int idx = rand(mt);
		const Position& position = positions[idx];
		if (!is_available(map, position)) {
			positions.erase(positions.begin() + idx);
			continue;
		}
		vector<Position>& availables = list_available(map, position);
		if (!availables.size()) {
			positions.erase(positions.begin() + idx);
			continue;
		}
		uniform_int_distribution<> rand2(0, availables.size() - 1);
		const Position& new_position = availables[rand2(mt)];
		positions.push_back(new_position);
		set_value(map, new_position, 1);
		set_value(map, center_position(new_position, position), 1);
	}
	//出口を作る
	//出口候補の登録
	for (int i = 3; i < height + 1; i += 2) {
		positions.push_back(Position(2, i));
		positions.push_back(Position(width + 1, i));
	}
	for (int i = 3; i < width + 1; i += 2) {
		positions.push_back(Position(i, 2));
		positions.push_back(Position(i, height + 1));
	}
	//2箇所
	for (int i = 0; i < 2; i++) {
		uniform_int_distribution<> rand(0, positions.size() - 1);
		int idx = rand(mt);
		const Position& position = positions[idx];
		set_value(map, position, 0);
		positions.erase(positions.begin() + idx);
	}
}

Position get_next(char** map, const Position& position)
{
	int x = position.getX();
	int y = position.getY();
	if (!map[y][x + 1])
		return Position(x + 1, y);
	if (!map[y][x - 1])
		return Position(x - 1, y);
	if (!map[y + 1][x])
		return Position(x, y + 1);
	if (!map[y - 1][x])
		return Position(x, y - 1);
	return Position(-1, -1);
}

void analyze_map(char **map, int width, int height)
{
	vector<Position> gateways = get_gateway(map, width, height);
	if (gateways.size() != 2) {
		cout << "CannotFindGatewaysException.\n";
		return;
	}

	vector<Position> positions;
	Position start(get_next(map, gateways[0]));
	positions.push_back(start);
	set_value(map, start, 2);
	Position goal(get_next(map, gateways[1]));
	cout << "goal(" << goal.getX() << ", " << goal.getY() << ")\n";
	set_value(map, goal, 0);
	while (positions.size()) {
		const Position& position = positions.back();
		cout << "(" << position.getX() << ", " << position.getY() << ")" << "[" << positions.size() << "]\n";
		set_value(map, position, 2);
		{
			Mat image(height * BLOCK_WIDTH, width * BLOCK_WIDTH, CV_8UC3);
			for (int i = 2; i < height + 2; i++) {
				int y = (i - 2) * BLOCK_WIDTH;
				for (int j = 2; j < width + 2; j++) {
					int x = (j - 2) * BLOCK_WIDTH;
					if (map[i][j])
						continue;
					for (int dy = 0; dy < BLOCK_WIDTH; dy++)
						for (int dx = 0; dx < BLOCK_WIDTH; dx++)
							for (int k = 0; k < 3; k++)
								image.data[(y + dy) * image.step + (x + dx) * image.elemSize() + k] = 255;
				}
			}

			for (int i = 2; i < height + 2; i++) {
				int y = (i - 2) * BLOCK_WIDTH;
				for (int j = 2; j < width + 2; j++) {
					int x = (j - 2) * BLOCK_WIDTH;
					if (map[i][j] != 2)
						continue;
					for (int dy = 0; dy < BLOCK_WIDTH; dy++)
						for (int dx = 0; dx < BLOCK_WIDTH; dx++)
							image.data[(y + dy) * image.step + (x + dx) * image.elemSize()] = 255;
				}
			}
			imwrite("maze.bmp", image);
			getchar();
		}
		if (position == goal)
			return;
		vector<Position>& availables = list_available(map, position);

/*
		if (!availables.size()) {
			set_value(map, position, 0);
			positions.pop_back();
			continue;
		}
*/

		if (!availables.size()) {
			//くぁｗせｄｒｆｔｇｙふじこｌｐ
		}
		positions.pop_back();
		for (int i = 0; i < availables.size(); i++) {
			positions.push_back(availables[i]);
		}
	}

}

int main(int argc, char *argv[])
{
	int w, h;
	char** map;

	scanf("%d", &w);
	scanf("%d", &h);

	w = w / 2 * 2 + 1;
	h = h / 2 * 2 + 1;
/*
	if(w > 255)
		w = 255;
	if(h > 255)
		h = 255;
*/
	printf("%d %d\n", w, h);

	map = generate_map(w, h);

	generate_maze(map, w, h);


/*
	if(argc == 1)
		print_maze(map, w, h);
	else
		print_maze_code(map, w, h);
*/

	Mat image(h * BLOCK_WIDTH, w * BLOCK_WIDTH, CV_8UC3);
	for (int i = 2; i < h + 2; i++) {
		int y = (i - 2) * BLOCK_WIDTH;
		for (int j = 2; j < w + 2; j++) {
			int x = (j - 2) * BLOCK_WIDTH;
			if (map[i][j])
				continue;
			for (int dy = 0; dy < BLOCK_WIDTH; dy++)
				for (int dx = 0; dx < BLOCK_WIDTH; dx++)
					for (int k = 0; k < 3; k++)
						image.data[(y + dy) * image.step + (x + dx) * image.elemSize() + k] = 255;
		}
	}
	imwrite("maze.bmp", image);
/*
	analyze_map(map, w, h);

	for (int i = 2; i < h + 2; i++) {
		int y = (i - 2) * BLOCK_WIDTH;
		for (int j = 2; j < w + 2; j++) {
			int x = (j - 2) * BLOCK_WIDTH;
			if (map[i][j] != 2)
				continue;
			for (int dy = 0; dy < BLOCK_WIDTH; dy++)
				for (int dx = 0; dx < BLOCK_WIDTH; dx++)
					image[y + dy][x + dx][1] = image[y + dy][x + dx][2] = 0;
		}
	}
	image.writeImage("maze.bmp");
*/
	destroy_map(map, h);

	return 0;
}
