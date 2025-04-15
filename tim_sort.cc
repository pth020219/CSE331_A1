
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

void insertion_sort(int* array, int start, int end) {
    for (int i = start + 1; i <= end; i++) {
        copy_count++;
        int key = array[i];
        int j = i - 1;

        while (j >= start && array[j] > key) {
            copy_count++;
            array[j + 1] = array[j];
            j--;
        }
        copy_count++;
        array[j + 1] = key;
    }
}

int count_run(int* array, int start, int size) {
    if (start == size - 1) return 1;

    int run_len = 1;
    if (array[start] <= array[start + 1]) {
        while ((start + run_len < size) &&
               (array[start + run_len - 1] <= array[start + run_len])) {
            run_len++;
        }
    } else {
        while ((start + run_len < size) &&
               (array[start + run_len - 1] > array[start + run_len])) {
            run_len++;
        }

        int left = start;
        int right = start + run_len - 1;
        while (left < right) {
            copy_count += 3;
            int temp = array[left];
            array[left] = array[right];
            array[right] = temp;
            left++;
            right--;
        }
    }
    return run_len;
}

void merge(int* array, int p, int q, int r) {
    int n1 = q - p + 1;
    int n2 = r - q;
    int* left_arr = new int[n1 + 1];
    int* right_arr = new int[n2 + 1];

    for (int i = 0; i < n1; i++) {
        copy_count++;
        left_arr[i] = array[p + i];
    }

    for (int j = 0; j < n2; j++) {
        copy_count++;
        right_arr[j] = array[q + 1 + j];
    }
    copy_count += 2;
    left_arr[n1] = INT_MAX;
    right_arr[n2] = INT_MAX;

    int i = 0, j = 0;
    for (int k = p; k <= r; k++) {
        if (left_arr[i] <= right_arr[j]) {
            copy_count++;

            array[k] = left_arr[i];
            i++;
        } else {
            copy_count++;

            array[k] = right_arr[j];
            j++;
        }
    }

    delete[] left_arr;
    delete[] right_arr;
}

void merge_at(vector<pair<int, int>>& run_stack, int i, int* array) {
    copy_count += 2;
    auto& run_1 = run_stack[i];
    auto& run_2 = run_stack[i + 1];

    int p = run_1.first;
    int q = p + run_1.second - 1;
    int r = run_2.first + run_2.second - 1;

    merge(array, p, q, r);

    copy_count++;
    run_stack[i] = {run_1.first, run_1.second + run_2.second};

    run_stack.erase(run_stack.begin() + (i + 1));
}

void merge_collapse(vector<pair<int, int>>& run_stack, int* array) {
    while (run_stack.size() > 1) {
        int n = static_cast<int>(run_stack.size());

        if (n >= 3 && run_stack[n - 3].second <=
                          (run_stack[n - 2].second + run_stack[n - 1].second)) {
            if (run_stack[n - 3].second < run_stack[n - 1].second) {
                merge_at(run_stack, n - 3, array);
            } else {
                merge_at(run_stack, n - 2, array);
            }
        } else if (run_stack[n - 2].second <= run_stack[n - 1].second) {
            merge_at(run_stack, n - 2, array);
        } else {
            break;
        }
    }
}

void merge_force_collapse(vector<pair<int, int>>& run_stack, int* array) {
    while (run_stack.size() > 1) {
        merge_at(run_stack, static_cast<int>(run_stack.size()) - 2, array);
    }
}

void tim_sort(Dataset* data) {
    int* array = data->array;
    int size = data->size;

    int n = size;
    int r = 0;
    while (n >= 64) {
        r |= (n & 1);
        n /= 2;
    }
    int min_run = n + r;

    vector<pair<int, int>> run_stack;
    run_stack.reserve(64);

    int i = 0;
    while (i < size) {
        int run_len = count_run(array, i, size);

        if (run_len < min_run) {
            int forced_run_len = std::min(min_run, size - i);
            insertion_sort(array, i, i + forced_run_len - 1);
            run_len = forced_run_len;
        }

        copy_count++;
        run_stack.push_back({i, run_len});
        merge_collapse(run_stack, array);
        i += run_len;
    }
    merge_force_collapse(run_stack, array);
}

void stability_insertion_sort(std::pair<int, int>* array, int start, int end) {
    for (int i = start + 1; i <= end; i++) {
        std::pair<int, int> key = array[i];
        int j = i - 1;

        while (j >= start && array[j].first > key.first) {
            array[j + 1] = array[j];
            j--;
        }
        array[j + 1] = key;
    }
}

