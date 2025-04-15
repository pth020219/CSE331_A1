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

void merge(Dataset* data, int p, int q, int r) {
    int n1, n2, i, j;
    n1 = q - p + 1;
    n2 = r - q;
    int* left_arr = new int[n1 + 1];
    int* right_arr = new int[n2 + 1];

    for (int i = 0; i < n1; i++) {
        copy_count++;
        left_arr[i] = data->array[p + i];
    }

    for (int j = 0; j < n2; j++) {
        copy_count++;
        right_arr[j] = data->array[q + 1 + j];
    }

    copy_count += 2;
    left_arr[n1] = INT_MAX;
    right_arr[n2] = INT_MAX;

    i = 0;
    j = 0;

    for (int k = p; k <= r; k++) {
        if (left_arr[i] <= right_arr[j]) {
            copy_count++;
            data->array[k] = left_arr[i];
            i++;
        } else {
            copy_count++;
            data->array[k] = right_arr[j];
            j++;
        }
    }

    delete[] left_arr;
    delete[] right_arr;
}

void merge_sort(Dataset* data, int p, int r) {
    if (p < r) {
        int q = (p + r) / 2;
        merge_sort(data, p, q);
        merge_sort(data, q + 1, r);
        merge(data, p, q, r);
    }
}

void stability_merge(Stability_Dataset* data, int p, int q, int r) {
    int n1, n2, i, j;
    n1 = q - p + 1;
    n2 = r - q;
    std::pair<int, int>* left_arr = new std::pair<int, int>[n1 + 1];
    std::pair<int, int>* right_arr = new std::pair<int, int>[n2 + 1];

    for (int i = 0; i < n1; i++) {
        left_arr[i] = data->array[p + i];
    }

    for (int j = 0; j < n2; j++) {
        right_arr[j] = data->array[q + 1 + j];
    }

    left_arr[n1].first = INT_MAX;
    right_arr[n2].first = INT_MAX;

    i = 0;
    j = 0;

    for (int k = p; k <= r; k++) {
        if (left_arr[i].first <= right_arr[j].first) {
            data->array[k] = left_arr[i];
            i++;
        } else {
            data->array[k] = right_arr[j];
            j++;
        }
    }

    delete[] left_arr;
    delete[] right_arr;
}

void stability_merge_sort(Stability_Dataset* data, int p, int r) {
    if (p < r) {
        int q = (p + r) / 2;
        stability_merge_sort(data, p, q);
        stability_merge_sort(data, q + 1, r);
        stability_merge(data, p, q, r);
    }
}

int main() {
    vector<int> sizes = {1000, 10000, 100000, 1000000};
    vector<int> types = {1, 2, 3, 4, 5, 6};
    bool stable = true;
    Stability_Dataset* stability_data = new Stability_Dataset();

    ofstream outfile_stability("merge_sort_stability.txt");

    stability_data->generate_stability_data();
    stability_merge_sort(stability_data, 0, 9999);
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

    ofstream outfile("merge_sort_results.csv");
    outfile << "type,size,trial,time_ms,copy_count,memory_kb\n";

    for (int type : types) {
        for (int size : sizes) {
            for (int trial = 1; trial <= 10; trial++) {
                copy_count = 0;
                Dataset* data = new Dataset(size, type);
                data->generate_data();

                auto start = high_resolution_clock::now();
                merge_sort(data, 0, size - 1);
                auto end = high_resolution_clock::now();

                auto time_ms =
                    std::chrono::duration<double, std::milli>(end - start)
                        .count();
                long memory_kb = getMemoryUsageKB();

                outfile << type << "," << size << "," << trial << "," << time_ms
                        << "," << copy_count << "," << memory_kb << "\n";
                outfile.flush();
                delete data;
                cout << "Trial " << trial << " for type " << type
                     << " and size " << size << " completed." << endl;
            }
        }
    }
    outfile.close();
    malloc_trim(0);

    return 0;
}