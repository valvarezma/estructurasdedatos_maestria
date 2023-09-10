#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include "tinyxml2.h"
#include <omp.h> // Para paralelizaci�n

struct ImageDataPoint {
    cv::Mat descriptor; // Descriptor de la imagen
    std::string label;  // Etiqueta de la imagen
};

// Estructura para un nodo del k-d tree
struct KDTreeNode {
    cv::Mat descriptor;
    std::string label;
    KDTreeNode* left;
    KDTreeNode* right;

    KDTreeNode(const cv::Mat& desc, const std::string& lbl) : descriptor(desc), label(lbl), left(nullptr), right(nullptr) {}
};

// Funci�n para obtener la etiqueta de una imagen desde un archivo XML
std::string getLabelFromXML(const std::string& xmlPath) {
    tinyxml2::XMLDocument doc;

    // Cargar el archivo XML
    if (doc.LoadFile(xmlPath.c_str()) != tinyxml2::XML_SUCCESS) {
        std::cerr << "Error al cargar el archivo XML: " << xmlPath << std::endl;
        return "unknown"; // Valor de etiqueta predeterminado en caso de error
    }

    tinyxml2::XMLElement* root = doc.FirstChildElement("annotation");

    // Verificar si se encontr� el elemento 'annotation'
    if (!root) {
        std::cerr << "Elemento 'annotation' no encontrado en el archivo XML: " << xmlPath << std::endl;
        return "unknown"; // Valor de etiqueta predeterminado en caso de error
    }

    tinyxml2::XMLElement* object = root->FirstChildElement("object");

    // Verificar si se encontr� el elemento 'object'
    if (!object) {
        std::cerr << "Elemento 'object' no encontrado en el archivo XML: " << xmlPath << std::endl;
        return "unknown"; // Valor de etiqueta predeterminado en caso de error
    }

    tinyxml2::XMLElement* name = object->FirstChildElement("name");

    // Verificar si se encontr� el elemento 'name'
    if (!name) {
        std::cerr << "Elemento 'name' no encontrado en el archivo XML: " << xmlPath << std::endl;
        return "unknown"; // Valor de etiqueta predeterminado en caso de error
    }

    const char* labelStr = name->GetText();

    // Verificar si se encontr� el valor de etiqueta
    if (!labelStr) {
        std::cerr << "Valor de etiqueta no encontrado en el archivo XML: " << xmlPath << std::endl;
        return "unknown"; // Valor de etiqueta predeterminado en caso de error
    }

    return labelStr;
}

// Funci�n para generar descriptores Canny de una imagen con dimensiones consistentes
cv::Mat generateCannyDescriptor(const cv::Mat& image, int desiredDimension) {
    // Verificar si se carg� la imagen correctamente
    if (image.empty()) {
        std::cerr << "Error: La imagen est� vac�a." << std::endl;
        return cv::Mat(); // Devolver una matriz vac�a en caso de error
    }

    cv::Mat edges;
    cv::Canny(image, edges, 100, 200); // Aplicar el detector de bordes Canny

    // Asegurarse de que el descriptor tenga un tama�o fijo de la dimensi�n deseada
    cv::Size desiredSize(1, desiredDimension); // Tama�o deseado para los descriptores Canny
    cv::resize(edges, edges, desiredSize);

    return edges; // Devolver el descriptor Canny con la dimensi�n deseada
}

// Funci�n para generar datos de entrenamiento a partir de im�genes y archivos XML
std::vector<ImageDataPoint> generateTrainingData(const std::string& folderPath, int numImages, int desiredDimension) {
    std::vector<ImageDataPoint> trainingData;
    trainingData.reserve(numImages); // Reservar espacio para evitar reallocaciones

#pragma omp parallel for
    for (int i = 0; i < numImages; ++i) {
        std::string imagePath = folderPath + "/images/road" + std::to_string(i) + ".png";
        std::string xmlPath = folderPath + "/annotations/road" + std::to_string(i) + ".xml";

        cv::Mat image = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
        cv::Mat descriptor = generateCannyDescriptor(image, desiredDimension);

        // Verificar si se gener� el descriptor y la etiqueta no es "unknown"
        if (!descriptor.empty()) {
            std::string label = getLabelFromXML(xmlPath);
            if (label != "unknown") {
                ImageDataPoint dataPoint;
                dataPoint.descriptor = descriptor;
                dataPoint.label = label;
#pragma omp critical
                trainingData.push_back(dataPoint);
            }
        }
    }

    return trainingData;
}

