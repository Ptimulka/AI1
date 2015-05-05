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

using namespace cv;

class ImageOperations {

private:
	UMat referenceImage;
	UMat referenceImageGrayscale;
	UMat recentOperation;
	UMat recentOperationGrayscale;
	std::vector<UMat> savedOperationsOnReferenceImage;
	std::vector<UMat> savedOperationsOnReferenceImageGrayscale;
	UMat meanImage;

	std::vector<UMat> loadedImages;
	std::vector<UMat> loadedImagesGrayscale;
	std::vector<UMat> recentOperationOnVector;
	std::vector<UMat> recentOperationOnVectorGrayscale;
	std::vector<std::vector<UMat>> savedOperationsOnVector;
	std::vector<std::vector<UMat>> savedOperationsOnVectorGrayscale;

	bool referenceImageLoaded;
	bool vectorOfImagesLoaded;


public:
	enum makeOperationOn {
		VECTOR_OF_IMAGES,
		REFERNCE_IMAGE,
		ALL
	};

private:
	bool isWhatShouldBeLoaded(makeOperationOn makeOn);
	bool isContourOk(std::vector<Point> contour, int sizeMin = 10);


public:
	ImageOperations(std::string referenceImagePath, std::vector<std::string> paths);
	ImageOperations();
	~ImageOperations();



	//³aduje obrazek referencyjny; jeœli jakiœ ju¿ by³ za³adowany to go nadpisuje; zwraca true/false w zale¿noœci od tego czy siê uda³o czy nie;
	bool loadReferenceImage(std::string referenceImagePath);
	//³aduje wektor obrazków; jeœli ju¿ s¹ za³adowane, to je nadpisuje; zwraca true/false w zale¿noœci od tego czy siê uda³o czy nie;
	bool loadVectorOfImages(std::vector<std::string> paths);
	//sprawdza czy s¹ za³adowane wszystkie obrazki  
	bool areImagesLoaded();
	//sprawdza czy jest za³adowany obrazek referencyjny
	bool isReferenceImageLoaded();
	//sprawdza czy wektor obrazków jest za³adowany (nie obrazek referencyjny!)
	bool isVectorOfImagesLoaded();

	//zapisuje ostatni¹ operacjê na wektorze obrazów
	void pushRecentOperationOnVector();
	//zapisuje ostani¹ operacjê na obrazku referencyjnym
	void pushRecentOperationOnReferenceImage();

	//pobranie obrazków w stanie po ostatniej operacji na nich
	std::vector<UMat> getRecentOperationOnVector(bool inColor = true);
	//pobranie obrazka referencyjnego w stanie po ostatniej operacji na nim
	UMat getRecentOperationOnReferenceImage(bool inColor = true);


	//poni¿sze funkcje wykonuj¹ operacje na ca³ym wektorku za³adowanych obrazów

	//liczy ró¿nice miêdzy obrazkiem referencyjnym a pozosta³ymi, lub tworzy obrazek œredni i go traktuje jako referencyjny, drugi parametr dotyczy w³aœnie tego
	//tzn czy chcemy skorzystaæ z wyrzucania pikselków bardzo odchylonych od œredniej
	bool imagesDifference(bool useUserReferenceImage = true, bool doItWiselyButLong = true);
	//wykrywa krawêdzie, jeszcze dobrze nie dziala!!!
	bool detectEdges(makeOperationOn makeOn = VECTOR_OF_IMAGES);
	//rozmazanko
	bool blurImages(makeOperationOn makeOn = ALL, int size = 5);
	//filtr medianowy - nie rozmazuje krawêdzi! Uwaga!!! Musi byæ NIEPARZYSTY SIZE!!!
	bool medianFiltr(makeOperationOn makeOn = ALL, int size = 5);
	//zawsze zapominam jak to s³ówko sie t³umaczy na polski - operacaja wykonywana tylko na 1-channelowym czyli np grayscale
	//thresh  - minimum ¿eby zostaæ bia³ym pikselem
	bool threshold(int thresh = 50, makeOperationOn makeOn = VECTOR_OF_IMAGES);


	//funkcja obczajaj¹ca kontury
	UMat markCars(int whichImage, unsigned char red = 255, unsigned char green = 0, unsigned char blue = 0);
	std::vector<UMat> markCars(unsigned char red = 255, unsigned char green = 0, unsigned char blue = 0);

	void robCosZebyZajacGPU();

};




#endif