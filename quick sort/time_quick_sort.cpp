#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <chrono>

int partition(std::vector<int>& arr, int low, int high) {
    int pivot = arr[high];
    int i = low - 1;

    for (int j = low; j < high; j++) {
        if (arr[j] < pivot) {
            i++;
            std::swap(arr[i], arr[j]);
        }
    }

    std::swap(arr[i + 1], arr[high]);
    return i + 1;
}

void quickSort(std::vector<int>& arr, int low, int high) {
    if (low < high) {
        int pivotIndex = partition(arr, low, high);
        quickSort(arr, low, pivotIndex - 1);
        quickSort(arr, pivotIndex + 1, high);
    }
}

double measureTimeSorting(void (*algorithm)(std::vector<int>&, int, int), int listSize) {
    std::vector<int> arr(listSize);
    srand(time(0));
    for (int i = 0; i < listSize; i++) {
        arr[i] = rand() % 100000 + 1;
    }

    auto startTime = std::chrono::high_resolution_clock::now();
    algorithm(arr, 0, listSize - 1);
    auto endTime = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> executionTime = endTime - startTime;
    return executionTime.count();
}

int main() {
    std::vector<int> listSizes = {100, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000, 20000, 30000, 40000, 50000};

    std::cout << "Tiempos de ejecución para diferentes tamaños de entrada:" << std::endl;
    std::cout << "| Tamaño de la lista | Tiempo de ejecución (segundos) |" << std::endl;
    std::cout << "|--------------------|-------------------------------|" << std::endl;

    std::ofstream outputFile("results_quick_sort.csv");
    outputFile << "Size,Time" << std::endl;

    for (int size : listSizes) {
        double executionTime = measureTimeSorting(quickSort, size);
        std::cout << "| " << size << "\t\t  | " << executionTime << "\t\t   |" << std::endl;
        outputFile << size << "," << executionTime << std::endl;
    }

    outputFile.close();

    std::cout << "Resultados exportados a results_quick_sort.csv" << std::endl;

    return 0;
}