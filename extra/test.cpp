#include "BTree.h"
#include "BTree.cpp"

#include <iostream>
#include <random>
#include <climits>

int generateRandomRangedInt(int start, int end) {
    std::random_device random_device;  // Generate random numbers from hardware.
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<int> distribution(start, end);
    return distribution(generator);
}

class TestClass {
public:
    TestClass() {

    }
    std::string key;
    explicit TestClass(std::string k) : key(k) {}
};

int main() {
    BTree<std::string, TestClass> b_tree;
    int num = 100000;
    std::vector<std::string> v(num);
//    std::vector<int> v{
//        6, 8, 4, 1, 5, 7, 1, 7, 7, 1
//    };
    for (int i = 0; i < num; ++i) {
        v[i] = std::to_string(generateRandomRangedInt(INT_MIN, INT_MAX));
    }
    for (auto n : v) {
        TestClass test_class(n);
        b_tree.insert(n, test_class);
    }
    std::vector<std::string> res;
    for (int i = 0; i < num; ++i) {
        if (!b_tree.contains(v[i])) {
            return 1;
        }
        auto val = b_tree.search(v[i]);
        res.push_back(val->key);
    }

    if (!std::equal(v.begin(), v.end(), res.begin())) {
        return 1;
    }

//    b_tree.traverse(b_tree.root_);

//    std::cout << "---------" << std::endl;
    for (int i = 0; i < num; ++i) {
        if (!b_tree.remove(v[i])) {
            return 1;
        }
//        b_tree.traverse(b_tree.root_);
//        std::cout << "---------" << std::endl;

    }

    std::cout << "Check!" << std::endl;

    return 0;
}