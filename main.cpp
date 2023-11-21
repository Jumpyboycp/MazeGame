#include <iostream>
#include <vector>
#include <fstream>
#include <utility>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <thread>
#include <stack>
using namespace std;

enum Direction { UP, DOWN, LEFT, RIGHT };

int startX = 0;
int startY = 0;

struct Node {
    int x, y, cost, heuristic;

    Node(int x, int y, int cost, int heuristic)
        : x(x), y(y), cost(cost), heuristic(heuristic) {}

    bool operator>(const Node& other) const {
        return (cost + heuristic) > (other.cost + other.heuristic);
    }
};

vector<vector<int>> readMazeFromFile(const string& filename) {
    ifstream file(filename);
    vector<vector<int>> maze;

    if (file.is_open()) {
        int value;
        char comma;
        vector<int> row;

        while (file >> value >> comma) {
            row.push_back(value);

            if (comma == ']') {
                maze.push_back(row);
                row.clear();
            }
        }

        file.close();
    } else {
        cout << "Unable to open file." << endl;
    }

    return maze;
}

void displayMaze(const vector<vector<int>>& maze) {
    for (const auto& row : maze) {
        for (int value : row) {
            switch (value) {
                case 0:
                    cout << " ";  // Empty space
                    break;
                case 1:
                    cout << "#";  // Wall
                    break;
                case 2:
                    cout << "S";  // Starting point
                    break;
                case 3:
                    cout << "G";  // Goal
                    break;
                case 4:
                    cout << "G";  // Googles
                    break;
                case 5:
                    cout << "P";  // Speed potion
                    break;
                case 6:
                    cout << "F";  // Fog
                    break;
                case 7:
                    cout << "P";  // Slowpoke potion
                    break;
                case 8:
                    cout << "*";  // Agent's path
                    break;
                default:
                    cout << value;
                    break;
            }
            cout << " ";
        }
        cout << endl;
    }
}

bool isValid(int x, int y, const vector<vector<int>>& maze) {
    return x >= 0 && x < maze.size() && y >= 0 && y < maze[0].size() && maze[x][y] == 0;
}

void handleBoostersAndHurdles(int& perceptiveField, int& stepWidth, int& score, int value) {
    if (value > 0) {
        cout << "Booster detected with value: " << value << endl;
        if (value == 4) {
            // Googles: Increase perceptive field by 1
            perceptiveField = min(3, perceptiveField + 1);
        } else if (value == 5) {
            // Speed potion: Increase step width by 1, but no more than 3
            stepWidth = min(3, stepWidth + 1);
        }
    } else if (value < 0) {
        cout << "Hurdle detected with value: " << value << endl;
        if (value == -6) {
            // Fog: Reduce perceptive field by 1, but no less than 1
            perceptiveField = max(1, perceptiveField - 1);
        } else if (value == -7) {
            // Slowpoke potion: Reduce step width by 1, but no less than 1
            stepWidth = max(1, stepWidth - 1);
        }
    } else {
        cout << "No booster or hurdle detected." << endl;
        score += 1;  // Increment the score for each step
    }
}


vector<pair<int, int>> findPathDFS(const vector<vector<int>>& maze, int startX, int startY, int goalX, int goalY) {
    const int dx[] = {0, 0, 1, -1};
    const int dy[] = {1, -1, 0, 0};

    stack<Node> st; // Include the stack
    vector<vector<bool>> visited(maze.size(), vector<bool>(maze[0].size(), false));

    st.push(Node(startX, startY, 0, 0));

    while (!st.empty()) {
        Node current = st.top();
        st.pop();

        if (current.x == goalX && current.y == goalY) {
            vector<pair<int, int>> path;
            while (current.x != startX || current.y != startY) {
                path.push_back({current.x, current.y});
                current.x -= dx[current.cost];
                current.y -= dy[current.cost];
            }
            path.push_back({startX, startY});
            reverse(path.begin(), path.end());
            return path;
        }

        if (visited[current.x][current.y]) {
            continue;
        }

        visited[current.x][current.y] = true;

        for (int i = 0; i < 4; ++i) {
            int newX = current.x + dx[i];
            int newY = current.y + dy[i];

            if (isValid(newX, newY, maze) && !visited[newX][newY]) {
                int newCost = current.cost + 1;
                int heuristic = abs(newX - goalX) + abs(newY - goalY);
                st.push(Node(newX, newY, newCost, heuristic));
            }
        }
    }

    return {};
}

