#include "dataset.h"

void Dataset::generate_data() {
    switch (type) {
        case 1:
            for (int i = 0; i < size; i++) array[i] = i;
            break;
        case 2:
            for (int i = 0; i < size; i++) array[i] = size - i - 1;
            break;
        case 3:
            for (int i = 0; i < size; i++) array[i] = rand() % size;
            break;
        case 4:
            for (int i = 0; i < size; i++) array[i] = i;
            std::shuffle(array, array + size, std::default_random_engine(43));
            std::sort(array, array + (int)(size * 0.5));
            break;
        case 5:
            for (int i = 0; i < size; i++) array[i] = i;
            std::shuffle(array, array + size, std::default_random_engine(43));
            std::sort(array, array + (int)(size * 0.99));
            break;
        case 6:
            for (int i = 0; i < size; i++)
                array[i] = (i % 2 == 0) ? i : size - i - 1;
            break;
        default:
            std::cerr << "Unknown dataset type: " << type << std::endl;
    }
}

