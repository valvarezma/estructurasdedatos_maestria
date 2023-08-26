#include <windows.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <map>

struct DataPoint {
    std::vector<double> features;
    int label;
};

double calculateDistance(const std::vector<double>& p1, const std::vector<double>& p2) {
    // Función de cálculo de distancia
    // ...
}

int kNNClassify(const std::vector<DataPoint>& dataset, const std::vector<double>& input, int k) {
    // Función de clasificación k-NN
    // ...
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
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

    // Mostrar el resultado en una ventana de mensaje
    MessageBox(NULL, std::to_string(predictedLabel).c_str(), "Resultado de Clasificación", MB_OK | MB_ICONINFORMATION);

    return 0;
}
