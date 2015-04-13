#ifndef IMAGEOPERATIONS
#define IMAGEOPERATIONS

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <vector>

using namespace cv;

class ImageOperations {

private:
	Mat referenceImage;
	Mat referenceImageGrayscale;
	Mat recentOperation;
	Mat recentOperationGrayscale;
	std::vector<Mat> savedOperationsOnReferenceImage;
	std::vector<Mat> savedOperationsOnReferenceImageGrayscale;

	std::vector<Mat> loadedImages;
	std::vector<Mat> loadedImagesGrayscale;
	std::vector<Mat> recentOperationOnVector;
	std::vector<Mat> recentOperationOnVectorGrayscale;
	std::vector<std::vector<Mat>> savedOperationsOnVector;
	std::vector<std::vector<Mat>> savedOperationsOnVectorGrayscale;

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
	std::vector<Mat> getRecentOperationOnVector(bool inColor = true);
	//pobranie obrazka referencyjnego w stanie po ostatniej operacji na nim
	Mat getRecentOperationOnReferenceImage(bool inColor = true);


	//poni�sze funkcje wykonuj� operacje na ca�ym wektorku za�adowanych obraz�w

	//liczy r�nice mi�dzy obrazkiem referencyjnym a pozosta�ymi
	bool imagesDifference();
	//wykrywa kraw�dzie, jeszcze dobrze nie dziala!!!
	bool detectEdges(makeOperationOn makeOn = ALL);
	//rozmazanko
	bool blurImages(makeOperationOn makeOn = ALL, int size = 5);
	//filtr medianowy - nie rozmazuje kraw�dzi! Uwaga!!! Musi by� NIEPARZYSTY SIZE!!!
	bool medianFiltr(makeOperationOn makeOn = ALL, int size = 5);
	//zawsze zapominam jak to s��wko sie t�umaczy na polski - operacaja wykonywana tylko na 1-channelowym czyli np grayscale, na razie nie ma tych �miechowych argument�w
	bool threshold(makeOperationOn makeOn = ALL);



};




#endif