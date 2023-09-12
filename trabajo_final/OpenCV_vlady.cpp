#include <iostream>
#include <cstdlib>  // Para utilizar la función system
#include <string>
#include <vector>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include <random>
#include "tinyxml2.h"
#include <omp.h> // Para paralelización
#include <iomanip>

int opcion_descriptor = 0;

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

// Función para generar descriptores SIFT de una imagen con dimensiones consistentes
cv::Mat generateSIFTDescriptor(const cv::Mat& image, int desiredDimension) {
    // Verificar si se cargó la imagen correctamente
    if (image.empty()) {
        std::cerr << "Error: La imagen está vacía." << std::endl;
        return cv::Mat(); // Devolver una matriz vacía en caso de error
    }

    // Inicializar el detector SIFT
    cv::Ptr<cv::SIFT> sift = cv::SIFT::create();

    // Detectar y calcular descriptores SIFT
    std::vector<cv::KeyPoint> keypoints;
    cv::Mat descriptor;
    sift->detectAndCompute(image, cv::noArray(), keypoints, descriptor);

    // Ajustar la dimensión del descriptor a la deseada
    if (descriptor.rows != desiredDimension) {
        if (descriptor.rows < desiredDimension) {
            // Rellenar el descriptor si es más pequeño que la dimensión deseada
            cv::Mat paddedDescriptor(desiredDimension, descriptor.cols, descriptor.type(), cv::Scalar(0));
            descriptor.copyTo(paddedDescriptor(cv::Rect(0, 0, descriptor.cols, descriptor.rows)));
            descriptor = paddedDescriptor;
        }
        else {
            // Truncar el descriptor si es más grande que la dimensión deseada
            descriptor = descriptor.rowRange(0, desiredDimension);
        }
    }

    return descriptor; // Devolver el descriptor SIFT con la dimensión deseada
}

// Función para generar descriptores ORB de una imagen con dimensiones consistentes
cv::Mat generateORBDescriptor(const cv::Mat& image, int desiredDimension) {
    // Verificar si se cargó la imagen correctamente
    if (image.empty()) {
        std::cerr << "Error: La imagen está vacía." << std::endl;
        return cv::Mat(); // Devolver una matriz vacía en caso de error
    }

    // Inicializar el detector ORB
    cv::Ptr<cv::ORB> orb = cv::ORB::create();

    // Detectar y calcular descriptores ORB
    std::vector<cv::KeyPoint> keypoints;
    cv::Mat descriptor;
    orb->detectAndCompute(image, cv::noArray(), keypoints, descriptor);

    // Ajustar la dimensión del descriptor a la deseada
    if (descriptor.rows != desiredDimension) {
        if (descriptor.rows < desiredDimension) {
            // Rellenar el descriptor si es más pequeño que la dimensión deseada
            cv::Mat paddedDescriptor(desiredDimension, descriptor.cols, descriptor.type(), cv::Scalar(0));
            descriptor.copyTo(paddedDescriptor(cv::Rect(0, 0, descriptor.cols, descriptor.rows)));
            descriptor = paddedDescriptor;
        }
        else {
            // Truncar el descriptor si es más grande que la dimensión deseada
            descriptor = descriptor.rowRange(0, desiredDimension);
        }
    }

    return descriptor; // Devolver el descriptor ORB con la dimensión deseada
}

