#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <thread>
#include <string>
#include <array>
#include <algorithm>
#include <cstdlib>

struct Point3D {
    double x, y, z;
};

struct ProjectedPoint {
    int x, y;
    double z;
};

void clearScreen() {
    #ifdef _WIN32
        system("cls"); // bugsoft windows support
    #else
        system("clear");
    #endif
}

std::pair<std::vector<Point3D>, std::vector<std::pair<int, int>>> createCube() {
    std::vector<Point3D> vertices = {
        {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
        {-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1}
    };

    std::vector<std::pair<int, int>> edges = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };

    return {vertices, edges};
}

void rotatePoint(Point3D& point, double angle_x, double angle_y) {
    // Rotate around X axis
    double y = point.y * cos(angle_x) - point.z * sin(angle_x);
    double z = point.y * sin(angle_x) + point.z * cos(angle_x);
    point.y = y;
    point.z = z;

    // Rotate around Y axis
    double x = point.x * cos(angle_y) + point.z * sin(angle_y);
    z = -point.x * sin(angle_y) + point.z * cos(angle_y);
    point.x = x;
    point.z = z;
}

ProjectedPoint projectPoint(const Point3D& point, int width, int height) {
    const double factor = 5.0;
    return {
        static_cast<int>(point.x * factor + width / 2),
        static_cast<int>(point.y * factor + height / 2),
        point.z
    };
}

void drawLine(std::vector<std::vector<char>>& screen, ProjectedPoint p1, ProjectedPoint p2, double min_z, double max_z) {
    int width = screen[0].size();
    int height = screen.size();
    
    int dx = abs(p2.x - p1.x);
    int dy = abs(p2.y - p1.y);
    int sx = p1.x < p2.x ? 1 : -1;
    int sy = p1.y < p2.y ? 1 : -1;
    int err = dx - dy;

    int x = p1.x;
    int y = p1.y;

    double total_steps = static_cast<double>(std::max(dx, dy));
    double current_step = 0.0;

    const std::array<char, 12> chars = {'.',',','-','~',':',';','=','*','!','#','$','@'};

    while (true) {
        // Calculate interpolated z
        double t = (total_steps == 0) ? 0.0 : current_step / total_steps;
        double z = p1.z + t * (p2.z - p1.z);

        // Normalize z to [0, 1]
        double normalized_z = (z - min_z) / (max_z - min_z);
        int index = static_cast<int>(normalized_z * 11 + 0.5);
        index = std::max(0, std::min(11, index));
        char c = chars[index];

        if (x >= 0 && x < width && y >= 0 && y < height) {
            screen[y][x] = c;
        }

        if (x == p2.x && y == p2.y) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }

        current_step += 1.0;
    }
}

int main(int argc, char* argv[]) {
    double speed = 1.0;
    if (argc > 2 && std::string(argv[1]) == "-s") {
        speed = std::stod(argv[2]);
    }

    const int width = 20;
    const int height = 20;
    double angle_x = 0.0;
    double angle_y = 0.0;

    auto [vertices, edges] = createCube();

    while (true) {
        std::vector<std::vector<char>> screen(height, std::vector<char>(width, ' '));
        std::vector<Point3D> rotated = vertices;

        // Rotate all points
        for (auto& vertex : rotated) {
            rotatePoint(vertex, angle_x, angle_y);
        }

        // Find min and max z for this frame
        double min_z = rotated[0].z;
        double max_z = rotated[0].z;
        for (const auto& vertex : rotated) {
            if (vertex.z < min_z) min_z = vertex.z;
            if (vertex.z > max_z) max_z = vertex.z;
        }
        if (max_z == min_z) {
            max_z += 1e-6;
        }

        // Draw edges
        for (const auto& edge : edges) {
            ProjectedPoint p1 = projectPoint(rotated[edge.first], width, height);
            ProjectedPoint p2 = projectPoint(rotated[edge.second], width, height);
            drawLine(screen, p1, p2, min_z, max_z);
        }

        // Display frame
        clearScreen();
        for (const auto& row : screen) {
            for (char c : row) {
                std::cout << c;
            }
            std::cout << '\n';
        }

        // Update angles
        angle_x += 0.05 * speed;
        angle_y += 0.03 * speed;

        // Sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(30.0 / speed)));
    }

    return 0;
}