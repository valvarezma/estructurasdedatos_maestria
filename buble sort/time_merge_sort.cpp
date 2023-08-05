#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <fstream>


using namespace std;
void export_to_csv_cpp(const vector<pair<int, double>>& results) {
    ofstream csvfile("results_cpp.csv");

    csvfile << "Tamaño de la lista, Tiempo de ejecución (segundos)" << endl;

    for (const auto& result : results) {
        csvfile << result.first << ", " << result.second << endl;
    }

    csvfile.close();
}

void bubbleSort(vector<int>& arr) {
    int n = arr.size();
    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < n - i - 1; ++j) {
            if (arr[j] > arr[j + 1]) {
                swap(arr[j], arr[j + 1]);
            }
        }
    }
}

double measureTimeSorting(void (*algorithm)(vector<int>&), int listSize) {
    vector<int> arr(listSize);
    srand(time(0));
    for (int i = 0; i < listSize; ++i) {
        arr[i] = rand() % 100000 + 1;
    }

    clock_t start_time = clock();
    algorithm(arr);
    clock_t end_time = clock();

    return static_cast<double>(end_time - start_time) / CLOCKS_PER_SEC;
}

int main() {
    vector<int> listSizes = {100, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000, 20000, 30000, 40000, 50000};

    cout << "Tiempos de ejecución para diferentes tamaños de entrada:" << endl;
    cout << "| Tamaño de la lista | Tiempo de ejecución (segundos) |" << endl;
    cout << "|--------------------|-------------------------------|" << endl;

    vector<pair<int, double>> results;

    for (int size : listSizes) {
        double executionTime = measureTimeSorting(bubbleSort, size);
        results.push_back(make_pair(size, executionTime));
        cout << "| " << size << "\t\t  | " << executionTime << "\t\t   |" << endl;
    }

    cout << "Resultados almacenados en el vector 'results':" << endl;
    for (const auto& result : results) {
        cout << result.first << " - " << result.second << endl;
    }


    return 0;
}
