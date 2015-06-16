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
	std::vector<std::vector<Rect>> vectorsOfRectsAll;
	std::vector<std::vector<uint>> rectsSetAsCars;
	std::vector<std::vector<Mat>> rectsChangedForAnn;

	bool vectorOfImagesLoaded;

	//tworzy obrazek sredni magicznym sposobem
	void createMeanImage();

	//pobiera obrazek-ró¿nicê i znalezione podejrzane miejsca o bycie samochodem dodaje do rects
	void findContoursAddRects(UMat &mat, std::vector<Rect> &rects);

	//usuwa baaardzo podobne prostok¹ty i tworzy nowe, union oczywiscie nie znaczy union tylko najmniejszy prostok¹t zawieraj¹cy oba
	void deleteDuplicatesAddUnions(std::vector<Rect> &rects, std::vector<Rect> &rectsUnions);

	//do uczenia sieci
	std::vector<Mat> learningImages;
	



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
	//zwraca OK = 0 jak sie uda albo blad
	ImagesErrors loadVectorOfImages(std::vector<std::string> paths);

	//sprawdza czy wektor obrazków jest za³adowany
	bool isVectorOfImagesLoaded();

	//pobranie obrazków
	std::vector<UMat>& getLoadedImages();
	std::vector<UMat>& getLoadedImagesWithPossibleCars();	//pobranie tych z zaznaczonymi na pomaranczowo a po wywolaniu markRealCars takze zielonymi

	//wykonuje na obrazku-ró¿nicy rozmazanko a potem ró¿ne treshholdy i dla ka¿dego dodaje prostok¹ty do ogólnej puli
	void addRectsWithOptions(int size, int numberOfBlurs, std::vector<int> threshes);

	
	//zaznacza miejsca podejrzane o bycie samochodami, te nie z unions i z unions na pomaranczowo
	void markAllPossibleCars();

	//pobranie wszystkich prostokatow juz jako Maty (wycinki szarych obrazkow)
	Mat getMatScaledTo(int obrazek, int podobrazek, int width, int height);

    inline int getImagesCount() const { return loadedImages.size(); }
    inline int getRectsCountForImage(int img) const { return vectorsOfRectsAll[img].size(); }

	//zapamietanie ze j-ty prostokat i-tego obrazka jest samochodem
	void setRectAsCar(uint image, uint rect);

	//to uruchamiamy po obczajeniu sieci¹ które s¹ samochodami - rysuje zielone na tych pomaranczowych
	void markRealCars();

	//to poczebne przy uczeniu sieci, zaladowanie i przeskalowanie obrazkow
	ImagesErrors loadVectorOfImagesToLearn(std::vector<std::string> paths);
	std::vector<Mat>& getLearningImagesScaledTo(int width, int height);

};




#endif