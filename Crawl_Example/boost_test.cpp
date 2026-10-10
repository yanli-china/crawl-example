#include <iostream>
#include <boost/optional.hpp>

boost::optional<int> divide(int a, int b) {
    if (b == 0)
        return boost::none;

    return a / b;
}

int main() {
    auto result = divide(10, 2);

    if (result) {
        std::cout << *result << std::endl;
    }
    else {
        std::cout << "error" << std::endl;
    }
}