#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <malloc.h>
#include "dataset.h"

using namespace std;
using namespace chrono;

long copy_count = 0;

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

void max_heapify(Dataset* data, int i, int heapsize) {
    // root index = 0
    int* array = data->array;
    int largest;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < heapsize && array[left] > array[i]) {
        largest = left;
    } else {
        largest = i;
    }

    if (right < heapsize && array[right] > array[largest]) {
        largest = right;
    }

    if (largest != i) {
        copy_count += 3;
        int temp = array[i];
        array[i] = array[largest];
        array[largest] = temp;
        max_heapify(data, largest, heapsize);
    }
}

void build_max_heap(Dataset* data) {
    int heapsize = data->size;
    for (int i = (heapsize - 2) / 2; i >= 0; i--) {
        max_heapify(data, i, heapsize);
    }
}

void heap_sort(Dataset* data) {
    int size = data->size;
    int* array = data->array;
    int heapsize = size;

    build_max_heap(data);

    for (int i = size - 1; i > 0; i--) {
        copy_count += 3;
        int temp = array[0];
        array[0] = array[i];
        array[i] = temp;
        heapsize--;
        max_heapify(data, 0, heapsize);
    }
}

void stability_max_heapify(Stability_Dataset* data, int i, int heapsize) {
    // root index = 0
    std::pair<int, int>* array = data->array;
    int largest;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < heapsize && array[left].first > array[i].first) {
        largest = left;
    } else
        largest = i;

    if (right < heapsize && array[right].first > array[largest].first) {
        largest = right;
    }

    if (largest != i) {
        std::pair<int, int> temp = array[i];
        array[i] = array[largest];
        array[largest] = temp;
        stability_max_heapify(data, largest, heapsize);
    }
}

void stability_build_max_heap(Stability_Dataset* data) {
    int heapsize = 10000;
    for (int i = (heapsize - 2) / 2; i >= 0; i--) {
        stability_max_heapify(data, i, heapsize);
    }
}

void stability_heap_sort(Stability_Dataset* data) {
    int size = 10000;
    std::pair<int, int>* array = data->array;
    int heapsize = size;

    stability_build_max_heap(data);

    for (int i = size - 1; i > 0; i--) {
        std::pair<int, int> temp = array[0];
        array[0] = array[i];
        array[i] = temp;
        heapsize--;
        stability_max_heapify(data, 0, heapsize);
    }
}

int main() {
    vector<int> sizes = {1000, 10000, 100000, 1000000};
    vector<int> types = {1, 2, 3, 4, 5, 6};
    bool stable = true;
    Stability_Dataset* stability_data = new Stability_Dataset();

    ofstream outfile_stability("heap_sort_stability.txt");

    stability_data->generate_stability_data();
    stability_heap_sort(stability_data);
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

    ofstream outfile("heap_sort_results.csv");
    outfile << "type,size,trial,time_ms,copy_count,memory_kb\n";

    for (int type : types) {
        for (int size : sizes) {
            for (int trial = 1; trial <= 10; trial++) {
                copy_count = 0;
                Dataset* data = new Dataset(size, type);
                data->generate_data();

                auto start = high_resolution_clock::now();
                heap_sort(data);
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