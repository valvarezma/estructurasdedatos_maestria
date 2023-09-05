#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <map>
#include <opencv2/opencv.hpp>
#include "tinyxml2.h"
using namespace cv;

struct ImageDataPoint {
    cv::Mat descriptor; // Descriptor (SIFT o Canny)
    std::string label;
};

// Función para calcular la distancia euclidiana entre dos descriptores SIFT
double calculateDistance(const cv::Mat& descriptor1, const cv::Mat& descriptor2) {
    try {
        // Verifica si las matrices tienen el mismo tamaño antes de llamar a cv::norm
        if (descriptor1.size() != descriptor2.size()) {
            // Maneja el error o lanza una excepción según tus necesidades
            throw std::runtime_error("Las matrices no tienen el mismo tamaño");
        }
        return cv::norm(descriptor1, descriptor2, cv::NORM_L2);
    }
    catch (cv::Exception& e) {
        // Maneja la excepción de OpenCV aquí
        std::cerr << "Excepción de OpenCV: " << e.what() << std::endl;
        // Puedes tomar medidas adecuadas, como devolver un valor predeterminado o lanzar una excepción personalizada si es necesario.
        throw; // Propaga la excepción nuevamente si es necesario.
    }
}

// Función para obtener la etiqueta de una imagen desde un archivo XML
std::string getLabelFromXML(const std::string& xmlPath) {
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(xmlPath.c_str()) != tinyxml2::XML_SUCCESS) {
        std::cerr << "Error al cargar el archivo XML: " << xmlPath << std::endl;
        return "unknown"; // Valor de etiqueta predeterminado en caso de error
    }

    tinyxml2::XMLElement* root = doc.FirstChildElement("annotation");
    if (!root) {
        std::cerr << "Elemento 'annotation' no encontrado en el archivo XML: " << xmlPath << std::endl;
        return "unknown"; // Valor de etiqueta predeterminado en caso de error
    }

    tinyxml2::XMLElement* object = root->FirstChildElement("object");
    if (!object) {
        std::cerr << "Elemento 'object' no encontrado en el archivo XML: " << xmlPath << std::endl;
        return "unknown"; // Valor de etiqueta predeterminado en caso de error
    }

    tinyxml2::XMLElement* name = object->FirstChildElement("name");
    if (!name) {
        std::cerr << "Elemento 'name' no encontrado en el archivo XML: " << xmlPath << std::endl;
        return "unknown"; // Valor de etiqueta predeterminado en caso de error
    }

    const char* labelStr = name->GetText();
    if (!labelStr) {
        std::cerr << "Valor de etiqueta no encontrado en el archivo XML: " << xmlPath << std::endl;
        return "unknown"; // Valor de etiqueta predeterminado en caso de error
    }

    return labelStr;
}

void showImage(const std::string& imagePath) {
    // Cargar la imagen desde el archivo
    cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR); // Cambia "imagen.jpg" por la ruta de tu imagen

    // Verificar si la imagen se ha cargado correctamente
    if (image.empty()) {
        std::cerr << "No se pudo cargar la imagen." << std::endl;
    }

    // Mostrar la imagen en una ventana
    cv::imshow("Imagen", image);

    // Esperar hasta que se presione una tecla
    cv::waitKey(0);
}

// Función para generar descriptores SIFT de una imagen
cv::Mat generateSIFTDescriptor(const std::string& imagePath) {
    cv::Mat image = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
    if (image.empty()) {
        std::cerr << "Error al cargar la imagen: " << imagePath << std::endl;
        return cv::Mat();
    }

    cv::Ptr<cv::SIFT> sift = cv::SIFT::create();
    std::vector<cv::KeyPoint> keypoints; // Lista vacía de keypoints
    cv::Mat descriptor;

    // Asegurarse de que el descriptor tenga un tamaño fijo
    cv::Size fixedSize(128, 1); // Tamaño fijo para los descriptores SIFT
    sift->compute(image, keypoints, descriptor);

    // Redimensionar el descriptor al tamaño fijo si es necesario
    if (descriptor.size() != fixedSize) {
        cv::resize(descriptor, descriptor, fixedSize);
    }

    return descriptor;
}

