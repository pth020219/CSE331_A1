

#include <limits.h>
#include <malloc.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "dataset.h"

using namespace std;
using namespace chrono;

struct node {
    int value;
    int index;
};

struct stability_node {
    std::pair<int, int> value;
    int index;
};
long getMemoryUsageKB() {
    std::ifstream status_file("/proc/self/status");
    std::string line;
    while (std::getline(status_file, line)) {
        if (line.find("VmRSS:") == 0) {
            std::istringstream iss(line);
            std::string key;
            long memory_kb;
            std::string unit;
            iss >> key >> memory_kb >> unit;
            return memory_kb;
        }
    }
    return -1;
}

void tournament_sort(Dataset* data, long& copy_count) {
    copy_count = 0;
    int size = data->size;
    int* array = data->array;
    int num_leaf = 1;

    while (num_leaf < size) num_leaf *= 2;
    // root index = 1
    node* tree = new node[2 * num_leaf];

    for (int i = 0; i < num_leaf; i++) {
        if (i < size) {
            copy_count++;
            tree[i + num_leaf] = {array[i], i};
        } else {
            copy_count++;
            tree[i + num_leaf] = {INT_MAX, -1};
        }
    }

    for (int i = num_leaf - 1; i > 0; i--) {
        if (tree[2 * i].value <= tree[2 * i + 1].value) {
            copy_count++;
            tree[i] = tree[2 * i];
        } else {
            copy_count++;
            tree[i] = tree[2 * i + 1];
        }
    }

    for (int i = 0; i < size; i++) {
        copy_count += 2;
        node win = tree[1];
        array[i] = win.value;

        int index = num_leaf + win.index;
        copy_count++;
        tree[index].value = INT_MAX;

        index = index / 2;
        while (index > 0) {
            if (tree[2 * index].value <= tree[2 * index + 1].value) {
                copy_count++;
                tree[index] = tree[2 * index];
            } else {
                copy_count++;
                tree[index] = tree[2 * index + 1];
            }
            index = index / 2;
        }
    }

    delete[] tree;
}

void stability_tournament_sort(Stability_Dataset* data) {
    int size = 10000;
    std::pair<int, int>* array = data->array;
    int num_leaf = 1;

    while (num_leaf < size) num_leaf *= 2;
    // root index = 0
    stability_node* tree = new stability_node[2 * num_leaf];

    for (int i = 0; i < num_leaf; i++) {
        if (i < size)
            tree[i + num_leaf] = {array[i], i};
        else
            tree[i + num_leaf] = {{INT_MAX, -1}, -1};
    }

    for (int i = num_leaf - 1; i > 0; i--) {
        if (tree[2 * i].value.first <= tree[2 * i + 1].value.first)
            tree[i] = tree[2 * i];
        else
            tree[i] = tree[2 * i + 1];
    }

    for (int i = 0; i < size; i++) {
        stability_node win = tree[1];
        array[i] = win.value;

        int index = num_leaf + win.index;
        tree[index].value = {INT_MAX, -1};

        index = index / 2;
        while (index > 0) {
            if (tree[2 * index].value.first <= tree[2 * index + 1].value.first)
                tree[index] = tree[2 * index];
            else
                tree[index] = tree[2 * index + 1];
            index = index / 2;
        }
    }

    delete[] tree;
}

int main() {
    vector<int> sizes = {1000, 10000, 100000, 1000000};
    vector<int> types = {1, 2, 3, 4, 5, 6};
    long copy_count = 0;
    bool stable = true;
    Stability_Dataset* stability_data = new Stability_Dataset();

    ofstream outfile_stability("tournament_sort_stability.txt");

    stability_data->generate_stability_data();
    stability_tournament_sort(stability_data);
    for (int i = 0; i < 9999; i++) {
        if (stability_data->array[i].first ==
            stability_data->array[i + 1].first) {
            if (stability_data->array[i].second >
                stability_data->array[i + 1].second) {
                stable = false;
                break;
            }
        }
    }
    delete stability_data;
    outfile_stability << (stable ? "Stable" : "Unstable") << "\n";

    outfile_stability.close();

    ofstream outfile("tournament_sort_results.csv");
    outfile << "type,size,trial,time_ms,copy_count,memory_"
               "kb\n";

    for (int type : types) {
        for (int size : sizes) {
            for (int trial = 1; trial <= 10; trial++) {
                Dataset* data = new Dataset(size, type);
                data->generate_data();

                auto start = high_resolution_clock::now();
                tournament_sort(data, copy_count);
                auto end = high_resolution_clock::now();

                auto time_ms =
                    std::chrono::duration<double, std::milli>(end - start)
                        .count();
                long memory_kb = getMemoryUsageKB();

                outfile << type << "," << size << "," << trial << "," << time_ms
                        << "," << copy_count << "," << memory_kb << "\n";
                outfile.flush();

                delete data;
                malloc_trim(0);
                cout << "Trial " << trial << " for type " << type
                     << " and size " << size << " completed." << endl;
            }
        }
    }

    outfile.close();

    return 0;
}