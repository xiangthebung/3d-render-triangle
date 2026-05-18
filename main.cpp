#include<iostream>
#include<vector>
#include<algorithm>

#include <termios.h>
#include <unistd.h>

#include <fstream>
using namespace std;



char getch() { // this snippet was not written by me
    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    char c = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return c;
}


struct Vector3D{
    double x, y, z;
    Vector3D(double x, double y, double z):x(x),y(y),z(z){}
    Vector3D operator+(const Vector3D& other) const {
        return Vector3D(x + other.x, y + other.y, z + other.z);
    }


    Vector3D operator-(const Vector3D& other) const {
        return Vector3D(x - other.x, y - other.y, z - other.z);
    }

    Vector3D operator*(double a) const {
        return Vector3D(x *a, y*a, z*a);
    }

    Vector3D& operator+=(const Vector3D& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }
    Vector3D cross(const Vector3D& b) const {
        return Vector3D( y*b.z - z*b.y, z*b.x - x*b.z, x*b.y - y*b.x);
    }
    double dot(const Vector3D& b) const {
        return x*b.x + y*b.y + z*b.z;
    }
    Vector3D normalize() const{
        double l = sqrt(x*x + y*y + z*z);
        return *this * (1/l);
    }
};

struct Vector2D {
    double x, y;

    Vector2D(double x = 0, double y = 0)
        : x(x), y(y) {}

    double operator*(const Vector2D& b) const {
        return x * b.x + y * b.y;
    }
    Vector2D getNorm() const { //left norm
        return Vector2D(y, -x);
    }
    Vector2D operator-(const Vector2D& other) const {
        return Vector2D(x-other.x, y-other.y);
    }
};

struct Triangle {
    Vector3D v1, v2, v3;

    Triangle(const Vector3D& a, const Vector3D& b, const Vector3D& c)
        : v1(a), v2(b), v3(c) {}
};

bool onLeftOfLine(const Vector2D& a, const Vector2D& b, const Vector2D& point){
    // determine if point is on the left of b-a
    // norm of b-a pointing to the left is (b.y-a.y), -(b.x-a.x)
    // dot this with point - a. (Or point - b)
    return (b-a).getNorm() * (point - a) >= 0;
}

struct Triangle2D{
    Vector2D v1, v2, v3;
    void getBoundingBox(int& minX, int& maxX, int& minY, int& maxY, int width, int height) const {
        minX = min({v1.x, v2.x, v3.x});
        minX = max(minX, 0);
        maxX = max({v1.x, v2.x, v3.x});
        maxX = min(maxX, width);
        minY = min({v1.y, v2.y, v3.y});
        minY = max(minY, 0);
        maxY = max({v1.y, v2.y, v3.y});
        maxY = min(maxY, height);
    }


    bool pointInside(const Vector2D& v){
        bool a = onLeftOfLine(v1, v2, v);
        bool b = onLeftOfLine(v2, v3, v);
        bool c = onLeftOfLine(v3, v1, v);
        // cout <<a && b && c || !a && !b && !c;
        // if counterclockwise, a & b & c satisfies
        // if clockwise !a & !b & !c satisfies
        // note if a point is outside the triangle, !a & !b & !c cannot be satisfied. 
        return a && b && c || !a && !b && !c;
    }

};

double screenDistance = 2; // Distance from eye to screen (Project onto this plane)

namespace Color {
    const string reset = "\033[0m";
    const string red = "\033[31m";
    const string green = "\033[32m";
    const string blue = "\033[34m";
}

struct Pixel{
    string color = Color::reset;
    char symbol;
    Pixel(): symbol('0'){}
    Pixel(char c): symbol(c){}
    Pixel(string col, char c): color(col), symbol(c){}
};

// ostream& operator<<(ostream& o, const Pixel& p){ //too slow
//     return o << p.color << p.symbol;
// }

class Object { // object made of triangles
public:
    vector<Triangle> triangles;
    Pixel texture;

