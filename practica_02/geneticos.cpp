#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <chrono> // Librería para medir el tiempo



using namespace std;
using namespace std::chrono;

const int numCities = 100; 
const int populationSize = 400;
const int generations = 3000;
const double mutationRate = 0.2;

// Matriz de distancias entre ciudades
/*vector<vector<int>> distanceMatrix = {
    {0, 113, 56, 167, 147},
    {113, 0, 137, 142, 98},
    {56, 137, 0, 133, 135},
    {167, 142, 133, 0, 58},
    {147, 98, 135, 58, 0}
};*/
//Best order: 3 2 0 1 4 
//Best distance: 458
//Time elapsed: 56 milliseconds
/*vector<vector<int>> distanceMatrix = {
{0, 120, 50, 167, 147, 200},
{120, 0, 200, 142, 98, 45},
{50, 200, 0, 133, 135, 65},
{167, 142, 133, 0, 58, 50},
{147, 98, 135, 58, 0, 40},
{200, 45, 65, 50, 40, 0}
};*/
//Best order: 2 0 1 4 3 5 
//Best distance: 441
//Time elapsed: 63 milliseconds
/*vector<vector<int>> distanceMatrix = {
{0, 150, 50, 165, 147, 200, 40},
{150, 0, 200, 140, 90, 50, 30},
{50, 200, 0, 133, 135, 65, 80},
{165, 140, 133, 0, 58, 50, 40},
{147, 90, 135, 58, 0, 40, 30},
{200, 50, 65, 50, 40, 0, 74},
{40, 30, 80, 40, 30, 74, 0},
};*/
//Best order: 1 4 3 5 2 0 6 
//Best distance: 383
//Time elapsed: 64 milliseconds

/*vector<vector<int>> distanceMatrix = {
{0, 80, 40, 120, 140, 100, 40, 25},
{80, 0, 100, 140, 90, 50, 30, 45},
{40, 100, 0, 133, 135, 65, 80, 60},
{120, 140, 133, 0, 58, 50, 40, 34},
{140, 90, 135, 58, 0, 40, 30, 50},
{100, 50, 65, 50, 40, 0, 74, 80},
{40, 30, 80, 40, 30, 74, 0, 55},
{25, 45, 60, 34, 50, 80, 55, 0}
};*/
//Best order: 0 2 5 1 6 4 3 7 
//Best distance: 332
//Time elapsed: 79 milliseconds
/*vector<vector<int>> distanceMatrix = {
{0, 50, 100, 150, 140, 110, 40, 25, 20, 35},
{50, 0, 70, 140, 90, 50, 30, 45, 80, 60},
{100, 70, 0, 133, 135, 65, 80, 60, 45, 35},
{150, 140, 133, 0, 58, 50, 40, 34, 20, 70},
{140, 90, 135, 58, 0, 40, 30, 50, 65, 55},
{110, 50, 65, 50, 40, 0, 74, 80, 60, 40},
{40, 30, 80, 40, 30, 74, 0, 55, 25, 30},
{25, 45, 60, 34, 50,80, 55, 0, 50, 70},
{20, 80, 45, 20, 65, 60, 25, 50, 0, 100},
{35, 60, 35, 70, 55, 40, 30, 70, 100, 0}
};*/
//Best order: 0 8 3 7 1 6 4 5 2 9 
//Best distance: 354
//Time elapsed: 88 milliseconds
// Estructura para representar un individuo
/*vector<vector<int>> distanceMatrix = {
{0, 80, 40, 120, 147, 200, 40, 25, 20, 35, 50, 30},
{80, 0, 100, 140, 90, 50, 30, 45, 80, 60, 40, 60},
{40, 100, 0, 133, 135, 65, 80, 60, 45, 35, 20, 50},
{120, 140, 133, 0, 58, 50, 40, 34, 20, 70, 60, 70},
{147, 90, 135, 58, 0, 40, 30, 50, 65, 55, 50, 30},
{200, 50, 65, 50, 40, 0, 74, 80, 60, 40, 30, 30},
{40, 30, 80, 40, 30, 74, 0, 55, 25, 30, 20, 40},
{25, 45, 60, 34, 50, 80, 55, 0, 50, 70, 60, 55},
{20, 80, 45, 20, 65, 60, 25, 50, 0, 60, 40, 77},
{35, 60, 35, 70, 55, 40, 30, 70, 60, 0, 30, 45},
{50, 40, 20, 60, 50, 30, 20, 60, 40, 30, 0, 50},
{30, 60, 50, 70, 30, 30, 40, 55, 77, 45, 50, 0}
};*/
//Best order: 1 6 9 10 2 8 3 5 4 11 0 7 
//Best distance: 395
//Time elapsed: 85 milliseconds