cv::Mat generateCannyDescriptor(const cv::Mat& image, int desiredDimension) {
    // Verificar si se cargó la imagen correctamente
    if (image.empty()) {
        std::cerr << "Error: La imagen está vacía." << std::endl;
        return cv::Mat(); // Devolver una matriz vacía en caso de error
    }

    // Obtener el gradiente de la imagen
    cv::Mat gradient;
    cv::GaussianBlur(image, image, cv::Size(3, 3), 0, 0, cv::BORDER_DEFAULT);
    cv::Sobel(image, gradient, -1, 1, 0, 3, 1, 0);

    // Aplicar la operación de Canny
    cv::Mat edges;
    cv::Canny(gradient, edges, 100, 200, 3);

    // Ajustar la dimensión del descriptor a la deseada
    if (edges.rows != desiredDimension) {
        if (edges.rows < desiredDimension) {
            // Rellenar el descriptor si es más pequeño que la dimensión deseada
            cv::Mat paddedDescriptor(desiredDimension, edges.cols, edges.type(), cv::Scalar(0));
            edges.copyTo(paddedDescriptor(cv::Rect(0, 0, edges.cols, edges.rows)));
            edges = paddedDescriptor;
        }
        else {
            // Truncar el descriptor si es más grande que la dimensión deseada
            edges = edges.rowRange(0, desiredDimension);
        }
    }

    cv::Size desiredSize(1, desiredDimension); // Tamaño deseado para los descriptores Canny
    cv::resize(edges, edges, desiredSize);

    // Transformar la imagen binaria en un vector
    cv::Mat descriptorVector = edges.reshape(1, edges.total());



    return descriptorVector; // Devolver el descriptor Canny con la dimensión deseada
}

//************************************************************

