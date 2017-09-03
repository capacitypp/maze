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

vector<vector<char>> generate_map(int width, int height)
{
	return vector<vector<char>>(height + 4, vector<char>(width + 4, 0));
}

bool is_available(const vector<vector<char>>& map, const Position& position)
{
	int x = position.getX();
	int y = position.getY();
	if (!map[y][x - 2])	return true;
	if (!map[y][x + 2])	return true;
	if (!map[y - 2][x])	return true;
	if (!map[y + 2][x])	return true;
	return false;
}

vector<Position>& list_available(const vector<vector<char>>& map, const Position& position)
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

void set_value(vector<vector<char>>& map, const Position& position, char value)
{
	map[position.getY()][position.getX()] = value;
}

Position center_position(const Position& p1, const Position& p2)
{
	return Position((p1.getX() + p2.getX()) / 2, (p1.getY() + p2.getY()) / 2);
}

void generate_maze(vector<vector<char>>& map, int width, int height)
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

int main(int argc, char *argv[])
{
	if (argc != 3) {
		cerr << argv[0] << " <width> <height>" << endl;
		return 1;
	}

	int w = stoi(argv[1]);
	int h = stoi(argv[2]);

	w = w / 2 * 2 + 1;
	h = h / 2 * 2 + 1;

	printf("%d %d\n", w, h);

	vector<vector<char>> map = generate_map(w, h);

	generate_maze(map, w, h);

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

	return 0;
}
