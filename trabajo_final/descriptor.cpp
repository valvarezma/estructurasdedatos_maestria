#include <opencv2/opencv.hpp>

int main() {
    // Cargar imágenes
    cv::Mat image1 = cv::imread("image1.jpg", cv::IMREAD_GRAYSCALE);
    cv::Mat image2 = cv::imread("image2.jpg", cv::IMREAD_GRAYSCALE);

    // Inicializar el detector y el extractor SIFT
    cv::Ptr<cv::SIFT> sift = cv::SIFT::create();

    // Encontrar puntos clave y calcular descriptores
    std::vector<cv::KeyPoint> keypoints1, keypoints2;
    cv::Mat descriptors1, descriptors2;

    sift->detectAndCompute(image1, cv::noArray(), keypoints1, descriptors1);
    sift->detectAndCompute(image2, cv::noArray(), keypoints2, descriptors2);

    // Realizar comparaciones entre descriptores, etc.

    return 0;
}
