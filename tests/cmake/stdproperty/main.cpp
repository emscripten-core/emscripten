#include <iostream>

struct vector2d{
	int x,y,z;
};

int main(int argc, char * argv[])
{
	vector2d v{1,2,3};
	std::cout << v.x << "," << v.y << "," << v.z << std::endl;
}
