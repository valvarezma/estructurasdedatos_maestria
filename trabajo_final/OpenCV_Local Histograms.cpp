#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include "tinyxml2.h"
#include <omp.h> // Para paralelización

struct ImageDataPoint {
    cv::Mat histogram; // Histograma de la imagen en escala de grises
    std::string label;  // Etiqueta de la imagen
};

// Estructura para un nodo del k-d tree
struct KDTreeNode {
    cv::Mat histogram;
    std::string label;
    KDTreeNode* left;
    KDTreeNode* right;

    KDTreeNode(const cv::Mat& hist, const std::string& lbl) : histogram(hist), label(lbl), left(nullptr), right(nullptr) {}
};

// Función para obtener la etiqueta de una imagen desde un archivo XML
std::string getLabelFromXML(const std::string& xmlPath) {
    tinyxml2::XMLDocument doc;

    // Cargar el archivo XML
    if (doc.LoadFile(xmlPath.c_str()) != tinyxml2::XML_SUCCESS) {
        std::cerr << "Error al cargar el archivo XML: " << xmlPath << std::endl;
        return "unknown"; // Valor de etiqueta predeterminado en caso de error
    }

    tinyxml2::XMLElement* root = doc.FirstChildElement("annotation");

    // Verificar si se encontró el elemento 'annotation'
    if (!root) {
        std::cerr << "Elemento 'annotation' no encontrado en el archivo XML: " << xmlPath << std::endl;
        return "unknown"; // Valor de etiqueta predeterminado en caso de error
    }

    tinyxml2::XMLElement* object = root->FirstChildElement("object");

    // Verificar si se encontró el elemento 'object'
    if (!object) {
        std::cerr << "Elemento 'object' no encontrado en el archivo XML: " << xmlPath << std::endl;
        return "unknown"; // Valor de etiqueta predeterminado en caso de error
    }

    tinyxml2::XMLElement* name = object->FirstChildElement("name");

    // Verificar si se encontró el elemento 'name'
    if (!name) {
        std::cerr << "Elemento 'name' no encontrado en el archivo XML: " << xmlPath << std::endl;
        return "unknown"; // Valor de etiqueta predeterminado en caso de error
    }

    const char* labelStr = name->GetText();

    // Verificar si se encontró el valor de etiqueta
    if (!labelStr) {
        std::cerr << "Valor de etiqueta no encontrado en el archivo XML: " << xmlPath << std::endl;
        return "unknown"; // Valor de etiqueta predeterminado en caso de error
    }

    return labelStr;
}

// Función para generar el histograma en escala de grises de una imagen
cv::Mat generateGrayscaleHistogram(const cv::Mat& image) {
    // Verificar si se cargó la imagen correctamente
    if (image.empty()) {
        std::cerr << "Error: La imagen está vacía." << std::endl;
        return cv::Mat(); // Devolver una matriz vacía en caso de error
    }

    int numBins = 256; // Número de bins para el histograma (0-255)

    // Calcular el histograma
    cv::Mat hist;
    int channels[] = { 0 }; // Canal 0 (escala de grises)
    int histSize[] = { numBins }; // Número de bins
    float valueRanges[] = { 0, 256 }; // Rango de valores (0-255)
    const float* ranges[] = { valueRanges };
    cv::calcHist(&image, 1, channels, cv::Mat(), hist, 1, histSize, ranges, true, false);

    return hist; // Devolver el histograma en escala de grises
}

// Función para generar datos de entrenamiento a partir de imágenes y archivos XML
std::vector<ImageDataPoint> generateTrainingData(const std::string& folderPath, int numImages) {
    std::vector<ImageDataPoint> trainingData;
    trainingData.reserve(numImages); // Reservar espacio para evitar reallocaciones

#pragma omp parallel for
    for (int i = 0; i < numImages; ++i) {
        std::string imagePath = folderPath + "/images/road" + std::to_string(i) + ".png";
        std::string xmlPath = folderPath + "/annotations/road" + std::to_string(i) + ".xml";

        cv::Mat image = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
        cv::Mat histogram = generateGrayscaleHistogram(image);

        // Verificar si se generó el histograma y la etiqueta no es "unknown"
        if (!histogram.empty()) {
            std::string label = getLabelFromXML(xmlPath);
            if (label != "unknown") {
                ImageDataPoint dataPoint;
                dataPoint.histogram = histogram;
                dataPoint.label = label;
#pragma omp critical
                trainingData.push_back(dataPoint);
            }
        }
    }

    return trainingData;
}