/*vector<vector<int>> distanceMatrix = {
{0, 80, 40, 120, 147, 200, 40, 25, 20, 35, 50, 30, 40, 35, 80},
{80, 0, 100, 140, 90, 50, 30, 45, 80, 60, 40, 60, 70, 100, 99},
{40, 100, 0, 133, 135, 65, 80, 60, 45, 35, 20, 50, 100, 40, 100},
{120, 140, 133, 0, 58, 50, 40, 34, 20, 70, 60, 70, 60, 35, 50},
{147, 90, 135, 58, 0, 40, 30, 50, 65, 55, 50, 30, 80, 70, 90},
{200, 50, 65, 50, 40, 0, 74, 80, 60, 40, 30, 30, 60, 50, 80},
{40, 30, 80, 40, 30, 74, 0, 55, 25, 30, 20, 40, 90, 40, 70},
{25, 45, 60, 34, 50, 80, 55, 0, 50, 70, 60, 55, 75, 35, 45},
{20, 80, 45, 20, 65, 60, 25, 50, 0, 60, 40, 77, 80, 60, 80},
{35, 60, 35, 70, 55, 40, 30, 70, 60, 0, 30, 45, 60, 50, 90},
{50, 40, 20, 60, 50, 30, 20, 60, 40, 30, 0, 50, 44, 45, 60},
{30, 60, 50, 70, 30, 30, 40, 55, 77, 45, 50, 0, 50, 60, 100},
{40, 70, 100, 60, 80, 60, 90, 75, 80, 60, 44, 50, 0, 80, 90},
{35, 100, 40, 35, 70, 50, 40, 35, 60, 50, 45, 60, 80, 0, 65},
{80, 99, 100, 50, 90, 80, 70, 45, 80, 90, 60, 100, 90, 65, 0}
};*/
//Best order: 12 11 4 6 13 2 8 3 14 7 1 5 10 9 0 
//Best distance: 580
//Time elapsed: 103 milliseconds
vector<vector<int>> distanceMatrix;
// Generar una matriz de distancias aleatorias

struct Individual {
    vector<int> order; // Orden de visita de las ciudades
    double fitness;    // Valor de aptitud
};

// Función para calcular el valor de aptitud de un individuo
double calcFitness(const Individual& ind) {
    double distance = 0;
    for (int i = 0; i < numCities - 1; ++i) {
        distance += distanceMatrix[ind.order[i]][ind.order[i+1]];
    }
    distance += distanceMatrix[ind.order[numCities - 1]][ind.order[0]];
    return 1.0 / distance; // Mayor fitness para distancias menores
}

// Función para crear una población inicial de individuos aleatorios
vector<Individual> createInitialPopulation() {
    vector<Individual> population;
    for (int i = 0; i < populationSize; ++i) {
        Individual ind;
        ind.order.resize(numCities);
        for (int j = 0; j < numCities; ++j) {
            ind.order[j] = j; // Inicializar el orden con 0, 1, 2, ...
        }
        random_shuffle(ind.order.begin(), ind.order.end()); // Mezclar el orden de forma aleatoria
        ind.fitness = calcFitness(ind); // Calcular el valor de aptitud
        population.push_back(ind);
    }
    return population;
}

// Función para realizar el cruce entre dos padres y generar un hijo