    Object():texture('0'){}
    void setTexture(Pixel t){
        texture = t;
    }
    void addTriangle(const Triangle& t) {
        triangles.push_back(t);
    }
};

class Buffer {
public:
    int width, height;
    vector<vector<Pixel>> buffer;
    vector<vector<double>> depths;

    Buffer(int w, int h): width(w), height(h), buffer(w, vector<Pixel>(h)), depths(w, vector<double>(h)) {}

    void clear() {
        for(int i = 0; i < width; i++){
            fill(buffer[i].begin(), buffer[i].end(), ' ');
            fill(depths[i].begin(), depths[i].end(), 2e9);
        }
    }

    void drawPoint(int x, int y, double z, const Pixel& c) {
        if (not (x >= 0 && x < width && y >= 0 && y < height))
            return;
        if (z > depths[x][y]) return;
        depths[x][y] = z;
        buffer[x][y] = c;
    }

    void display() {
        cout << "\033[H";
        string curCol = Color::reset;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (curCol != buffer[x][y].color)
                    cout << buffer[x][y].color;
                cout << buffer[x][y].symbol;
            }
            if (y < height - 1) cout << "\n";
        }
        cout.flush();
    }

    /*
    (-1, 1), (0, 1), (1, 1)
    (-1, 0), (0, 0), (1, 0)
    (-1, -1), (0, -1), (1, -1)

    [0][0], [1][0], [2][0]
    [0][1], [1][1], [2][1]
    [0][2], [1][2], [2][2]
    
    */
};

Vector3D rotateLeft(const Vector3D& v,  const double& angle) {
    const double& x = v.x, &y = v.y, &z = v.z;
    double l = sqrt(x*x + z*z);
    double a = atan2(z,x);
    return {l*cos(a + angle), y, l*sin(a+angle)};
}

Vector3D rotateDown(const Vector3D& v, const double& angle){
    // x is constant
    const double& x = v.x, &y = v.y, &z = v.z;
    double l = sqrt(y*y+ z*z);
    double a = atan2(z, y);
    return {x, l*cos(a+angle), l*sin(a+angle)};

}

class Camera{

    Vector3D position;
    double angleLeft{};
    double angleDown{};
    public:
    
    Camera(const Vector3D& pos):position(pos){}
    Vector3D pointInReferenceToCamera(const Vector3D& obj){
        Vector3D translated = obj-position;
        return rotateDown(rotateLeft(translated, -angleLeft), -angleDown);
    }

    void move(const Vector3D& pos){ //RELATIVE TO CAMERA ANGLE
        position += rotateDown(rotateLeft(pos, angleLeft), angleDown);
    }

    void lookLeft(double angle){
        angleLeft += angle;
    }

    void lookDown(double angle){
        angleDown += angle;
    }
};



class Draw {
    public:
    Buffer* buffer;
    Camera* camera;
    Draw(Buffer* b, Camera* c): buffer(b), camera(c){}

    
    Vector2D getProjectedPoint(const Vector3D& v){
        Vector2D ret;
        // Vector3D cameraPoint = camera->pointInReferenceToCamera(v);
        if (v.z < 0.001) return {-1000, -1000};

        ret.x = v.x/v.z * screenDistance;
        ret.y = v.y/v.z * screenDistance;
        return ret;
    }
    
    Vector2D toScreen(const Vector2D& v) {
        // [-1 , 1]
        double screenX = (v.x + 1) / 2 * (buffer->width - 1);
        // [1, -1]
        double screenY = (1- v.y) / 2* (buffer->height - 1);
        return Vector2D(screenX, screenY);
    }

    Vector2D screenToProjected(double sx, double sy) {
        double x =(sx / (buffer->width - 1)) * 2.0 - 1.0;
        double y = 1.0 - (sy / (buffer->height - 1)) * 2.0;

        return Vector2D(x,y);
    }

    Vector2D projectToScreen(const Vector3D& v){
        return toScreen(getProjectedPoint(v));
    }