// Función para construir un árbol k-d a partir de los datos de entrenamiento
KDTreeNode* buildKDTree(std::vector<ImageDataPoint>& dataset, int depth) {
    if (dataset.empty()) {
        return nullptr;
    }

    int dimensions = dataset[0].histogram.rows;

    // Elegir la dimensión actual como la que tiene la mayor varianza
    int axis = depth % dimensions;

    // Ordenar el dataset en función de la dimensión actual
    std::sort(dataset.begin(), dataset.end(), [axis](const ImageDataPoint& a, const ImageDataPoint& b) {
        return a.histogram.at<float>(axis) < b.histogram.at<float>(axis);
        });

    // Encontrar el valor mediano
    int medianIndex = dataset.size() / 2;

    // Crear el nodo actual y construir recursivamente los subárboles
    KDTreeNode* node = new KDTreeNode(
        dataset[medianIndex].histogram.clone(),
        dataset[medianIndex].label
    );

    std::vector<ImageDataPoint> leftDataset(dataset.begin(), dataset.begin() + medianIndex);
    std::vector<ImageDataPoint> rightDataset(dataset.begin() + medianIndex + 1, dataset.end());

    node->left = buildKDTree(leftDataset, depth + 1);
    node->right = buildKDTree(rightDataset, depth + 1);

    return node;
}

// Función para calcular la distancia entre dos histogramas utilizando el k-d tree
double calculateDistance(const cv::Mat& histogram1, const cv::Mat& histogram2) {
    if (histogram1.empty() || histogram2.empty()) {
        std::cerr << "Error: Uno o ambos histogramas están vacíos." << std::endl;
        return std::numeric_limits<double>::max();
    }

    if (histogram1.size() != histogram2.size()) {
        std::cerr << "Error: Las dimensiones de los histogramas no son compatibles." << std::endl;
        return std::numeric_limits<double>::max();
    }

    return cv::compareHist(histogram1, histogram2, cv::HISTCMP_CHISQR_ALT);
}

// Función para realizar una búsqueda en el k-d tree y encontrar el vecino más cercano
void searchNearestNeighbor(
    KDTreeNode* currentNode,
    const cv::Mat& targetHistogram,
    double& bestDistance,
    std::string& bestLabel,
    int depth
) {
    if (currentNode == nullptr) {
        return;
    }

    int dimensions = targetHistogram.rows;
    int axis = depth % dimensions;

    double currentDistance = calculateDistance(targetHistogram, currentNode->histogram);

    if (currentDistance < bestDistance) {
        bestDistance = currentDistance;
        bestLabel = currentNode->label;
    }

    // Calcular la distancia mínima entre el plano de corte y el objetivo
    double planeDistance = targetHistogram.at<float>(axis) - currentNode->histogram.at<float>(axis);

    KDTreeNode* nearerNode;
    KDTreeNode* furtherNode;

    if (planeDistance < 0) {
        nearerNode = currentNode->left;
        furtherNode = currentNode->right;
    }
    else {
        nearerNode = currentNode->right;
        furtherNode = currentNode->left;
    }

    searchNearestNeighbor(nearerNode, targetHistogram, bestDistance, bestLabel, depth + 1);

    // Poda del árbol si la distancia en el eje actual es mayor que la mejor distancia actual
    if (planeDistance * planeDistance < bestDistance) {
        searchNearestNeighbor(furtherNode, targetHistogram, bestDistance, bestLabel, depth + 1);
    }
}

// Función para clasificar una imagen usando el k-d tree
std::string kdTreeClassify(KDTreeNode* root, const cv::Mat& inputHistogram) {
    if (root == nullptr) {
        return "unknown";
    }

    double bestDistance = std::numeric_limits<double>::max();
    std::string bestLabel = "unknown";

    searchNearestNeighbor(root, inputHistogram, bestDistance, bestLabel, 0);

    return bestLabel;
}