// Función para generar descriptores Canny de una imagen
cv::Mat generateCannyDescriptor(const std::string& imagePath) {
    cv::Mat image = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
    if (image.empty()) {
        std::cerr << "Error al cargar la imagen: " << imagePath << std::endl;
        return cv::Mat();
    }

    cv::Mat edges;
    cv::Canny(image, edges, 100, 200); // Ajusta los umbrales según sea necesario

    // Asegurarse de que el descriptor tenga un tamaño fijo
    cv::Size fixedSize(128, 1); // Tamaño fijo para los descriptores Canny (ajusta según sea necesario)
    cv::resize(edges, edges, fixedSize);

    return edges;
}

std::vector<ImageDataPoint> generateTrainingData(const std::string& folderPath, int numImages) {
    std::vector<ImageDataPoint> trainingData;

    for (int i = 0; i < numImages; ++i) {
        std::string imagePath = folderPath + "/images/road" + std::to_string(i) + ".png";
        std::string xmlPath = folderPath + "/annotations/road" + std::to_string(i) + ".xml";

        cv::Mat descriptor = generateCannyDescriptor(imagePath);

        if (!descriptor.empty()) {
            std::string label = getLabelFromXML(xmlPath);
            if (label != "unknown") {
                ImageDataPoint dataPoint;
                dataPoint.descriptor = descriptor;
                dataPoint.label = label;
                trainingData.push_back(dataPoint);
            }
        }
    }

    return trainingData;
}

// Función para clasificar una imagen usando el algoritmo k-NN
std::string kNNClassify(const std::vector<ImageDataPoint>& dataset, const cv::Mat& inputDescriptor, int k) {
    // Calcular distancias entre el descriptor de entrada y los descriptores en el conjunto de datos
    std::vector<std::pair<double, std::string>> distances; // distancia, etiqueta

    for (const ImageDataPoint& dataPoint : dataset) {
        try {
            double distance = calculateDistance(dataPoint.descriptor, inputDescriptor);
            distances.push_back(std::make_pair(distance, dataPoint.label));
        }
        catch (std::exception& e) {
            std::cerr << "Excepción no controlada: " << e.what() << std::endl;
        }
    }

    // Ordenar las distancias de menor a mayor
    std::sort(distances.begin(), distances.end());

    // Contar las etiquetas de los k vecinos más cercanos
    std::map<std::string, int> labelCount;
    for (int i = 0; i < k && i < distances.size(); ++i) {
        labelCount[distances[i].second]++;
    }

    // Encontrar la etiqueta más común entre los k vecinos
    int maxCount = 0;
    std::string bestLabel = "unknown";
    for (const auto& pair : labelCount) {
        if (pair.second > maxCount) {
            maxCount = pair.second;
            bestLabel = pair.first;
        }
    }

    return bestLabel;
}

int main() {
    std::cout << "Iniciando..." << std::endl;
    // Ruta de la carpeta de entrenamiento
    std::string trainingFolderPath = "road_signs";

    // Número total de imágenes de entrenamiento
    int numTrainingImages = 500;

    // Generar los datos de entrenamiento
    std::vector<ImageDataPoint> trainingData = generateTrainingData(trainingFolderPath, numTrainingImages);
    std::cout << "Cantidad en trainingData: " << trainingData.size() << std::endl;

    // Ruta de la imagen de entrada para clasificar
    std::string inputImagePath = "image/road1.png";

    // Generar el descriptor Canny para la imagen de entrada
    cv::Mat inputDescriptor = generateCannyDescriptor(inputImagePath);

    if (inputDescriptor.empty()) {
        return 1;
    }

    // Valor de k para el algoritmo k-NN
    int k = 3;

    // Clasificar la imagen de entrada
    std::string predictedLabel = kNNClassify(trainingData, inputDescriptor, k);

    std::cout << "La imagen pertenece a la etiqueta: " << predictedLabel << std::endl;

    return 0;
}
