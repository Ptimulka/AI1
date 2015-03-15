#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;





Mat *detectEdges(String name, bool color) {

	Mat mat;
	if (!color)
		mat = imread(name, CV_LOAD_IMAGE_GRAYSCALE);
	else
		mat = imread(name, CV_LOAD_IMAGE_COLOR);

	if (mat.data == NULL)
		return NULL;	//not found
	

	Mat *detected = new Mat();

	int lowThreshold = 80;
	int ratio = 3;
	int kernel_size = 3;


	blur(mat, mat, Size(3, 3));

	Canny(mat, *detected, lowThreshold, lowThreshold*ratio, kernel_size);

	return detected;	//everything is awesome
}



void compareImages() {

	Mat im_gray0 = imread("zdj0.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	Mat im_color0 = imread("zdj0.jpg", CV_LOAD_IMAGE_COLOR);

	Mat im_gray = imread("zdj000013.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	Mat im_color = imread("zdj000013.jpg", CV_LOAD_IMAGE_COLOR);

	blur(im_gray0, im_gray0, Size(5, 5));
	blur(im_gray, im_gray, Size(5, 5));

	

	Mat dst;

	//compare(im_gray0, im_gray, dst, CMP_EQ);
	//addWeighted(im_gray, 4, im_gray0, -4, 60, dst);
	absdiff(im_gray0, im_gray, dst);
	blur(dst, dst, Size(11, 11));

	threshold(dst, dst, 40, 255, THRESH_TOZERO);
	//blur(dst, dst, Size(11, 11));

	namedWindow("window3", WINDOW_AUTOSIZE); // Create a window for display.
	imshow("window3", dst); // Show our image inside it.
	waitKey();//without this image won't be shown
}


void compareAndEdges() {

	

	Mat im_gray0 = imread("zdj0.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	Mat im_color0 = imread("zdj0.jpg", CV_LOAD_IMAGE_COLOR);

	Mat im_gray = imread("zdj000013.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	Mat im_color = imread("zdj000013.jpg", CV_LOAD_IMAGE_COLOR);

	blur(im_gray0, im_gray0, Size(5, 5));
	blur(im_gray, im_gray, Size(5, 5));



	Mat dst;

	//compare(im_gray0, im_gray, dst, CMP_EQ);
	//addWeighted(im_gray, 4, im_gray0, -4, 60, dst);
	absdiff(im_gray0, im_gray, dst);
	blur(dst, dst, Size(11, 11));

	threshold(dst, dst, 10, 255, THRESH_BINARY);
	//blur(dst, dst, Size(11, 11));





	int lowThreshold = 10;
	int const max_lowThreshold = 100;
	int ratio = 3;
	int kernel_size = 3;


	//Canny(dst, dst, lowThreshold, lowThreshold*ratio, kernel_size);




	namedWindow("window4", WINDOW_AUTOSIZE); // Create a window for display.
	imshow("window4", dst); // Show our image inside it.
	waitKey();//without this image won't be shown





}

Mat *imagesDifference(Mat mat1, Mat mat2) {

	Mat *diff = new Mat();
	absdiff(mat1, mat2, *diff);

	return diff;	//everything is awesome
}


Mat *imagesDifference(Mat mat1, String im2, bool color) {	

	Mat mat2;
	Mat *diff = new Mat();

	if (color) 
		mat2 = imread(im2, CV_LOAD_IMAGE_COLOR);
	else 
		mat2 = imread(im2, CV_LOAD_IMAGE_GRAYSCALE);


	if (mat2.data == NULL)
		return NULL;	//image not found
	absdiff(mat1, mat2, *diff);

	return diff;	//everything is awesome
}


Mat *imagesDifference(String im1, String im2, bool color) {

	Mat mat1;
	Mat mat2;

	if (color) {
		mat1 = imread(im1, CV_LOAD_IMAGE_COLOR);
		mat2 = imread(im2, CV_LOAD_IMAGE_COLOR);
	}
	else {
		mat1 = imread(im1, CV_LOAD_IMAGE_GRAYSCALE);
		mat2 = imread(im2, CV_LOAD_IMAGE_GRAYSCALE);
	}

	if (mat1.data == NULL || mat2.data == NULL)
		return NULL;	//image not found

	Mat *diff = new Mat();
	absdiff(mat1, mat2, *diff);

	return diff;	//everything is awesome
}



Mat *averageImage(std::vector<String> &imageNames, bool color) {

	vector<Mat> images;
	for (int i = 0; i < imageNames.size(); i++) {
		Mat next;
		if (color)
			next = imread(imageNames.at(i), CV_LOAD_IMAGE_COLOR);
		else
			next = imread(imageNames.at(i), CV_LOAD_IMAGE_GRAYSCALE);
		if (next.data == NULL)
			return NULL;	//image not found
		images.push_back(next);
	}
	
	Mat *average = new Mat();
	*average = images.at(0);

	for (int i = 1; i < images.size(); i++) 
		addWeighted(*average, (double)(i) / (i + 1), images.at(i), 1.0 / (i + 1), 0, *average);

	return average;	//everything is awesome
}



int main(int argc, char** argv)
{
	/*
	vector<String> imageNames;
	

	imageNames.push_back("zdj0.jpg");
	imageNames.push_back("zdj000001.jpg");
	imageNames.push_back("zdj000002.jpg");
	imageNames.push_back("zdj000003.jpg");
	imageNames.push_back("zdj000004.jpg");
	imageNames.push_back("zdj000005.jpg");
	imageNames.push_back("zdj000006.jpg");
	imageNames.push_back("zdj000007.jpg");
	imageNames.push_back("zdj000008.jpg");
	imageNames.push_back("zdj000009.jpg");
	imageNames.push_back("zdj000010.jpg");
	imageNames.push_back("zdj000011.jpg");
	imageNames.push_back("zdj000012.jpg");
	imageNames.push_back("zdj000013.jpg");
	
	

	bool color = true;

	//compareAndEdges();
	Mat *average = averageImage(imageNames, color);

	//Mat *diff = imagesDifference(*average, "zdj000013.jpg", color);
	Mat *diff = imagesDifference("zdj0.jpg", "zdj000013.jpg", color);

	blur(*diff, *diff, Size(9, 9));
	blur(*diff, *diff, Size(9, 9));
	blur(*diff, *diff, Size(9, 9));
	threshold(*diff, *diff, 30, 255, THRESH_BINARY);

	*/

	

	/*

	Mat *detectedEdges = detectEdges("cctv45009.jpg", false);

	namedWindow("window5", WINDOW_AUTOSIZE); // Create a window for display.
	imshow("window5", *detectedEdges); // Show our image inside it.
	*/

	Mat hist = imread("cctv45009.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	equalizeHist(hist, hist);

	Mat hist2 = imread("cctv00000.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	equalizeHist(hist2, hist2);

	Mat *diff = imagesDifference("cctv00000.jpg", "cctv45009.jpg", false);
	blur(*diff, *diff, Size(3, 3));
	threshold(*diff, *diff, 60, 255, THRESH_TOZERO);

	Mat *diff2 = imagesDifference(hist, hist2);
	blur(*diff2, *diff2, Size(3, 3));
	threshold(*diff2, *diff2, 60, 255, THRESH_TOZERO);


	namedWindow("window5", WINDOW_AUTOSIZE); // Create a window for display.
	imshow("window5", *diff2); // Show our image inside it.

	namedWindow("window6", WINDOW_AUTOSIZE); // Create a window for display.
	imshow("window6", *diff); // Show our image inside it.
	waitKey();//without this image won't be shown

		
	return 0;
}