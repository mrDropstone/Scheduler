#pragma once
#include <string>
#include <map>
#include <vector>
#include <random>

int randint(int a, int b) {
    static std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distr(a, b);
    int number =  distr(gen);
    return number;
}

int randfloat(float a, float b) {
    static std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distr(a, b);
    //std::cout << "randint exit" << std::endl;
    return distr(gen);
}

template <typename T, typename U>
constexpr std::pair<T, U&> get_random(std::map<T, U>& map) {
    assert(map.size() != 0);
    auto it = map.begin();
    int index = randint(0, map.size() - 1);
    std::advance(it, index);
    return *it;
}
template <typename T>
constexpr T& get_random(std::vector<T>& vector) {
    assert(vector.size() != 0);
    return vector.at(randint(0, vector.size() - 1));
}
template <typename T>
constexpr const T& get_random(const std::vector<T>& vector) {
    assert(vector.size() != 0);
    return vector.at(randint(0, vector.size() - 1));
}

class Possibility {
    float uniform;
    Possibility(float possibility) : uniform(possibility) {}
    operator bool() {
        return randfloat(0.0, 1.0) < uniform;
    }
};

std::string multiply_string(const std::string& string, int times) {
    std::string result;
    for (int i = 0; i < times; i++)
        result += string;
    return result;
}
