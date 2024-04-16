#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <limits>
using namespace std;

struct Point {
    double x;
    double y;
    double z;

    Point() : x(0), y(0), z(0) {}
    Point(double a, double b, double c) : x(a), y(b), z(c) {}

    friend ostream& operator<<(ostream& os, const Point& p) {
        os << "(" << p.x << ", " << p.y << ", " << p.z << ")";
        return os;
    }
};

double disteucl(const Point& a, const Point& b) {
    double d = 0.0;
    d += pow(a.x - b.x, 2);
    d += pow(a.y - b.y, 2);
    d += pow(a.z - b.z, 2);
    return sqrt(d);
}

class Octree {
public:
    Octree* children[8];
    vector<Point> points;

    Point bottomLeft;
    double h;
    int nPoints;
    Octree(Point b, double a);
    bool exist(const Point& point, Octree*& insertNode);
    void insert(const Point& point);
    void printAllPointsWithLevel(int l);
    void printAllPoints();
    Point find_closest(const Point& b, int radius);
};

Octree::Octree(Point b, double a) : bottomLeft(b), h(a), nPoints(0) {
    for (int i = 0; i < 8; ++i) {
        children[i] = nullptr;
    }
}

bool Octree::exist(const Point& point, Octree*& insertNode) {
    if (point.x < bottomLeft.x || point.x >= bottomLeft.x + h ||
        point.y < bottomLeft.y || point.y >= bottomLeft.y + h ||
        point.z < bottomLeft.z || point.z >= bottomLeft.z + h) {
        insertNode = this;
        return false;
    }

    for (const auto& p : points) {
        if (p.x == point.x && p.y == point.y && p.z == point.z) {
            insertNode = this;
            return true;
        }
    }

    for (int i = 0; i < 8; ++i) {
        if (children[i] && children[i]->exist(point, insertNode)) {
            return true;
        }
    }

    insertNode = this;
    return false;
}

void Octree::insert(const Point& point) {
    Octree* insertNode;

    if (exist(point, insertNode)) {
        cout << "Punto no insertado porque ya existe: (" << point.x << ", " << point.y << ", " << point.z << ")" << endl;
        return;
    }

    if (point.x < bottomLeft.x || point.x >= bottomLeft.x + h ||
        point.y < bottomLeft.y || point.y >= bottomLeft.y + h ||
        point.z < bottomLeft.z || point.z >= bottomLeft.z + h) {
        return;
        cout << "Punto no insertado porque no está dentro de los parámetros: (" << point.x << ", " << point.y << ", " << point.z << ")" << endl;
    }

    points.push_back(point);
    nPoints++;

    if (points.size() > 2) {
        for (int i = 0; i < 8; ++i) {
            double newX = (i & 1) ? bottomLeft.x + h / 2.0 : bottomLeft.x;
            double newY = (i & 2) ? bottomLeft.y + h / 2.0 : bottomLeft.y;
            double newZ = (i & 4) ? bottomLeft.z + h / 2.0 : bottomLeft.z;

            if (!children[i]) {
                children[i] = new Octree(Point(newX, newY, newZ), h / 2.0);
            }

            for (size_t j = 0; j < points.size(); ++j) {
                if (points[j].x >= newX && points[j].x < newX + h / 2.0 &&
                    points[j].y >= newY && points[j].y < newY + h / 2.0 &&
                    points[j].z >= newZ && points[j].z < newZ + h / 2.0) {
                    children[i]->insert(points[j]);
                    points.erase(points.begin() + j);
                    --j;
                }
            }
        }
    }
}

void Octree::printAllPoints() {
    for (const auto& p : points) {
        cout << p << endl;
    }
    for (int i = 0; i < 8; ++i) {
        if (children[i]) {
            children[i]->printAllPoints();
        }
    }
}

void Octree::printAllPointsWithLevel(int level) {
    vector<Point> a;
    for (const auto& p : points) {
        
        a.push_back(p);
    }

    cout << "Nivel " << level << ": ";
    for (const auto& p : a) {
        cout << p << " ";
    }
    cout << endl;

    for (int i = 0; i < 8; ++i) {
        if (children[i]) {
            children[i]->printAllPointsWithLevel(level + 1);
        }
    }
}

Point Octree::find_closest(const Point& b, int radius) {
    Point closest;
    double minDistance = numeric_limits<double>::max();

    for (const auto& node : points) {
        double dist = disteucl(node, b);
        if (dist <= radius && dist < minDistance) {
            closest = node;
            minDistance = dist;
        }
    }

    for (int i = 0; i < 8; ++i) {
        if (children[i]) {
            Point childClosest = children[i]->find_closest(b, radius);
            double dist = disteucl(childClosest, b);
            if (dist <= radius && dist < minDistance) {
                closest = childClosest;
                minDistance = dist;
            }
        }
    }

    return closest;
}

