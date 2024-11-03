#include <vector>
#include <iostream>

template <typename T>
class LimitedVector {
private:
    std::vector<T> data;
    size_t maxSize;

public:
    // Constructor to set the maximum size
    LimitedVector(size_t max) : maxSize(max) {}

    // Method to add a new element
    void add(const T& element) {
        if (data.size() >= maxSize) {
            // Remove the oldest element (at the front)
            data.erase(data.begin());
        }
        data.push_back(element);
    }

    // Method to get the size of the vector
    size_t size() const {
        return data.size();
    }

    T& LastElement(){
        return data.back();
    }

    // Method to access elements
    T& operator[](size_t index) {
        return data[index];
    }

    const T& operator[](size_t index) const {
        return data[index];
    }

    // Print method for demonstration
    void print() const {
        for (const auto& item : data) {
            std::cout << item << " ";
        }
        std::cout << std::endl;
    }
};
