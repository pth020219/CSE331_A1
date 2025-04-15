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

int partition(Dataset* data, int p, int r) {
    int i;
    int* array = data->array;

    copy_count++;
    int pivot = array[r];
    i = p - 1;

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

void quick_sort(Dataset* data, int p, int r) {
    if (p < r) {
        int q = partition(data, p, r);
        quick_sort(data, p, q - 1);
        quick_sort(data, q + 1, r);
    }
}

int stability_partition(Stability_Dataset* data, int p, int r) {
    int i;
    std::pair<int, int>* array = data->array;
    int pivot = array[r].first;
    i = p - 1;

    for (int j = p; j < r; j++) {
        if (array[j].first <= pivot) {
            i++;
            std::pair<int, int> temp = array[i];
            array[i] = array[j];
            array[j] = temp;
        }
    }
    std::pair<int, int> temp = array[i + 1];
    array[i + 1] = array[r];
    array[r] = temp;
    return i + 1;
}

void stability_quick_sort(Stability_Dataset* data, int p, int r) {
    if (p < r) {
        int q = stability_partition(data, p, r);
        stability_quick_sort(data, p, q - 1);
        stability_quick_sort(data, q + 1, r);
    }
}

int main() {
    vector<int> sizes = {1000, 10000, 100000, 1000000};
    vector<int> types = {3, 4, 5};
    bool stable = true;
    Stability_Dataset* stability_data = new Stability_Dataset();

    ofstream outfile_stability("quick_sort_stability.txt");

    stability_data->generate_stability_data();
    stability_quick_sort(stability_data, 0, 9999);
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
    outfile_stability << (stable ? "Stable" : "Unstable") << "\n";

    outfile_stability.close();

    ofstream outfile("quick_sort_results.csv");
    outfile << "type,size,trial,time_ms,copy_count,memory_kb\n";

    for (int type : types) {
        for (int size : sizes) {
            for (int trial = 1; trial <= 10; trial++) {
                copy_count = 0;
                Dataset* data = new Dataset(size, type);
                data->generate_data();

                auto start = high_resolution_clock::now();
                quick_sort(data, 0, size - 1);
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