#include <memory>

class Point
{
public:
    Point(int ox = 0, int oy = 0) :x(ox),y(oy){}
    int getX() const {return x;}
    int getY() const {return y;}
    void setX(int ox) {x = ox;}
    void setY(int oy) {y = oy;}

    ~Point(){}
private:
    int x, y;
};

int main()
{
    std::shared_ptr<Point> sp = std::make_shared<Point>();
    return 1;
}