void displayPath(vector<vector<int>>& maze, const vector<pair<int, int>>& path) {
    for (const auto& position : path) {
        maze[position.first][position.second] = 8;
    }
}

void turnAgent(Direction& currentDirection, bool turnRight) {
    if (turnRight) {
        switch (currentDirection) {
            case UP:
                currentDirection = RIGHT;
                break;
            case DOWN:
                currentDirection = LEFT;
                break;
            case LEFT:
                currentDirection = UP;
                break;
            case RIGHT:
                currentDirection = DOWN;
                break;
        }
    } else {
        switch (currentDirection) {
            case UP:
                currentDirection = LEFT;
                break;
            case DOWN:
                currentDirection = RIGHT;
                break;
            case LEFT:
                currentDirection = DOWN;
                break;
            case RIGHT:
                currentDirection = UP;
                break;
        }
    }
}

int main() {
    ifstream inputFile("D:\\PROJECT2\\MAZE\\maze.txt");

    if (!inputFile.is_open()) {
        cerr << "Error opening the file." << endl;
        return 1;
    }

    vector<vector<int>> maze;
    string line;

    while (getline(inputFile, line)) {
        vector<int> row;
        size_t start = line.find_first_of("[");
        size_t end = line.find_last_of("]");

        if (start != string::npos && end != string::npos) {
            line = line.substr(start + 1, end - start - 1);

            size_t pos = 0;
            while ((pos = line.find(',')) != string::npos) {
                row.push_back(stoi(line.substr(0, pos)));
                line.erase(0, pos + 1);
            }

            row.push_back(stoi(line));
            maze.push_back(row);
        }
    }
    displayMaze(maze);
    inputFile.close();

    int goalX = 4, goalY = 5;
    int endX = maze.size() - 1;
    int endY = maze[0].size() - 1;

    int adjustedStepWidth = 1;
    int perceptiveField = 1;
    int score = 0;

    vector<pair<int, int>> path = findPathDFS(maze, startX, startY, endX, endY);

    Direction direction = UP;
    bool turnRight = true;

    cout << "Current Direction: " << direction << endl;

    int boosterValue = 5;
    int hurdleValue = -3;
    int neutralValue = 0;

    // Agent moves autonomously along the calculated path
    for (const auto& position : path) {
        handleBoostersAndHurdles(perceptiveField, adjustedStepWidth, score, maze[position.first][position.second]);
        maze[position.first][position.second] = 8;

        // Display the maze after each move
        displayMaze(maze);
        cout << "Score: " << score << endl;

          }

    // Allow the user to make moves within the path
    char move;
    do {
        cout << "Enter your move (W for Up, A for Left, S for Down, D for Right, Q to Quit): ";
        cin >> move;

        switch (move) {
            case 'W':
            case 'w':
                if (startX > 0 && maze[startX - 1][startY] == 0) {
                    --startX;
                }
                break;
            case 'A':
            case 'a':
                if (startY > 0 && maze[startX][startY - 1] == 0) {
                    --startY;
                }
                break;
            case 'S':
            case 's':
                if (startX < endX && maze[startX + 1][startY] == 0) {
                    ++startX;
                }
                break;
            case 'D':
            case 'd':
                if (startY < endY && maze[startX][startY + 1] == 0) {
                    ++startY;
                }
                break;
            case 'Q':
            case 'q':
                cout << "Quitting the game. Total Score: " <<score << endl;
                return 0;
            default:
                cout << "Invalid move. Try again." << endl;
                continue;
        }

        handleBoostersAndHurdles(perceptiveField, adjustedStepWidth, score, maze[startX][startY]);
        maze[startX][startY] = 8;

        // Display the maze after each move
        displayMaze(maze);
        cout << "Score: " << score << endl;

        // Add a delay for visualization
        std::chrono::milliseconds duration(500);
        //std::this_thread::sleep_for(duration);

        // Check if there are any more moves

    }
     while (startX != endX || startY != endY);

    cout << "Congratulations! You reached the goal." << endl;

    // Wait for user input before exiting
    cout << "Press Enter to exit...";
    cin.ignore();
    cin.get();

    return 0;
}
