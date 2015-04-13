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
	std::vector<Mat> getRecentOperationOnVector(bool inColor = true);
	//pobranie obrazka referencyjnego w stanie po ostatniej operacji na nim
	Mat getRecentOperationOnReferenceImage(bool inColor = true);


	//poni¿sze funkcje wykonuj¹ operacje na ca³ym wektorku za³adowanych obrazów

	//liczy ró¿nice miêdzy obrazkiem referencyjnym a pozosta³ymi
	bool imagesDifference();
	//wykrywa krawêdzie, jeszcze dobrze nie dziala!!!
	bool detectEdges(makeOperationOn makeOn = ALL);
	//rozmazanko
	bool blurImages(makeOperationOn makeOn = ALL, int size = 5);
	//filtr medianowy - nie rozmazuje krawêdzi! Uwaga!!! Musi byæ NIEPARZYSTY SIZE!!!
	bool medianFiltr(makeOperationOn makeOn = ALL, int size = 5);
	//zawsze zapominam jak to s³ówko sie t³umaczy na polski - operacaja wykonywana tylko na 1-channelowym czyli np grayscale, na razie nie ma tych œmiechowych argumentów
	bool threshold(makeOperationOn makeOn = ALL);



};




#endif