// Función para generar datos de entrenamiento a partir de imágenes y archivos XML
std::vector<ImageDataPoint> generateTrainingData(const std::string& folderPath, int numImages, int desiredDimension) {
    std::vector<ImageDataPoint> trainingData;
    trainingData.reserve(numImages); // Reservar espacio para evitar reallocaciones

#pragma omp parallel for
    for (int i = 0; i < numImages; ++i) {
        std::string imagePath = folderPath + "/images/road" + std::to_string(i) + ".png";
        std::string xmlPath = folderPath + "/annotations/road" + std::to_string(i) + ".xml";

        cv::Mat image = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
        cv::Mat descriptor;

        switch(opcion_descriptor)
        {
        case 1: descriptor = generateSIFTDescriptor(image, desiredDimension);
            break;
        case 2: descriptor = generateCannyDescriptor(image, desiredDimension);
            break;
        case 3: descriptor = generateORBDescriptor(image, desiredDimension);
            break;
        default:
            
            break;
        }

        // Verificar si se generó el descriptor y la etiqueta no es "unknown"
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

// Función para construir un árbol k-d a partir de los datos de entrenamiento
KDTreeNode* buildKDTree(std::vector<ImageDataPoint>& dataset, int depth) {
    if (dataset.empty()) {
        return nullptr;
    }

    int dimensions = dataset[0].descriptor.rows;

    // Elegir la dimensión actual como la que tiene la mayor varianza
    int axis = depth % dimensions;

    // Ordenar el dataset en función de la dimensión actual
    std::sort(dataset.begin(), dataset.end(), [axis](const ImageDataPoint& a, const ImageDataPoint& b) {
        return a.descriptor.at<float>(axis) < b.descriptor.at<float>(axis);
        });

    // Encontrar el valor mediano
    int medianIndex = dataset.size() / 2;

    // Crear el nodo actual y construir recursivamente los subárboles
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

// Función para calcular la distancia entre dos descriptores utilizando el k-d tree
double calculateDistance(const cv::Mat& descriptor1, const cv::Mat& descriptor2) {
    if (descriptor1.empty() || descriptor2.empty()) {
        std::cerr << "Error: Uno o ambos descriptores están vacíos." << std::endl;
        return std::numeric_limits<double>::max();
    }

    if (descriptor1.size() != descriptor2.size()) {
        std::cerr << "Error: Las dimensiones de los descriptores no son compatibles." << std::endl;
        return std::numeric_limits<double>::max();
    }

    return cv::norm(descriptor2, descriptor1, cv::NORM_L2);
}

// Función para realizar una búsqueda en el k-d tree y encontrar el vecino más cercano
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

    // Calcular la distancia mínima entre el plano de corte y el objetivo
    double planeDistance = targetDescriptor.at<float>(axis) - currentNode->descriptor.at<float>(axis);

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

    // Poda del árbol si la distancia en el eje actual es mayor que la mejor distancia actual
    if (planeDistance * planeDistance < bestDistance) {
        searchNearestNeighbor(furtherNode, targetDescriptor, bestDistance, bestLabel, depth + 1);
    }
}

// Función para clasificar una imagen usando el k-d tree
std::string kdTreeClassify(KDTreeNode* root, const cv::Mat& inputDescriptor) {
    if (root == nullptr) {
        return "unknown";
    }

    double bestDistance = std::numeric_limits<double>::max();
    std::string bestLabel = "unknown";

    searchNearestNeighbor(root, inputDescriptor, bestDistance, bestLabel, 0);

    return bestLabel;
}

int GenerarValorAleatorio(int inicio, int final) {
    std::random_device rd;  // Generador de números aleatorios
    std::mt19937 gen(rd()); // Semilla para el generador

    // Distribución uniforme entre 'inicio' y 'final'
    std::uniform_int_distribution<int> distribucion(inicio, final);

    // Genera y devuelve un valor aleatorio dentro del rango
    return distribucion(gen);
}

// Experimento considerando las imagenes de prueba como parte del entrenamiento
void experimento_00() {

    switch (opcion_descriptor)
    {
    case 1: std::cout << "SIFT" << std::endl;
        break;
    case 2: std::cout << "Canny" << std::endl;
        break;
    case 3: std::cout << "ORB" << std::endl;
        break;
    default:

        break;
    }

    std::cout << "Iniciando..." << std::endl;

    // Ruta de la carpeta de entrenamiento
    std::string trainingFolderPath = "road_signs";

    // Número total de imágenes de entrenamiento
    int numTrainingImages = 700;

    // Dimensión deseada para los descriptores SIFT
    int desiredDimension = 256; // Cambia esto a la dimensión deseada

    // Generar los datos de entrenamiento con la dimensión deseada
    std::vector<ImageDataPoint> trainingData = generateTrainingData(trainingFolderPath, numTrainingImages, desiredDimension);
    std::cout << "Cantidad en trainingData: " << trainingData.size() << std::endl;

    // Construir el árbol k-d a partir de los datos de entrenamiento
    KDTreeNode* kdTreeRoot = buildKDTree(trainingData, 0);

    // Ruta de la carpeta de imágenes de prueba
    //std::string testFolderPath = "test_images";

    // Número total de imágenes de prueba
    int numTestImages = 20;  // Cambia esto al número de imágenes de prueba que tengas

    // Evaluar el conjunto de imágenes de prueba
    //testAndEvaluate(kdTreeRoot, testFolderPath, numTestImages, desiredDimension);

    // Pedir al usuario que ingrese el nombre de la imagen a comparar
    //std::string inputImagePath;
    //std::cout << "Ingrese el nombre de la imagen a comparar (por ejemplo, '01.png'): ";
    //std::cin >> inputImagePath;
    //inputImagePath = "image/02.png"; //+ inputImagePath; // Concatenar el nombre de la imagen al final de la cadena

    int aciertos = 0;
    int desaciertos = 0;

    for (int i = 0; i < numTestImages; ++i) {

        int num_random = GenerarValorAleatorio(1, 700);

        std::string imagePath = trainingFolderPath + "/images/road" + std::to_string(num_random) + ".png";
        std::string xmlPath = trainingFolderPath + "/annotations/road" + std::to_string(num_random) + ".xml";

        cv::Mat image = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
        //cv::Mat descriptor = generateORBDescriptor(image, desiredDimension);

        // Verificar si se generó el descriptor y la etiqueta no es "unknown"
        std::string label = getLabelFromXML(xmlPath);

        if (image.empty()) {
            std::cerr << "Error: No se pudo cargar la imagen." << std::endl;
            return; // Salir del programa si no se pudo cargar la imagen
        }

        // Generar el descriptor para la imagen de entrada con la dimensión deseada
        cv::Mat inputDescriptor;

        switch (opcion_descriptor)
        {
        case 1: inputDescriptor = generateSIFTDescriptor(image, desiredDimension);
            break;
        case 2: inputDescriptor = generateCannyDescriptor(image, desiredDimension);
            break;
        case 3: inputDescriptor = generateORBDescriptor(image, desiredDimension);
            break;
        default:

            break;
        }

        if (inputDescriptor.empty()) {
            std::cerr << "Error: No se pudo generar el descriptor para la imagen." << std::endl;
            return; // Salir del programa si no se pudo generar el descriptor
        }

        // Clasificar la imagen de entrada
        std::string predictedLabel = kdTreeClassify(kdTreeRoot, inputDescriptor);

        if (label == predictedLabel) {
            std::cout << "Predicción correcta: " << predictedLabel << std::endl;
            aciertos += 1;
        }
        else {
            std::cout << "Predicción incorrecta: Original: " << label << " Predicción: " << predictedLabel << std::endl;
            desaciertos += 1;
        }

    }

    std::cout << "Porcentaje de aciertos: " << (aciertos * 100) / numTestImages << " Porcentaje de desaciertos: " << (desaciertos * 100) / numTestImages << std::endl;

    int Precision = aciertos / (aciertos + desaciertos);
    int Recall = 1;

    std::cout << "Indicador F1 Score: (2 * precisión)/(precisión + recall) = " << (2 * Precision * Recall) / (Precision + Recall) << " Indicador Accuracy = " << (aciertos + desaciertos) / numTestImages << std::endl;

    // Cargar la imagen de entrada

    // Mostrar la imagen en escala de grises
    //cv::imshow("Imagen en Escala de Grises", inputImage);
    //cv::waitKey(0); // Esperar hasta que se presione una tecla

    // Liberar la memoria del árbol k-d
    delete kdTreeRoot;
}

// Experimento excluyendo las imagenes de prueba como parte del entrenamiento

void experimento_01() {

    switch (opcion_descriptor)
    {
            case 1: std::cout << "SIFT" << std::endl;
                break;
            case 2: std::cout << "Canny" << std::endl;
                break;
            case 3: std::cout << "ORB" << std::endl;
                break;
    default:
        break;
    }

    std::cout << "Iniciando..." << std::endl;

    // Ruta de la carpeta de entrenamiento
    std::string trainingFolderPath = "road_signs";

    // Número total de imágenes de entrenamiento
    int numTrainingImages = 700;

    // Dimensión deseada para los descriptores SIFT
    int desiredDimension = 256; // Cambia esto a la dimensión deseada

    // Generar los datos de entrenamiento con la dimensión deseada
    std::vector<ImageDataPoint> trainingData = generateTrainingData(trainingFolderPath, numTrainingImages, desiredDimension);
    std::cout << "Cantidad en trainingData: " << trainingData.size() << std::endl;

    // Construir el árbol k-d a partir de los datos de entrenamiento
    KDTreeNode* kdTreeRoot = buildKDTree(trainingData, 0);

    // Número total de imágenes de prueba
    int numTestImages = 117;  // Cambia esto al número de imágenes de prueba que tengas

    // Evaluar el conjunto de imágenes de prueba
    //testAndEvaluate(kdTreeRoot, testFolderPath, numTestImages, desiredDimension);

    // Pedir al usuario que ingrese el nombre de la imagen a comparar
    //std::string inputImagePath;
    //std::cout << "Ingrese el nombre de la imagen a comparar (por ejemplo, '01.png'): ";
    //std::cin >> inputImagePath;
    //inputImagePath = "image/02.png"; //+ inputImagePath; // Concatenar el nombre de la imagen al final de la cadena

    int aciertos = 0;
    int desaciertos = 0;

    trainingFolderPath = "test_images";

    for (int i = 0; i < numTestImages; ++i) {

        int num_random = GenerarValorAleatorio(1, 117);

        std::string imagePath = trainingFolderPath + "/images/test_" + std::to_string(num_random) + ".png";
        std::string xmlPath = trainingFolderPath + "/annotations/test_" + std::to_string(num_random) + ".xml";

        cv::Mat image = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
        
        // Verificar si se generó el descriptor y la etiqueta no es "unknown"
        std::string label = getLabelFromXML(xmlPath);

        if (image.empty()) {
            std::cerr << "Error: No se pudo cargar la imagen." << std::endl;
            return; // Salir del programa si no se pudo cargar la imagen
        }

        // Generar el descriptor para la imagen de entrada con la dimensión deseada
        cv::Mat inputDescriptor;

        switch (opcion_descriptor)
        {
        case 1: inputDescriptor = generateSIFTDescriptor(image, desiredDimension);
            break;
        case 2: inputDescriptor = generateCannyDescriptor(image, desiredDimension);
            break;
        case 3: inputDescriptor = generateORBDescriptor(image, desiredDimension);
            break;
        default:

            break;
        }

        if (inputDescriptor.empty()) {
            std::cerr << "Error: No se pudo generar el descriptor para la imagen." << std::endl;
            return; // Salir del programa si no se pudo generar el descriptor
        }

        // Clasificar la imagen de entrada
        std::string predictedLabel = kdTreeClassify(kdTreeRoot, inputDescriptor);

        if (label == predictedLabel) {
            std::cout << "Predicción correcta: " << predictedLabel << std::endl;
            aciertos += 1;
            //cv::imshow("Imagen en Escala de Grises", image);
            //cv::waitKey(0); // Esperar hasta que se presione una tecla
        }
        else {
            std::cout << "Predicción incorrecta: Original: " << label << " Predicción: " << predictedLabel << std::endl;
            desaciertos += 1;
        }

    }

    std::cout << "Porcentaje de aciertos: " << std::fixed << std::setprecision(2) << (aciertos * 100)/numTestImages << " Porcentaje de desaciertos: " << std::fixed << std::setprecision(2) << (desaciertos * 100)/numTestImages << std::endl;

    float Precision = aciertos/(aciertos + desaciertos);
    float Recall = 1;

    std::cout << "Indicador F1 Score: (2 * precisión)/(precisión + recall) = " << (2 * Precision * Recall) / (Precision + Recall) <<  " || Indicador Accuracy = "  << (aciertos+desaciertos)/numTestImages << std::endl;

    // Cargar la imagen de entrada

    // Mostrar la imagen en escala de grises
    //cv::imshow("Imagen en Escala de Grises", inputImage);
    //cv::waitKey(0); // Esperar hasta que se presione una tecla

    // Liberar la memoria del árbol k-d
    delete kdTreeRoot;
}

int main() {

    int control;
    std::string opcion;
 
    while (true) {
        // Mostrar el menú
        std::cout << "Seleccione unos de los descriptores de la lista:" << std::endl;
        std::cout << "1. SIFT" << std::endl;
        std::cout << "2. Canny" << std::endl;
        std::cout << "3. ORB" << std::endl;      
        std::cout << "4. Salir" << std::endl;
        std::cout << "Seleccione una opción: ";

        // Leer la opción seleccionada
        std::cin >> control;

        // Realizar acciones en función de la opción seleccionada
        switch (control) {
        case 1:
            std::cout << "Descriptor SIFT" << std::endl;
            opcion = "SIFT";
            opcion_descriptor = 1;
            break;
        case 2:
            std::cout << "Descriptor Canny" << std::endl;
            opcion = "Canny";
            opcion_descriptor = 2;
            break;
        case 3:
            std::cout << "Descriptor ORB" << std::endl;
            opcion = "ORB";
            opcion_descriptor = 3;
            break;
        case 4:
            std::cout << "Saliendo del programa." << std::endl;
            return 0;
        default:
            std::cout << "Opción no válida. Por favor, elige una opción válida." << std::endl;
            break;
        }
        system("cls");
        if (opcion_descriptor > 0) {
            experimento_01();
            return 0;
        }       
    }   
    return 0;
}
