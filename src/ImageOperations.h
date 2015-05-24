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
	std::vector<UMat> loadedImagesWithPossibleCars;
	std::vector<UMat> loadedImagesGrayscale;
	std::vector<UMat> imagesDifferences;

	std::vector<std::vector<Rect>> vectorsOfRectsFound;
	std::vector<std::vector<Rect>> vectorsOfRectsGeneratedByUnions;
	
	std::vector<std::vector<uint>> rectsSetAsCars;

	std::vector<std::vector<Mat>> rectsChangedForAnn;

	bool vectorOfImagesLoaded;

	//tworzy obrazek sredni magicznym sposobem
	void createMeanImage();
	//pobiera obrazek-r�nic� i znalezione podejrzane miejsca o bycie samochodem dodaje do rects
	void findContoursAddRects(UMat &mat, std::vector<Rect> &rects);
	//usuwa baaardzo podobne prostok�ty i tworzy nowe, union oczywiscie nie znaczy union tylko najmniejszy prostok�t zawieraj�cy oba
	void deleteDuplicatesAddUnions(std::vector<Rect> &rects, std::vector<Rect> &rectsUnions);




public:
	ImageOperations();
	~ImageOperations();

	enum ImagesErrors {
		OK,
		NO_PATH,
		DIFFERENT_SIZES,
		OPENCV_ERROR
	};

	//�aduje wektor obrazk�w; je�li ju� s� za�adowane, to je nadpisuje; 
	//zwraca OK = 0 jak sie uda albo blad
	ImagesErrors loadVectorOfImages(std::vector<std::string> paths);
	//sprawdza czy wektor obrazk�w jest za�adowany
	bool isVectorOfImagesLoaded();
	//pobranie obrazk�w
	std::vector<UMat>& getLoadedImages();
	std::vector<UMat>& getLoadedImagesWithPossibleCars();

	//wykonuje na obrazku-r�nicy rozmazanko a potem r�ne treshholdy i dla ka�dego dodaje prostok�ty do og�lnej puli
	void addRectsWithOptions(int size, int numberOfBlurs, std::vector<int> threshes);

	
	//zaznacza miejsca podejrzane o bycie samochodami, te nie z unions i z unions
	void markAllPossibleCars();
	std::vector<std::vector<Mat>>& getMatsScaledTo(int width, int height);
	void setRectAsCar(uint image, uint rect);
	//to uruchamiamy po obczajeniu sieci� kt�re s� samochodami
	void markRealCars();


	void tryTrickWithOpenCL();


};




#endif