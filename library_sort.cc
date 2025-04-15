#include <limits.h>
#include <malloc.h>

#include <chrono>
#include <cmath>
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

void library_sort(Dataset* data) {
    int* A = data->array;
    int n = data->size;

    int m = 2 * n;
    int* B = new int[m];
    bool* used = new bool[m];

    for (int i = 0; i < m; i++) {
        copy_count++;
        used[i] = false;
    }

    vector<int> pos;

    int mid = m / 2;
    copy_count += 3;
    B[mid] = A[0];
    used[mid] = true;
    pos.push_back(mid);
    int count = 1;

    auto rebalance = [&]() {
        vector<int> temp;

        for (int idx : pos) {
            copy_count++;
            temp.push_back(B[idx]);
        }

        copy_count += m;
        fill(used, used + m, false);
        pos.clear();

        for (int i = 0; i < count; i++) {
            int new_index = (i * m) / count;
            copy_count += 3;
            B[new_index] = temp[i];
            used[new_index] = true;
            pos.push_back(new_index);
        }
    };

    for (int i = 1; i < n; i++) {
        copy_count++;
        int x = A[i];

        int lo = 0, hi = pos.size();
        while (lo < hi) {
            int midIdx = lo + (hi - lo) / 2;
            if (B[pos[midIdx]] < x) {
                lo = midIdx + 1;
            } else {
                hi = midIdx;
            }
        }
        int insertion_rank = lo;

        int desired_index;
        if (pos.empty()) {
            desired_index = m / 2;
        } else if (insertion_rank == 0) {
            if (pos[0] > 0) {
                desired_index = pos[0] - 1;
            } else {
                desired_index = pos[0] + 1;
            }
        } else if (insertion_rank == pos.size()) {
            if (pos.back() < m - 1) {
                desired_index = pos.back() + 1;
            } else {
                desired_index = pos.back() - 1;
            }
        } else {
            desired_index = (pos[insertion_rank - 1] + pos[insertion_rank]) / 2;
        }

        int pos_to_insert = -1;
        int offset = 0;
        bool found = false;
        while (!found) {
            int left = desired_index - offset;
            int right = desired_index + offset;
            if (left >= 0 && left < m && !used[left]) {
                pos_to_insert = left;
                found = true;
                break;
            }
            if (right >= 0 && right < m && !used[right]) {
                pos_to_insert = right;
                found = true;
                break;
            }
            offset++;

            if (desired_index - offset < 0 && desired_index + offset >= m) {
                break;
            }
        }

        if (!found) {
            rebalance();

            lo = 0, hi = pos.size();
            while (lo < hi) {
                int midIdx = lo + (hi - lo) / 2;
                if (B[pos[midIdx]] < x) {
                    lo = midIdx + 1;
                } else {
                    hi = midIdx;
                }
            }
            insertion_rank = lo;
            if (pos.empty()) {
                desired_index = m / 2;
            } else if (insertion_rank == 0) {
                if (pos[0] > 0)
                    desired_index = pos[0] - 1;
                else
                    desired_index = pos[0] + 1;
            } else if (insertion_rank == pos.size()) {
                if (pos.back() < m - 1)
                    desired_index = pos.back() + 1;
                else
                    desired_index = pos.back() - 1;
            } else {
                desired_index =
                    (pos[insertion_rank - 1] + pos[insertion_rank]) / 2;
            }
            offset = 0;
            found = false;
            while (!found) {
                int left = desired_index - offset;
                int right = desired_index + offset;
                if (left >= 0 && left < m && !used[left]) {
                    pos_to_insert = left;
                    found = true;
                    break;
                }
                if (right >= 0 && right < m && !used[right]) {
                    pos_to_insert = right;
                    found = true;
                    break;
                }
                offset++;
            }
        }

        copy_count += 3;
        B[pos_to_insert] = x;
        used[pos_to_insert] = true;

        pos.insert(pos.begin() + insertion_rank, pos_to_insert);
        count++;
    }

    for (int i = 0; i < count; i++) {
        copy_count++;
        A[i] = B[pos[i]];
    }

    delete[] B;
    delete[] used;
}

