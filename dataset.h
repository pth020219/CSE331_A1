// Size: 1K, 10K, 100K, 1M
// type: 1. sorted - acending, 2. sorted - decending, 3. random, 4. 50% sorted
// data,  5.99% sorted data, 6. Zigzag
//  evaluate: time, memory, stablility, copy_count

// special case: duplicate data for evaluating stability, for example, [(3, a),
// (3, b), (3, c), (3, d), (1, a), (1, b), (1, c), (1, d), ...]

#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <algorithm>
#include <iostream>
#include <random>
#include <string>

class Dataset {
   public:
    int size;
    int type;
    int* array;
    Dataset(int size, int type) : size(size), type(type) {
        array = new int[size];
    }

    void generate_data();

    void print_data() {
        for (int i = 0; i < size; ++i) {
            std::cout << array[i] << " ";
        }
        std::cout << std::endl;
    }

    ~Dataset() { delete[] array; }
};

class Stability_Dataset {
   public:
    std::pair<int, int> array[10000];
    Stability_Dataset() {}
    void generate_stability_data() {
        srand(static_cast<unsigned>(time(0)));
        for (int i = 0; i < 10000; i++) {
            int random_key = rand() % 100;
            array[i].first = random_key;
            array[i].second = i;
        }
    }
    void print_data() {
        for (int i = 0; i < 10000; ++i) {
            std::cout << "(" << array[i].first << ", " << array[i].second
                      << ") ";
        }
        std::cout << std::endl;
    }

    ~Stability_Dataset() {}
};

#endif