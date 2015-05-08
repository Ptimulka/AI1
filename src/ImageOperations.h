#ifndef IMAGEOPERATIONS
#define IMAGEOPERATIONS

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/video/background_segm.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/core/ocl.hpp"

#include <vector>
#include <assert.h>
#include <limits>
#include <iostream>

using namespace cv;

class ImageOperations {

private:	
	UMat meanImageGrayscale;
	Size sizeOfImage;

	std::vector<UMat> loadedImages;
	std::vector<UMat> loadedImagesGrayscale;

	std::vector<std::vector<Rect>> vectorsOfRectsFound;
	std::vector<std::vector<Rect>> vectorsOfRectsGeneratedByJoining;

	bool vectorOfImagesLoaded;
	void createMeanImage();
	void imagesDifference();
	void markCars(UMat & mat);

public:
	ImageOperations();
	~ImageOperations();

	enum ImagesErrors {
		OK,
		NO_PATH,
		DIFFERENT_SIZES,
		OPENCV_ERROR
	};

	//³aduje wektor obrazków; jeœli ju¿ s¹ za³adowane, to je nadpisuje; 
	//zwraca 0 jak sie uda albo blad
	ImagesErrors loadVectorOfImages(std::vector<std::string> paths);
	//sprawdza czy wektor obrazków jest za³adowany
	bool isVectorOfImagesLoaded();
	//pobranie obrazków
	std::vector<UMat> getLoadedImages();

	//funkcja robiaca wszystko :)
	void markCarsWithOptions(int size, int numberOfBlurs, int thresh, Scalar color);


	void tryTrickWithOpenCL();


};




#endif