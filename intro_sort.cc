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

void insertion_sort(int* array, int p, int q) {
    int key, j;
    for (int i = p + 1; i <= q; i++) {
        copy_count++;
        key = array[i];
        j = i - 1;

        while (j >= p && array[j] > key) {
            copy_count++;
            array[j + 1] = array[j];
            j--;
        }

        copy_count++;
        array[j + 1] = key;
    }
}

void max_heapify(int* array, int p, int i, int heapsize) {
    // root index = 0
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < heapsize && array[p + left] > array[p + i]) {
        largest = left;
    }

    if (right < heapsize && array[p + right] > array[p + largest]) {
        largest = right;
    }

    if (largest != i) {
        copy_count += 3;
        int temp = array[p + i];
        array[p + i] = array[p + largest];
        array[p + largest] = temp;
        max_heapify(array, p, largest, heapsize);
    }
}

void build_max_heap(int* array, int p, int q) {
    int heapsize = q - p + 1;
    for (int i = heapsize / 2 - 1; i >= 0; i--) {
        max_heapify(array, p, i, heapsize);
    }
}

void heap_sort(int* array, int p, int q) {
    int size = q - p + 1;
    int heapsize = size;

    build_max_heap(array, p, q);

    for (int i = size - 1; i >= 1; i--) {
        copy_count += 3;
        int temp = array[p];
        array[p] = array[p + i];
        array[p + i] = temp;
        heapsize--;
        max_heapify(array, p, 0, heapsize);
    }
}

int partition(int* array, int p, int r) {
    int i = p - 1;
    copy_count++;
    int pivot = array[r];

    for (int j = p; j < r; j++) {
        if (array[j] <= pivot) {
            i++;
            copy_count += 3;
            int temp = array[i];
            array[i] = array[j];
            array[j] = temp;
        }
    }
    copy_count += 3;
    int temp = array[i + 1];
    array[i + 1] = array[r];
    array[r] = temp;
    return i + 1;
}

void introsort(int* array, int p, int q, int max_depth) {
    int size = q - p + 1;
    if (size <= 16) {
        insertion_sort(array, p, q);
    } else if (max_depth == 0) {
        heap_sort(array, p, q);
    } else {
        int pivot = partition(array, p, q);
        introsort(array, p, pivot - 1, max_depth - 1);
        introsort(array, pivot + 1, q, max_depth - 1);
    }
}

void intro_sort(Dataset* data) {
    int* array = data->array;
    int size = data->size;
    int max_depth = 2 * (int)(log2(size) - 1);
    introsort(array, 0, size - 1, max_depth);
}

void stability_insertion_sort(std::pair<int, int>* array, int p, int q) {
    std::pair<int, int> key;
    int j;
    for (int i = p + 1; i <= q; i++) {
        key = array[i];
        j = i - 1;

        while (j >= p && array[j].first > key.first) {
            copy_count++;
            array[j + 1] = array[j];
            j--;
        }

        copy_count++;
        array[j + 1] = key;
    }
}

void stability_max_heapify(std::pair<int, int>* array, int p, int i,
                           int heapsize) {
    // root index = 0
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < heapsize && array[p + left].first > array[p + i].first)
        largest = left;

    if (right < heapsize && array[p + right].first > array[p + largest].first)
        largest = right;

    if (largest != i) {
        copy_count += 3;
        std::pair<int, int> temp = array[p + i];
        array[p + i] = array[p + largest];
        array[p + largest] = temp;
        stability_max_heapify(array, p, largest, heapsize);
    }
}

void stability_build_max_heap(std::pair<int, int>* array, int p, int q) {
    int heapsize = q - p + 1;
    for (int i = heapsize / 2 - 1; i >= 0; i--) {
        stability_max_heapify(array, p, i, heapsize);
    }
}

void stability_heap_sort(std::pair<int, int>* array, int p, int q) {
    int size = q - p + 1;
    int heapsize = size;

    stability_build_max_heap(array, p, q);

    for (int i = size - 1; i >= 1; i--) {
        copy_count += 3;
        std::pair<int, int> temp = array[p];
        array[p] = array[p + i];
        array[p + i] = temp;
        heapsize--;
        stability_max_heapify(array, p, 0, heapsize);
    }
}

int stability_partition(std::pair<int, int>* array, int p, int r) {
    int i = p - 1;
    int pivot = array[r].first;

    for (int j = p; j < r; j++) {
        if (array[j].first <= pivot) {
            i++;
            copy_count += 3;
            std::pair<int, int> temp = array[i];
            array[i] = array[j];
            array[j] = temp;
        }
    }
    copy_count += 3;
    std::pair<int, int> temp = array[i + 1];
    array[i + 1] = array[r];
    array[r] = temp;
    return i + 1;
}

void stability_introsort(std::pair<int, int>* array, int p, int q,
                         int max_depth) {
    int size = q - p + 1;
    if (size <= 16) {
        stability_insertion_sort(array, p, q);
    } else if (max_depth == 0) {
        stability_heap_sort(array, p, q);
    } else {
        int pivot = stability_partition(array, p, q);
        stability_introsort(array, p, pivot - 1, max_depth - 1);
        stability_introsort(array, pivot + 1, q, max_depth - 1);
    }
}

void stability_intro_sort(Stability_Dataset* data) {
    std::pair<int, int>* array = data->array;
    int size = 10000;
    int max_depth = 2 * (int)(log2(size) - 1);
    stability_introsort(array, 0, size - 1, max_depth);
}

int main() {
    vector<int> sizes = {1000, 10000, 100000, 1000000};
    vector<int> types = {1, 2, 3, 4, 5, 6};
    bool stable = true;
    Stability_Dataset* stability_data = new Stability_Dataset();

    ofstream outfile_stability("intro_sort_stability.txt");

    stability_data->generate_stability_data();
    stability_intro_sort(stability_data);
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

    ofstream outfile("intro_sort_results.csv");
    outfile << "type,size,trial,time_ms,copy_count,memory_"
               "kb\n";

    for (int type : types) {
        for (int size : sizes) {
            for (int trial = 1; trial <= 10; trial++) {
                copy_count = 0;
                Dataset* data = new Dataset(size, type);
                data->generate_data();

                auto start = high_resolution_clock::now();
                intro_sort(data);
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