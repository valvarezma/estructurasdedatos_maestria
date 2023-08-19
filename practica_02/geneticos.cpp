#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdlib>

using namespace std;

const int numCities = 5;
const int populationSize = 50;
const int generations = 1000;
const double mutationRate = 0.2;

// Matriz de distancias entre ciudades
vector<vector<int>> distanceMatrix = {
    {0, 113, 56, 167, 147},
    {113, 0, 137, 142, 98},
    {56, 137, 0, 133, 135},
    {167, 142, 133, 0, 58},
    {147, 98, 135, 58, 0}
};

// Estructura para representar un individuo
struct Individual {
    vector<int> order; // Orden de visita de las ciudades
    double fitness;    // Valor de aptitud
};

// Función para calcular el valor de aptitud de un individuo
double calcFitness(const Individual& ind) {
    double distance = 0;
    for (int i = 0; i < numCities - 1; ++i) {
        // Sumar la distancia entre ciudades adyacentes en el orden dado
        distance += distanceMatrix[ind.order[i]][ind.order[i+1]];
    }
    // Agregar la distancia de regreso a la ciudad inicial
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

// Función para aplicar una mutación a un individuo
void mutate(Individual& ind) {
    for (int i = 0; i < numCities; ++i) {
        if (static_cast<double>(rand()) / RAND_MAX < mutationRate) { // Probabilidad de mutación
            int indexA = rand() % numCities;
            int indexB = (indexA + 1) % numCities;
            swap(ind.order[indexA], ind.order[indexB]); // Intercambiar dos ciudades en el orden
        }
    }
    ind.fitness = calcFitness(ind); // Recalcular el valor de aptitud después de la mutación
}

int main() {
    srand(time(nullptr)); // Inicializar la semilla para números aleatorios

    // Crear una población inicial de individuos aleatorios
    vector<Individual> population = createInitialPopulation();

    // Bucle principal de evolución
    for (int gen = 0; gen < generations; ++gen) {
        vector<Individual> newPopulation;

        // Elitismo: Mantener el mejor individuo sin cambios en cada generación
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

    cout << "Best distance: " << bestDistance << endl;

    return 0;
}