// Funci�n para construir un �rbol k-d a partir de los datos de entrenamiento
KDTreeNode* buildKDTree(std::vector<ImageDataPoint>& dataset, int depth) {
    if (dataset.empty()) {
        return nullptr;
    }

    int dimensions = dataset[0].descriptor.rows;

    // Elegir la dimensi�n actual como la que tiene la mayor varianza
    int axis = depth % dimensions;

    // Ordenar el dataset en funci�n de la dimensi�n actual
    std::sort(dataset.begin(), dataset.end(), [axis](const ImageDataPoint& a, const ImageDataPoint& b) {
        return a.descriptor.at<uchar>(axis) < b.descriptor.at<uchar>(axis);
        });

    // Encontrar el valor mediano
    int medianIndex = dataset.size() / 2;

    // Crear el nodo actual y construir recursivamente los sub�rboles
    KDTreeNode* node = new KDTreeNode(
        dataset[medianIndex].descriptor,
        dataset[medianIndex].label
    );

    std::vector<ImageDataPoint> leftDataset(dataset.begin(), dataset.begin() + medianIndex);
    std::vector<ImageDataPoint> rightDataset(dataset.begin() + medianIndex + 1, dataset.end());

    node->left = buildKDTree(leftDataset, depth + 1);
    node->right = buildKDTree(rightDataset, depth + 1);

    return node;
}

// Funci�n para calcular la distancia entre dos descriptores utilizando el k-d tree
double calculateDistance(const cv::Mat& descriptor1, const cv::Mat& descriptor2) {
    if (descriptor1.empty() || descriptor2.empty()) {
        std::cerr << "Error: Uno o ambos descriptores est�n vac�os." << std::endl;
        return std::numeric_limits<double>::max();
    }

    if (descriptor1.size() != descriptor2.size()) {
        std::cerr << "Error: Las dimensiones de los descriptores no son compatibles." << std::endl;
        return std::numeric_limits<double>::max();
    }

    double distance = 0.0;

    // Calcular la distancia Euclidiana entre los descriptores Canny
    for (int i = 0; i < descriptor1.rows; ++i) {
        double diff = static_cast<double>(descriptor1.at<uchar>(i)) - static_cast<double>(descriptor2.at<uchar>(i));
        distance += diff * diff;
    }

    return std::sqrt(distance);
}

// Funci�n para realizar una b�squeda en el k-d tree y encontrar el vecino m�s cercano
void searchNearestNeighbor(
    KDTreeNode* currentNode,
    const cv::Mat& targetDescriptor,
    double& bestDistance,
    std::string& bestLabel,
    int depth
) {
    if (currentNode == nullptr) {
        return;
    }

    int dimensions = targetDescriptor.rows;
    int axis = depth % dimensions;

    double currentDistance = calculateDistance(targetDescriptor, currentNode->descriptor);

    if (currentDistance < bestDistance) {
        bestDistance = currentDistance;
        bestLabel = currentNode->label;
    }

    // Calcular la distancia m�nima entre el plano de corte y el objetivo
    double planeDistance = static_cast<double>(targetDescriptor.at<uchar>(axis)) - static_cast<double>(currentNode->descriptor.at<uchar>(axis));

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

    searchNearestNeighbor(nearerNode, targetDescriptor, bestDistance, bestLabel, depth + 1);

    // Poda del �rbol si la distancia en el eje actual es mayor que la mejor distancia actual
    if (planeDistance * planeDistance < bestDistance) {
        searchNearestNeighbor(furtherNode, targetDescriptor, bestDistance, bestLabel, depth + 1);
    }
}