// Función para cargar y clasificar imágenes de prueba
void testAndEvaluate(KDTreeNode* kdTreeRoot, const std::string& testFolderPath, int numTestImages) {
    int truePositives = 0;
    int falsePositives = 0;
    int trueNegatives = 0;
    int falseNegatives = 0;

    for (int i = 1; i <= numTestImages; ++i) {
        std::string testImagePath = testFolderPath + "/test_" + std::to_string(i) + ".png";
        std::string trueLabel = "unknown"; // Etiqueta inicializada como "unknown"

        // Modificar el código para obtener la etiqueta verdadera desde el archivo XML
        std::string xmlPath = testFolderPath + "/test_" + std::to_string(i) + ".xml";
        trueLabel = getLabelFromXML(xmlPath);

        cv::Mat testImage = cv::imread(testImagePath, cv::IMREAD_GRAYSCALE);

        if (testImage.empty()) {
            std::cerr << "Error: No se pudo cargar la imagen de prueba " << testImagePath << std::endl;
            continue;
        }

        cv::Mat testHistogram = generateGrayscaleHistogram(testImage);

        if (testHistogram.empty()) {
            std::cerr << "Error: No se pudo generar el histograma para la imagen de prueba " << testImagePath << std::endl;
            continue;
        }

        // Clasificar la imagen de prueba
        std::string predictedLabel = kdTreeClassify(kdTreeRoot, testHistogram);

        std::cout << "Imagen de prueba " << i << ": Etiqueta verdadera = " << trueLabel << ", Etiqueta predicha = " << predictedLabel << std::endl;

        if (predictedLabel == trueLabel) {
            if (predictedLabel == "positive") {
                truePositives++;
                //std::cout << "Resultado: Verdadero Positivo (TP)" << std::endl;
            }
            else {
                trueNegatives++;
                //std::cout << "Resultado: Verdadero Negativo (TN)" << std::endl;
            }
        }
        else {
            if (predictedLabel == "positive") {
                falsePositives++;
                //std::cout << "Resultado: Falso Positivo (FP)" << std::endl;
            }
            else {
                falseNegatives++;
                //std::cout << "Resultado: Falso Negativo (FN)" << std::endl;
            }
        }
    }

    // Calcular métricas
    double accuracy = static_cast<double>(truePositives + trueNegatives) / numTestImages;
    double precision = static_cast<double>(truePositives) / (truePositives + falsePositives);
    double recall = static_cast<double>(truePositives) / (truePositives + falseNegatives);
    double f1Score = 2.0 * (precision * recall) / (precision + recall);

    std::cout << "Accuracy: " << accuracy * 100.0 << "%" << std::endl;
    std::cout << "Precision: " << precision * 100.0 << "%" << std::endl;
    std::cout << "Recall: " << recall * 100.0 << "%" << std::endl;
    std::cout << "F1-Score: " << f1Score << std::endl;
}

int main() {
    std::cout << "Iniciando..." << std::endl;

    // Ruta de la carpeta de entrenamiento
    std::string trainingFolderPath = "road_signs";

    // Número total de imágenes de entrenamiento
    int numTrainingImages = 700;

    // Generar los datos de entrenamiento
    std::vector<ImageDataPoint> trainingData = generateTrainingData(trainingFolderPath, numTrainingImages);
    std::cout << "Cantidad en trainingData: " << trainingData.size() << std::endl;

    // Construir el árbol k-d a partir de los datos de entrenamiento
    KDTreeNode* kdTreeRoot = buildKDTree(trainingData, 0);

    // Ruta de la carpeta de imágenes de prueba
    std::string testFolderPath = "test_images";

    // Número total de imágenes de prueba
    int numTestImages = 198;  // Cambia esto al número de imágenes de prueba que tengas

    // Evaluar el conjunto de imágenes de prueba
    testAndEvaluate(kdTreeRoot, testFolderPath, numTestImages);

    // Pedir al usuario que ingrese el nombre de la imagen a comparar
    std::string inputImagePath;
    std::cout << "Ingrese el nombre de la imagen a comparar (por ejemplo, '01.png'): ";
    std::cin >> inputImagePath;
    inputImagePath = "image/" + inputImagePath; // Concatenar el nombre de la imagen al final de la cadena

    // Cargar la imagen de entrada
    cv::Mat inputImage = cv::imread(inputImagePath, cv::IMREAD_GRAYSCALE);

    if (inputImage.empty()) {
        std::cerr << "Error: No se pudo cargar la imagen." << std::endl;
        return 1; // Salir del programa si no se pudo cargar la imagen
    }

    // Generar el histograma en escala de grises para la imagen de entrada
    cv::Mat inputHistogram = generateGrayscaleHistogram(inputImage);

    if (inputHistogram.empty()) {
        std::cerr << "Error: No se pudo generar el histograma para la imagen." << std::endl;
        return 1; // Salir del programa si no se pudo generar el histograma
    }

    // Clasificar la imagen de entrada
    std::string predictedLabel = kdTreeClassify(kdTreeRoot, inputHistogram);

    std::cout << "La imagen pertenece a la etiqueta: " << predictedLabel << std::endl;

    // Mostrar la imagen en escala de grises
    cv::imshow("Imagen en Escala de Grises", inputImage);
    cv::waitKey(0); // Esperar hasta que se presione una tecla

    // Liberar la memoria del árbol k-d
    delete kdTreeRoot;

    return 0;
}