vector<int> crossover(const vector<int>& parentA, const vector<int>& parentB) {
    int startPos = rand() % numCities;
    int endPos = rand() % numCities;
    if (startPos > endPos) {
        swap(startPos, endPos); // Asegurarse de que startPos sea menor que endPos
    }

    vector<int> child(numCities, -1); // Crear un hijo con valores iniciales -1

    for (int i = startPos; i <= endPos; ++i) {
        child[i] = parentA[i]; // Copiar la parte del padre A al hijo
    }

    int currIndex = 0;
    for (int i = 0; i < numCities; ++i) {
        if (currIndex == startPos) {
            currIndex = endPos + 1; // Saltar la parte ya copiada del padre A
        }
        if (find(child.begin(), child.end(), parentB[i]) == child.end()) {
            child[currIndex % numCities] = parentB[i]; // Copiar elementos del padre B que no estén en el hijo
            currIndex++;
        }
    }

    return child;
}

// Función para aplicar una mutación a un orden de ciudades
void mutate(vector<int>& order) {
    for (int i = 0; i < numCities; ++i) {
        if (static_cast<double>(rand()) / RAND_MAX < mutationRate) { // Probabilidad de mutación
            int indexA = rand() % numCities;
            int indexB = (indexA + 1) % numCities;
            swap(order[indexA], order[indexB]); // Intercambiar dos ciudades en el orden
        }
    }
}
// Función para generar una distancia aleatoria entre ciudades
int generateRandomDistance() {
    return rand() % 100 + 1; // Genera un número aleatorio entre 1 y 100
}

int main() {
    srand(time(nullptr)); // Inicializar la semilla para números aleatorios
    vector<vector<int>> distanceMatrix_a(numCities, vector<int>(numCities, 0));

    for (int i = 0; i < numCities; ++i) {
        for (int j = 0; j < numCities; ++j) {
            if (i == j) {
                distanceMatrix_a[i][j] = 0; // La distancia de una ciudad a sí misma es 0
            } else {
                distanceMatrix_a[i][j] = generateRandomDistance();
            }
        }
    }
    distanceMatrix = distanceMatrix_a;
     auto start = high_resolution_clock::now();
    // Crear una población inicial de individuos aleatorios
    vector<Individual> population = createInitialPopulation();

    // Bucle principal de evolución
    for (int gen = 0; gen < generations; ++gen) {
        vector<Individual> newPopulation;

        double bestFitness = population[0].fitness;
        int bestIndex = 0;

        // Encontrar el mejor individuo en la población actual
        for (int i = 1; i < populationSize; ++i) {
            if (population[i].fitness > bestFitness) {
                bestFitness = population[i].fitness;
                bestIndex = i;
            }
        }

        // Agregar el mejor individuo a la nueva población
        newPopulation.push_back(population[bestIndex]);

        // Realizar cruces y mutaciones para generar el resto de la nueva población
        while (newPopulation.size() < populationSize) {
            int indexA = rand() % populationSize;
            int indexB = rand() % populationSize;

            const vector<int>& parentA = population[indexA].order;
            const vector<int>& parentB = population[indexB].order;

            vector<int> child = crossover(parentA, parentB);
            mutate(child);

            Individual ind;
            ind.order = child;
            ind.fitness = calcFitness(ind);

            newPopulation.push_back(ind);
        }

        // Reemplazar la población actual con la nueva población generada
        population = newPopulation;
    }

    // Encontrar el mejor individuo en la población final
    double bestFitness = population[0].fitness;
    int bestIndex = 0;

    for (int i = 1; i < populationSize; ++i) {
        if (population[i].fitness > bestFitness) {
            bestFitness = population[i].fitness;
            bestIndex = i;
        }
    }

    const vector<int>& bestOrder = population[bestIndex].order;
    double bestDistance = 1.0 / bestFitness;

    // Imprimir el mejor orden y distancia encontrados
    cout << "Best order: ";
    for (int i = 0; i < numCities; ++i) {
        cout << bestOrder[i] << " ";
    }
    cout << endl;
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);

    cout << "Best distance: " << bestDistance << endl;
    cout << "Time elapsed: " << duration.count() << " milliseconds" << endl;
    cout << "-----------------------" << endl;

    return 0;
}
