#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <chrono>

void selectionSort(std::vector<int>& arr) {
    int n = arr.size();
    for (int i = 0; i < n - 1; i++) {
        int min_index = i;
        for (int j = i + 1; j < n; j++) {
            if (arr[j] < arr[min_index]) {
                min_index = j;
            }
        }
        std::swap(arr[i], arr[min_index]);
    }
}

double measureTimeSorting(void (*algorithm)(std::vector<int>&), int listSize) {
    std::vector<int> arr(listSize);
    srand(time(0));
    for (int i = 0; i < listSize; i++) {
        arr[i] = rand() % 100000 + 1;
    }

    auto startTime = std::chrono::high_resolution_clock::now();
    algorithm(arr);
    auto endTime = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> executionTime = endTime - startTime;
    return executionTime.count();
}

int main() {
    std::vector<int> listSizes = {100, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000, 20000, 30000, 40000, 50000};

    std::cout << "Tiempos de ejecución para diferentes tamaños de entrada:" << std::endl;
    std::cout << "| Tamaño de la lista | Tiempo de ejecución (segundos) |" << std::endl;
    std::cout << "|--------------------|-------------------------------|" << std::endl;

    std::ofstream outputFile("results_selection_sort.csv");
    outputFile << "Size,Time" << std::endl;

    for (int size : listSizes) {
        double executionTime = measureTimeSorting(selectionSort, size);
        std::cout << "| " << size << "\t\t  | " << executionTime << "\t\t   |" << std::endl;
        outputFile << size << "," << executionTime << std::endl;
    }

    outputFile.close();

    std::cout << "Resultados exportados a results_selection_sort.csv" << std::endl;

    return 0;
}