int stability_count_run(std::pair<int, int>* array, int start, int size) {
    if (start == size - 1) return 1;
    int run_len = 1;

    if (array[start].first <= array[start + 1].first) {
        while ((start + run_len < size) && (array[start + run_len - 1].first <=
                                            array[start + run_len].first)) {
            run_len++;
        }
    } else {
        while ((start + run_len < size) && (array[start + run_len - 1].first >
                                            array[start + run_len].first)) {
            run_len++;
        }
        int left = start;
        int right = start + run_len - 1;
        while (left < right) {
            copy_count++;
            std::pair<int, int> temp = array[left];
            array[left] = array[right];
            array[right] = temp;
            left++;
            right--;
        }
    }

    return run_len;
}

void stability_merge(std::pair<int, int>* array, int p, int q, int r) {
    int n1 = q - p + 1;
    int n2 = r - q;

    std::pair<int, int>* left_arr = new std::pair<int, int>[n1 + 1];
    std::pair<int, int>* right_arr = new std::pair<int, int>[n2 + 1];

    for (int i = 0; i < n1; i++) {
        left_arr[i] = array[p + i];
    }
    for (int j = 0; j < n2; j++) {
        right_arr[j] = array[q + 1 + j];
    }

    left_arr[n1] = {INT_MAX, -1};
    right_arr[n2] = {INT_MAX, -1};

    int i = 0, j = 0;
    for (int k = p; k <= r; k++) {
        if (left_arr[i].first <= right_arr[j].first) {
            array[k] = left_arr[i++];
        } else {
            array[k] = right_arr[j++];
        }
    }

    delete[] left_arr;
    delete[] right_arr;
}

// 벡터 버전 merge_at
void stability_merge_at(vector<pair<int, int>>& run_stack, int i,
                        std::pair<int, int>* array) {
    auto& run_1 = run_stack[i];
    auto& run_2 = run_stack[i + 1];

    int p = run_1.first;
    int q = p + run_1.second - 1;
    int r = run_2.first + run_2.second - 1;

    stability_merge(array, p, q, r);

    run_stack[i] = {run_1.first, run_1.second + run_2.second};
    run_stack.erase(run_stack.begin() + (i + 1));
}

void stability_merge_collapse(vector<pair<int, int>>& run_stack,
                              std::pair<int, int>* array) {
    while (run_stack.size() > 1) {
        int n = static_cast<int>(run_stack.size());
        if (n >= 3 && run_stack[n - 3].second <=
                          run_stack[n - 2].second + run_stack[n - 1].second) {
            if (run_stack[n - 3].second < run_stack[n - 1].second) {
                stability_merge_at(run_stack, n - 3, array);
            } else {
                stability_merge_at(run_stack, n - 2, array);
            }
        } else if (run_stack[n - 2].second <= run_stack[n - 1].second) {
            stability_merge_at(run_stack, n - 2, array);
        } else {
            break;
        }
    }
}

void stability_merge_force_collapse(vector<pair<int, int>>& run_stack,
                                    std::pair<int, int>* array) {
    while (run_stack.size() > 1) {
        stability_merge_at(run_stack, static_cast<int>(run_stack.size()) - 2,
                           array);
    }
}

void stability_tim_sort(Stability_Dataset* data) {
    std::pair<int, int>* array = data->array;
    int size = 10000;
    int n = size;
    int r = 0;
    while (n >= 64) {
        r |= (n & 1);
        n /= 2;
    }
    int min_run = n + r;

    // run stack
    vector<pair<int, int>> run_stack;
    run_stack.reserve(64);

    int i = 0;
    while (i < size) {
        int run_len = stability_count_run(array, i, size);
        if (run_len < min_run) {
            int forced_run_len = std::min(min_run, size - i);
            stability_insertion_sort(array, i, i + forced_run_len - 1);
            run_len = forced_run_len;
        }
        run_stack.push_back({i, run_len});
        stability_merge_collapse(run_stack, array);
        i += run_len;
    }
    stability_merge_force_collapse(run_stack, array);
}

int main() {
    vector<int> sizes = {1000, 10000, 100000, 1000000};
    vector<int> types = {1, 2, 3, 4, 5, 6};
    bool stable = true;
    Stability_Dataset* stability_data = new Stability_Dataset();

    ofstream outfile_stability("tim_sort_stability.txt");

    stability_data->generate_stability_data();
    stability_tim_sort(stability_data);
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

    ofstream outfile("tim_sort_results.csv");
    outfile << "type,size,trial,time_ms,copy_count,memory_"
               "kb\n";

    for (int type : types) {
        for (int size : sizes) {
            for (int trial = 1; trial <= 10; trial++) {
                copy_count = 0;
                Dataset* data = new Dataset(size, type);
                data->generate_data();

                auto start = high_resolution_clock::now();
                tim_sort(data);
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