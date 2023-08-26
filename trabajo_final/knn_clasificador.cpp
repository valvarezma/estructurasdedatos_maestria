#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <map>
#include <windows.h>

struct DataPoint {
    std::vector<double> features;
    int label;
};

// Función para calcular la distancia euclidiana entre dos puntos
double calculateDistance(const std::vector<double>& p1, const std::vector<double>& p2) {
    double distance = 0.0;
    for (size_t i = 0; i < p1.size(); ++i) {
        distance += std::pow(p1[i] - p2[i], 2);
    }
    return std::sqrt(distance);
}

// Función para clasificar un punto usando el algoritmo k-NN
int kNNClassify(const std::vector<DataPoint>& dataset, const std::vector<double>& input, int k) {
    // Calcular distancias entre el punto de entrada y los puntos en el conjunto de datos
    std::vector<std::pair<double, int>> distances; // distancia, etiqueta

    for (const DataPoint& dataPoint : dataset) {
        double distance = calculateDistance(dataPoint.features, input);
        distances.emplace_back(distance, dataPoint.label);
    }

    // Ordenar las distancias de menor a mayor
    std::sort(distances.begin(), distances.end());

    // Contar las etiquetas de los k vecinos más cercanos
    std::map<int, int> labelCount;
    for (int i = 0; i < k; ++i) {
        labelCount[distances[i].second]++;
    }

    // Encontrar la etiqueta más común entre los k vecinos
    int maxCount = 0;
    int bestLabel = -1;
    for (const auto& pair : labelCount) {
        if (pair.second > maxCount) {
            maxCount = pair.second;
            bestLabel = pair.first;
        }
    }

    return bestLabel;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Tu código de aplicación aquí

 
        // Ejemplo de conjunto de datos
        std::vector<DataPoint> dataset = {
            {{2.0, 3.0}, 0},
            {{5.0, 8.0}, 1},
            {{1.0, 2.0}, 0},
            {{8.0, 9.0}, 1}
            // Agrega más puntos según sea necesario
        };

        // Punto de entrada para clasificar
        std::vector<double> input = {3.5, 5.5};

        // Valor de k para el algoritmo k-NN
        int k = 3;

        // Clasificar el punto de entrada
        int predictedLabel = kNNClassify(dataset, input, k);

        std::cout << "El punto pertenece a la etiqueta: " << predictedLabel << std::endl;

        return 0;
    
}