// Funci�n para clasificar una imagen usando el k-d tree
std::string kdTreeClassify(KDTreeNode* root, const cv::Mat& inputDescriptor) {
    if (root == nullptr) {
        return "unknown";
    }

    double bestDistance = std::numeric_limits<double>::max();
    std::string bestLabel = "unknown";

    searchNearestNeighbor(root, inputDescriptor, bestDistance, bestLabel, 0);

    return bestLabel;
}

// Funci�n para cargar y clasificar im�genes de prueba
void testAndEvaluate(KDTreeNode* kdTreeRoot, const std::string& testFolderPath, int numTestImages, int desiredDimension) {
    int truePositives = 0;
    int falsePositives = 0;
    int trueNegatives = 0;
    int falseNegatives = 0;

    for (int i = 1; i <= numTestImages; ++i) {
        std::string testImagePath = testFolderPath + "/test_" + std::to_string(i) + ".png";
        std::string trueLabel = "unknown"; // Etiqueta inicializada como "unknown"

        // Modificar el c�digo para obtener la etiqueta verdadera desde el archivo XML
        std::string xmlPath = testFolderPath + "/test_" + std::to_string(i) + ".xml";
        trueLabel = getLabelFromXML(xmlPath);

        cv::Mat testImage = cv::imread(testImagePath, cv::IMREAD_GRAYSCALE);

        if (testImage.empty()) {
            std::cerr << "Error: No se pudo cargar la imagen de prueba " << testImagePath << std::endl;
            continue;
        }

        cv::Mat testDescriptor = generateCannyDescriptor(testImage, desiredDimension);

        if (testDescriptor.empty()) {
            std::cerr << "Error: No se pudo generar el descriptor para la imagen de prueba " << testImagePath << std::endl;
            continue;
        }

        // Clasificar la imagen de prueba
        std::string predictedLabel = kdTreeClassify(kdTreeRoot, testDescriptor);

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

    // Calcular m�tricas
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

    // N�mero total de im�genes de entrenamiento
    int numTrainingImages = 700;

    // Dimensi�n deseada para los descriptores Canny
    int desiredDimension = 256; // Cambia esto a la dimensi�n deseada

    // Generar los datos de entrenamiento con la dimensi�n deseada
    std::vector<ImageDataPoint> trainingData = generateTrainingData(trainingFolderPath, numTrainingImages, desiredDimension);
    std::cout << "Cantidad en trainingData: " << trainingData.size() << std::endl;

    // Construir el �rbol k-d a partir de los datos de entrenamiento
    KDTreeNode* kdTreeRoot = buildKDTree(trainingData, 0);

    // Ruta de la carpeta de im�genes de prueba
    std::string testFolderPath = "test_images";

    // N�mero total de im�genes de prueba
    int numTestImages = 198;  // Cambia esto al n�mero de im�genes de prueba que tengas

    // Evaluar el conjunto de im�genes de prueba
    testAndEvaluate(kdTreeRoot, testFolderPath, numTestImages, desiredDimension);

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

    // Generar el descriptor Canny para la imagen de entrada con la dimensi�n deseada
    cv::Mat inputDescriptor = generateCannyDescriptor(inputImage, desiredDimension);

    if (inputDescriptor.empty()) {
        std::cerr << "Error: No se pudo generar el descriptor para la imagen." << std::endl;
        return 1; // Salir del programa si no se pudo generar el descriptor
    }

    // Clasificar la imagen de entrada
    std::string predictedLabel = kdTreeClassify(kdTreeRoot, inputDescriptor);

    std::cout << "La imagen pertenece a la etiqueta: " << predictedLabel << std::endl;

    // Mostrar la imagen en escala de grises
    cv::imshow("Imagen en Escala de Grises", inputImage);
    cv::waitKey(0); // Esperar hasta que se presione una tecla

    // Liberar la memoria del �rbol k-d
    delete kdTreeRoot;

    return 0;
}
