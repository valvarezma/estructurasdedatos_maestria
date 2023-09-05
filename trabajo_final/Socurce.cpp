#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <map>
#include <opencv2/opencv.hpp>
#include "tinyxml2.h"
#include <fstream>

using namespace cv;

struct ImageDataPoint {
    cv::Mat descriptor; // SIFT descriptor
    std::string label; // Cambiado a std::string
};

// Función para calcular la distancia euclidiana entre dos descriptores Canny
double calculateDistance(const cv::Mat& descriptor1, const cv::Mat& descriptor2) {
    try {
        if (descriptor1.size() != descriptor2.size()) {
            throw std::runtime_error("Las matrices no tienen el mismo tamaño");
            std::cerr << "Descriptor1: " << descriptor1.size() << "Descriptor2: " << descriptor2.size() << std::endl;
        }
        return cv::norm(descriptor1, descriptor2, cv::NORM_L2);
    }
    catch (cv::Exception& e) {
        std::cerr << "Excepción de OpenCV: " << e.what() << std::endl;
        throw;
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
void showImage(const std::string& imagePath)
{
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
    sift->compute(image, keypoints, descriptor);

    return descriptor;
}






// Función para clasificar una imagen usando el algoritmo k-NN
std::string kNNClassify(const std::vector<ImageDataPoint>& dataset, const cv::Mat& inputDescriptor, int k) {
    std::vector<std::pair<double, std::string>> distances;

    for (const ImageDataPoint& dataPoint : dataset) {
        try {
            double distance = calculateDistance(dataPoint.descriptor, inputDescriptor);
            distances.push_back(std::make_pair(distance, dataPoint.label));
        }
        catch (std::exception& e) {
            std::cerr << "Excepción no controlada: " << e.what() << std::endl;
        }
    }

    std::sort(distances.begin(), distances.end());

    std::map<std::string, int> labelCount;
    for (int i = 0; i < k && i < distances.size(); ++i) {
        labelCount[distances[i].second]++;
    }

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

cv::Mat generateCannyDescriptor(const std::string& imagePath, const cv::Size& targetSize) {
    cv::Mat image = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
    if (image.empty()) {
        std::cerr << "Error al cargar la imagen: " << imagePath << std::endl;
        return cv::Mat();
    }

    // Redimensiona la imagen al tamaño de destino
    cv::resize(image, image, targetSize);

    cv::Mat edges;
    cv::Canny(image, edges, 100, 200);

    return edges;
}

std::vector<ImageDataPoint> generateTrainingData(const std::string& folderPath, int numImages, const cv::Size& targetSize) {
    std::vector<ImageDataPoint> trainingData;

    for (int i = 0; i < numImages; ++i) {
        std::string imagePath = folderPath + "/images/road" + std::to_string(i) + ".png";
        std::string xmlPath = folderPath + "/annotations/road" + std::to_string(i) + ".xml";

        cv::Mat cannyDescriptor = generateCannyDescriptor(imagePath, targetSize);

        if (!cannyDescriptor.empty()) {
            std::string label = getLabelFromXML(xmlPath);
            if (label != "unknown") {
                ImageDataPoint dataPoint;
                dataPoint.descriptor = cannyDescriptor;
                dataPoint.label = label;
                trainingData.push_back(dataPoint);
            }
        }
    }

    return trainingData;
}

int generarNumeroAleatorio(int min, int max) {
    // Inicializar la semilla para la generación de números aleatorios
    srand(static_cast<unsigned int>(time(nullptr)));

    // Generar un número aleatorio entre min y max
    int randomNumber = rand() % (max - min + 1) + min;

    return randomNumber;
}

// Función para exportar los datos del vector a un archivo CSV
void exportToCSV(const std::vector<ImageDataPoint>& data, const std::string& outputPath) {
    std::ofstream outputFile(outputPath);
    if (!outputFile.is_open()) {
        std::cerr << "Error al abrir el archivo CSV para escritura: " << outputPath << std::endl;
        return;
    }

    // Escribir la cabecera del archivo CSV (columnas)
    outputFile << "Label,Descriptor" << std::endl;

    // Escribir los datos de cada punto de datos en el vector
    for (const ImageDataPoint& dataPoint : data) {
        outputFile << dataPoint.label << ",";

        // Convertir la matriz descriptor a una cadena CSV
        for (int i = 0; i < dataPoint.descriptor.rows; ++i) {
            for (int j = 0; j < dataPoint.descriptor.cols; ++j) {
                outputFile << static_cast<int>(dataPoint.descriptor.at<uchar>(i, j));
                if (j < dataPoint.descriptor.cols - 1) {
                    outputFile << " ";
                }
            }
            if (i < dataPoint.descriptor.rows - 1) {
                outputFile << ",";
            }
        }

        outputFile << std::endl;
    }

    outputFile.close();
}

// Función para cargar datos desde un archivo CSV y agregarlos al vector trainingData
void loadFromCSV(const std::string& filePath, std::vector<ImageDataPoint>& trainingData) {
    std::ifstream inputFile(filePath);
    if (!inputFile.is_open()) {
        std::cerr << "Error al abrir el archivo CSV para lectura: " << filePath << std::endl;
        return;
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        std::istringstream ss(line);
        ImageDataPoint dataPoint;

        // Leer la etiqueta
        std::getline(ss, dataPoint.label, ',');

        // Leer el descriptor y convertirlo de cadena a matriz
        std::string descriptorStr;
        std::getline(ss, descriptorStr);

        std::istringstream descriptorStream(descriptorStr);
        std::vector<std::string> descriptorValues;
        std::string value;
        while (std::getline(descriptorStream, value, ' ')) {
            descriptorValues.push_back(value);
        }

        int rows = descriptorValues.size();
        int cols = 1; // Inicialmente, asumimos una sola columna

        dataPoint.descriptor = cv::Mat(rows, cols, CV_8U);

        for (int i = 0; i < rows; ++i) {
            int intValue = 0;
            try {
                intValue = std::stoi(descriptorValues[i]);
            }
            catch (std::invalid_argument& e) {
                // Manejar la excepción si la conversión falla
                std::cerr << "Error al convertir el valor en fila " << i << ": " << e.what() << std::endl;
                continue; // Saltar esta fila
            }
            dataPoint.descriptor.at<uchar>(i, 0) = static_cast<uchar>(intValue);
        }

        trainingData.push_back(dataPoint);
    }

    inputFile.close();
}

void entrenamiento() {
    std::cout << "Iniciando: " << std::endl;

    // Ruta de la carpeta de entrenamiento
    std::string trainingFolderPath = "road_signs";

    // Número total de imágenes de entrenamiento
    int numTrainingImages = 876;

    // Tamaño de destino para redimensionar las imágenes
    cv::Size targetSize(256, 256); // Cambia el tamaño según tus necesidades

    // Generar los datos de entrenamiento
    std::vector<ImageDataPoint> trainingData = generateTrainingData(trainingFolderPath, numTrainingImages, targetSize);
    std::cout << "Cantidad en trainingData: " << trainingData.size() << std::endl;
    // Exportar los datos a un archivo CSV
    std::string csvFilePath = "training_data.csv";
    exportToCSV(trainingData, csvFilePath);
    std::cout << "Datos exportados a: " << csvFilePath << std::endl;
}
void prediccion() {
    int numTestImages = 20;
    int numTrainingImages = 876;
    std::string trainingFolderPath = "road_signs";
    // Tamaño de destino para redimensionar las imágenes
    cv::Size targetSize(256, 256); // Cambia el tamaño según tus necesidades

    // Ruta del archivo CSV a cargar
    std::string csvFilePath = "training_data.csv";

    // Vector para almacenar los datos cargados
    std::vector<ImageDataPoint> trainingData;

    // Cargar datos desde el archivo CSV
    loadFromCSV(csvFilePath, trainingData);
    std::cout << "Cantidad en trainingData: " << trainingData.size() << std::endl;

    for (int i = 0; i < numTestImages; ++i) {
        int numeroAleatorio = generarNumeroAleatorio(0, 876);
        std::string imagePath = trainingFolderPath + "/images/road" + std::to_string(numeroAleatorio) + ".png";
        std::string xmlPath = trainingFolderPath + "/annotations/road" + std::to_string(numeroAleatorio) + ".xml";

        cv::Mat inputDescriptor = generateCannyDescriptor(imagePath, targetSize);
        if (inputDescriptor.empty()) {
            break;
        }
        int k = 3;

        // Clasificar la imagen de entrada
        std::string predictedLabel = kNNClassify(trainingData, inputDescriptor, k);

        std::string label = getLabelFromXML(xmlPath);
        if (label == predictedLabel) {
            std::cout << "Etiqueta predecida: " << predictedLabel << std::endl;
        }
        else
        {
            std::cout << "Etiqueta de la imagen: " << label << "Etiqueta predecida: " << predictedLabel << std::endl;
        }
    }

}

int main() {
    //entrenamiento();
    prediccion();
}
