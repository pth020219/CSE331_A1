

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

void selection_sort(Dataset* data, long& copy_count) {
    copy_count = 0;
    int size = data->size;
    int* array = data->array;
    int min_index, j, temp;

    for (int i = 0; i < size - 1; i++) {
        min_index = i;
        j = i + 1;
        while (j < size) {
            if (array[j] < array[min_index]) {
                min_index = j;
            }
            j++;
        }

        if (min_index != i) {
            copy_count += 3;
            temp = array[i];
            array[i] = array[min_index];
            array[min_index] = temp;
        }
    }
}

void stability_selection_sort(Stability_Dataset* data) {
    std::pair<int, int>* array = data->array;
    int min_index, j;
    std::pair<int, int> temp;

    for (int i = 0; i < 9999; i++) {
        min_index = i;
        j = i + 1;
        while (j < 10000) {
            if (array[j].first < array[min_index].first) min_index = j;
            j++;
        }

        if (min_index != i) {
            temp = array[i];
            array[i] = array[min_index];
            array[min_index] = temp;
        }
    }
}

int main() {
    vector<int> sizes = {1000, 10000, 100000, 1000000};
    vector<int> types = {1, 2, 3, 4, 5, 6};

    long copy_count = 0;
    bool stable = true;
    Stability_Dataset* stability_data = new Stability_Dataset();

    ofstream outfile_stability("selection_sort_stability.txt");

    stability_data->generate_stability_data();
    stability_selection_sort(stability_data);
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

    ofstream outfile("selection_sort_results.csv");
    outfile << "type,size,trial,time_ms,copy_count,memory_kb\n";

    for (int type : types) {
        for (int size : sizes) {
            for (int trial = 1; trial <= 10; trial++) {
                Dataset* data = new Dataset(size, type);
                data->generate_data();

                auto start = high_resolution_clock::now();
                selection_sort(data, copy_count);
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