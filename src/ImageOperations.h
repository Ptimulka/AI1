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



	//�aduje obrazek referencyjny; je�li jaki� ju� by� za�adowany to go nadpisuje; zwraca true/false w zale�no�ci od tego czy si� uda�o czy nie;
	bool loadReferenceImage(std::string referenceImagePath);
	//�aduje wektor obrazk�w; je�li ju� s� za�adowane, to je nadpisuje; zwraca true/false w zale�no�ci od tego czy si� uda�o czy nie;
	bool loadVectorOfImages(std::vector<std::string> paths);
	//sprawdza czy s� za�adowane wszystkie obrazki  
	bool areImagesLoaded();
	//sprawdza czy jest za�adowany obrazek referencyjny
	bool isReferenceImageLoaded();
	//sprawdza czy wektor obrazk�w jest za�adowany (nie obrazek referencyjny!)
	bool isVectorOfImagesLoaded();

	//zapisuje ostatni� operacj� na wektorze obraz�w
	void pushRecentOperationOnVector();
	//zapisuje ostani� operacj� na obrazku referencyjnym
	void pushRecentOperationOnReferenceImage();

	//pobranie obrazk�w w stanie po ostatniej operacji na nich
	std::vector<UMat> getRecentOperationOnVector(bool inColor = true);
	//pobranie obrazka referencyjnego w stanie po ostatniej operacji na nim
	UMat getRecentOperationOnReferenceImage(bool inColor = true);


	//poni�sze funkcje wykonuj� operacje na ca�ym wektorku za�adowanych obraz�w

	//liczy r�nice mi�dzy obrazkiem referencyjnym a pozosta�ymi, lub tworzy obrazek �redni i go traktuje jako referencyjny, drugi parametr dotyczy w�a�nie tego
	//tzn czy chcemy skorzysta� z wyrzucania pikselk�w bardzo odchylonych od �redniej
	bool imagesDifference(bool useUserReferenceImage = true, bool doItWiselyButLong = true);
	//wykrywa kraw�dzie, jeszcze dobrze nie dziala!!!
	bool detectEdges(makeOperationOn makeOn = VECTOR_OF_IMAGES);
	//rozmazanko
	bool blurImages(makeOperationOn makeOn = ALL, int size = 5);
	//filtr medianowy - nie rozmazuje kraw�dzi! Uwaga!!! Musi by� NIEPARZYSTY SIZE!!!
	bool medianFiltr(makeOperationOn makeOn = ALL, int size = 5);
	//zawsze zapominam jak to s��wko sie t�umaczy na polski - operacaja wykonywana tylko na 1-channelowym czyli np grayscale
	//thresh  - minimum �eby zosta� bia�ym pikselem
	bool threshold(int thresh = 50, makeOperationOn makeOn = VECTOR_OF_IMAGES);


	//funkcja obczajaj�ca kontury
	UMat markCars(int whichImage, unsigned char red = 255, unsigned char green = 0, unsigned char blue = 0);
	std::vector<UMat> markCars(unsigned char red = 255, unsigned char green = 0, unsigned char blue = 0);

	void robCosZebyZajacGPU();

};




#endif