void stability_library_sort(Stability_Dataset* data) {
    std::pair<int, int>* A = data->array;
    int n = 10000;

    int m = 2 * n;
    std::pair<int, int>* B = new std::pair<int, int>[m];
    bool* used = new bool[m];

    for (int i = 0; i < m; i++) {
        used[i] = false;
    }

    std::vector<int> pos;

    int mid = m / 2;
    B[mid] = A[0];
    used[mid] = true;
    pos.push_back(mid);
    int count = 1;

    auto rebalance = [&]() {
        std::vector<std::pair<int, int>> temp;

        for (int idx : pos) {
            temp.push_back(B[idx]);
        }

        std::fill(used, used + m, false);
        pos.clear();

        for (int i = 0; i < count; i++) {
            int new_index = (i * m) / count;
            B[new_index] = temp[i];
            used[new_index] = true;
            pos.push_back(new_index);
        }
    };

    for (int i = 1; i < n; i++) {
        std::pair<int, int> x = A[i];

        int lo = 0, hi = pos.size();
        while (lo < hi) {
            int midIdx = lo + (hi - lo) / 2;
            if (B[pos[midIdx]].first <= x.first)
                lo = midIdx + 1;
            else
                hi = midIdx;
        }
        int insertion_rank = lo;

        int desired_index;
        if (pos.empty()) {
            desired_index = m / 2;
        } else if (insertion_rank == 0) {
            if (pos[0] > 0)
                desired_index = pos[0] - 1;
            else
                desired_index = pos[0] + 1;
        } else if (insertion_rank == pos.size()) {
            if (pos.back() < m - 1)
                desired_index = pos.back() + 1;
            else
                desired_index = pos.back() - 1;
        } else {
            desired_index = (pos[insertion_rank - 1] + pos[insertion_rank]) / 2;
        }

        int pos_to_insert = -1;
        int offset = 0;
        bool found = false;
        while (!found) {
            int left = desired_index - offset;
            int right = desired_index + offset;
            if (left >= 0 && left < m && !used[left]) {
                pos_to_insert = left;
                found = true;
                break;
            }
            if (right >= 0 && right < m && !used[right]) {
                pos_to_insert = right;
                found = true;
                break;
            }
            offset++;
            if (desired_index - offset < 0 && desired_index + offset >= m)
                break;
        }

        if (!found) {
            rebalance();
            lo = 0, hi = pos.size();
            while (lo < hi) {
                int midIdx = lo + (hi - lo) / 2;
                if (B[pos[midIdx]].first <= x.first)
                    lo = midIdx + 1;
                else
                    hi = midIdx;
            }
            insertion_rank = lo;
            if (pos.empty()) {
                desired_index = m / 2;
            } else if (insertion_rank == 0) {
                if (pos[0] > 0)
                    desired_index = pos[0] - 1;
                else
                    desired_index = pos[0] + 1;
            } else if (insertion_rank == pos.size()) {
                if (pos.back() < m - 1)
                    desired_index = pos.back() + 1;
                else
                    desired_index = pos.back() - 1;
            } else {
                desired_index =
                    (pos[insertion_rank - 1] + pos[insertion_rank]) / 2;
            }
            offset = 0;
            found = false;
            while (!found) {
                int left = desired_index - offset;
                int right = desired_index + offset;
                if (left >= 0 && left < m && !used[left]) {
                    pos_to_insert = left;
                    found = true;
                    break;
                }
                if (right >= 0 && right < m && !used[right]) {
                    pos_to_insert = right;
                    found = true;
                    break;
                }
                offset++;
            }
        }
        B[pos_to_insert] = x;
        used[pos_to_insert] = true;
        pos.insert(pos.begin() + insertion_rank, pos_to_insert);
        count++;
    }

    for (int i = 0; i < count; i++) {
        A[i] = B[pos[i]];
    }

    delete[] B;
    delete[] used;
}

int main() {
    vector<int> sizes = {1000, 10000, 100000, 1000000};
    vector<int> types = {1, 2, 3, 4, 5, 6};
    bool stable = true;
    Stability_Dataset* stability_data = new Stability_Dataset();

    ofstream outfile_stability("library_sort_stability.txt");

    stability_data->generate_stability_data();
    stability_library_sort(stability_data);
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

    ofstream outfile("library_sort_results.csv");
    outfile << "type,size,trial,time_ms,copy_count,memory_"
               "kb\n";

    for (int type : types) {
        for (int size : sizes) {
            for (int trial = 1; trial <= 10; trial++) {
                copy_count = 0;
                Dataset* data = new Dataset(size, type);
                data->generate_data();

                auto start = high_resolution_clock::now();
                library_sort(data);
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