    void drawTriangle(const Triangle& t, Pixel texture){
        // triangle location relative to camera
        Triangle trianglelocation = Triangle(camera->pointInReferenceToCamera(t.v1), camera->pointInReferenceToCamera(t.v2), camera->pointInReferenceToCamera(t.v3));
        Triangle2D tri{projectToScreen(camera->pointInReferenceToCamera(t.v1)), 
            projectToScreen(camera->pointInReferenceToCamera(t.v2)), projectToScreen(camera->pointInReferenceToCamera(t.v3))};

        int minX, maxX, minY, maxY;
        tri.getBoundingBox(minX, maxX, minY, maxY, buffer->width, buffer->height);


         for (int y = minY; y <= maxY; y++) {
            for (int x = minX; x <= maxX; x++) {
                if (tri.pointInside(Vector2D(x, y))) {
                    // need to get depth given x, y screen
                    // reproject the point back onto the triangle in 3d? 
                    // seems quite inefficient but it should be correct 
                    

                    //cross product to find norm of triangle plane then normalize the norm
                    // set v = vector from camera to plane (where x,y maps to) - ie shoot laser from camera through where (x,y) would be on our projected screen
                    // ratio = v dot norm how much v is on norm
                    // let A be a vertex on the triangle
                    // A dot norm gets perpendicular length
                    // perp length / ratio gives how many v's are needed to reach the triangle
                    // get z value of perp length / ratio * v 
                    Vector2D p = screenToProjected(x, y);
                    Vector3D proj = {p.x ,p.y, screenDistance};
                    Vector3D normTri = (trianglelocation.v2 - trianglelocation.v1).cross(trianglelocation.v3 - trianglelocation.v1).normalize();
                    
                    double projOnNorm = proj.dot(normTri);
                    double lenPerp = trianglelocation.v1.dot(normTri);
                    double depth = (proj * (lenPerp/projOnNorm)).z;
                    depth = abs(depth);
                    buffer->drawPoint(x, y, depth, texture);
                }
            }
        }
    }

    void renderObject(const Object& m){
        for (const Triangle& t: m.triangles){
            drawTriangle(t, m.texture);
        }
    }
    

};


int main(){
    Buffer buffer(200, 120);
    Camera cam({0,0,0});

    Draw draw(&buffer, &cam);
    Object obj1;
    obj1.setTexture(Pixel({Color::red, '@'}));

    Object obj2;
    obj2.setTexture(Pixel({Color::blue, '#'}));


    obj1.addTriangle(Triangle(
        Vector3D(-2, -1, 8),
        Vector3D(2, -2, 4),
        Vector3D(0,1, 5)
    ));


    obj2.addTriangle(Triangle(
        Vector3D(-3,  3, 6),
        Vector3D(1,  1, 8),
        Vector3D(0, -2, 5)
    ));

    const double moveSpeed = 0.1;
    const double lookSpeed = 0.03;


    while (true) {
        buffer.clear();
        draw.renderObject(obj1);
        draw.renderObject(obj2);
        buffer.display();

        int key = getch();
        if      (key == 'w') cam.move(Vector3D(0,0,1) * moveSpeed);
        else if (key == 's') cam.move(Vector3D(0,0,1) * (-moveSpeed));
        else if (key == 'a') cam.move(Vector3D(1,0,0) * (-moveSpeed));
        else if (key == 'd') cam.move(Vector3D(1,0,0) * (moveSpeed));
        else if (key == 'x') cam.move(Vector3D(0,1,0) * moveSpeed);
        else if (key == 'c') cam.move(Vector3D(0,1,0) * (-moveSpeed));
        else if (key == 'q') cam.lookLeft(lookSpeed);
        else if (key == 'e') cam.lookLeft(-lookSpeed);
        else if (key == 'r') cam.lookDown(-lookSpeed);  
        else if (key == 'f') cam.lookDown(lookSpeed);
        else if (key == 'b') break;                     
    }
    
}
