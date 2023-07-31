#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <fstream>


using namespace std;

void merge(vector<int>& arr, int left, int mid, int right) {
    int left_size = mid - left + 1;
    int right_size = right - mid;

    vector<int> left_half(left_size), right_half(right_size);

    for (int i = 0; i < left_size; ++i)
        left_half[i] = arr[left + i];

    for (int j = 0; j < right_size; ++j)
        right_half[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;

    while (i < left_size && j < right_size) {
        if (left_half[i] <= right_half[j]) {
            arr[k] = left_half[i];
            ++i;
        }
        else {
            arr[k] = right_half[j];
            ++j;
        }
        ++k;
    }

    while (i < left_size) {
        arr[k] = left_half[i];
        ++i;
        ++k;
    }

    while (j < right_size) {
        arr[k] = right_half[j];
        ++j;
        ++k;
    }
}

void merge_sort(vector<int>& arr, int left, int right) {
    if (left >= right)
        return;

    int mid = left + (right - left) / 2;
    merge_sort(arr, left, mid);
    merge_sort(arr, mid + 1, right);
    merge(arr, left, mid, right);
}

double measure_time_sorting(int list_size) {
    // Generar lista de números desordenados aleatorios
    vector<int> arr(list_size);
    srand(time(0));

    for (int i = 0; i < list_size; ++i)
        arr[i] = rand() % 100000 + 1;

    // Medir tiempo de ejecución
    clock_t start_time = clock();
    merge_sort(arr, 0, list_size - 1);
    clock_t end_time = clock();

    return static_cast<double>(end_time - start_time) / CLOCKS_PER_SEC;
}

void export_to_csv_cpp(const vector<pair<int, double>>& results) {
    ofstream csvfile("results_cpp.csv");

    csvfile << "Tamaño de la lista, Tiempo de ejecución (segundos)" << endl;

    for (const auto& result : results) {
        csvfile << result.first << ", " << result.second << endl;
    }

    csvfile.close();
}

int main() {
    vector<int> list_sizes = {100, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000, 20000, 30000, 40000, 50000};

    cout << "Tiempos de ejecución para diferentes tamaños de entrada:" << endl;
    cout << "| Tamaño de la lista | Tiempo de ejecución (segundos) |" << endl;
    cout << "|--------------------|-------------------------------|" << endl;
    vector<pair<int, double>> results;
    for (int size : list_sizes) {
        double execution_time = measure_time_sorting(size);
        cout << "| " << size << "\t\t  | " << execution_time << "\t\t   |" << endl;
        results.push_back(make_pair(size, execution_time));
    }
    export_to_csv_cpp(results);

    return 0;
}
