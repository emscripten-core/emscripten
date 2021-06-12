#include <concepts>

template <typename T>
concept Test = std::destructible<T>;

template <Test T>
class MyTest {
public:
    T value;
};

int main() {
    MyTest<int> test;
    return 0;
}
