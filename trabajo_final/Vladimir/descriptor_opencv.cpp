#include<opencv\cv.h>
#include<opencv\highgui.h>
#include<opencv2\nonfree\nonfree.hpp>

using namespace cv;
using namespace std;
int main(){

Mat src, descriptors,dest;
vector<KeyPoint> keypoints;

src = imread("D:\\Prueba.jpg");


cvtColor(src, src, CV_BGR2GRAY);

SIFT sift(2000,3,0.004);
sift(src, src, keypoints, descriptors, false);
drawKeypoints(src, keypoints, dest);
imshow("Sift", dest);
cvWaitKey(0);
return 0;
}