int main() {
    vector<Point> points;

    ifstream file("points1.csv");
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string token;
            vector<double> coordinates;
            while (getline(ss, token, ',')) {
                coordinates.push_back(stod(token));
            }
            if (coordinates.size() == 3) {
                points.emplace_back(coordinates[0], coordinates[1], coordinates[2]);
            }
        }
        file.close();
    }
    else {
        cerr << "Error al abrir el archivo." << endl;
        return 1;
    }

    Point bottomLeftPoint(points[0].x, points[0].y, points[0].z);
    Point topRightPoint(points[0].x, points[0].y, points[0].z);

    for (const auto& p : points) {
        if (p.x < bottomLeftPoint.x) bottomLeftPoint.x = p.x;
        if (p.y < bottomLeftPoint.y) bottomLeftPoint.y = p.y;
        if (p.z < bottomLeftPoint.z) bottomLeftPoint.z = p.z;

        if (p.x > topRightPoint.x) topRightPoint.x = p.x;
        if (p.y > topRightPoint.y) topRightPoint.y = p.y;
        if (p.z > topRightPoint.z) topRightPoint.z = p.z;
    }

    double height = max({ topRightPoint.x - bottomLeftPoint.x, topRightPoint.y - bottomLeftPoint.y, topRightPoint.z - bottomLeftPoint.z });
    cout << "(" << bottomLeftPoint.x << ", " << bottomLeftPoint.y << ", " << bottomLeftPoint.z << ")" << endl;
    cout << height << endl;

    Octree oct(bottomLeftPoint, height);
    for (int i = 0; i < points.size(); i++) {
        oct.insert(points[i]);
        //cout << "Punto " << i << ": (" << points[i].x << ", " << points[i].y << ", " << points[i].z << ")" << endl;
    }

    cout << "-----------------------------------------------------------------------" << endl;
    oct.printAllPoints(); 
    //oct.printAllPointsWithLevel(1);
    cout << "-----------------------------------------------------------------------" << endl;
    cout << "Numero de puntos en el csv: " << points.size() << endl;
    cout << "Numero de puntos en el octree: " << oct.nPoints << endl;

    /*
    (-167, 108, -30)
    (-170, 103, -35)
    (-167, 125, -30)
    (-170, 128, -37)
    (-170, 140, -46)
    (-172, 147, -26)
    (-116, 137, -66)
    (85, 15, 18)
    (73, 44, 23)
    (82, 35, 17)
    (79, 50, 22)
    (87, 49, 23)
    */

    cout << "-----------------------------------------------------------------------" << endl;
    Point a(50, 50, 50);
    Point b = oct.find_closest(a, 50);
    cout << "El punto mas cercano del "  ;
    cout << "Punto : (" << a.x << ", " << a.y << ", " << a.z << ") " << "es el "<< "Punto : (" << b.x << ", " << b.y << ", " << b.z << ") ";
    cout << "-----------------------------------------------------------------------" << endl;

    cout << "-----------------------------------------------------------------------" << endl;
    Point a1(200, 320, -5);
    Point c = oct.find_closest(a1, 50);
    cout << "El punto mas cercano del ";
    cout << "Punto : (" << a1.x << ", " << a1.y << ", " << a1.z << ") " << "es el " << "Punto : (" << c.x << ", " << c.y << ", " << c.z << ") ";
    cout << "-----------------------------------------------------------------------" << endl;

    cout << "-----------------------------------------------------------------------" << endl;
    Point a2(50, 50, 50);
    Point d = oct.find_closest(a2, 18);
    cout << "El punto mas cercano del ";
    cout << "Punto : (" << a2.x << ", " << a2.y << ", " << a2.z << ") " << "es el " << "Punto : (" << d.x << ", " << d.y << ", " << d.z << ") ";
    cout << "-----------------------------------------------------------------------" << endl;

    cout << "-----------------------------------------------------------------------" << endl;
    Point a3(250, 20, 50);
    Point f = oct.find_closest(a3, 50);
    cout << "El punto mas cercano del ";
    cout << "Punto : (" << a3.x << ", " << a3.y << ", " << a3.z << ") " << "es el " << "Punto : (" << f.x << ", " << f.y << ", " << f.z << ") ";
    cout << "-----------------------------------------------------------------------" << endl;

    cout << "-----------------------------------------------------------------------" << endl;
    Point a4(200, 315, -9);
    Point g = oct.find_closest(a4, 50);
    cout << "El punto mas cercano del ";
    cout << "Punto : (" << a4.x << ", " << a4.y << ", " << a4.z << ") " << "es el " << "Punto : (" << g.x << ", " << g.y << ", " << g.z << ") ";
    cout << "-----------------------------------------------------------------------" << endl;

    return 0;
}
