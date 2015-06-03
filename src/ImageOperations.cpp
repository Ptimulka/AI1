#include "ImageOperations.h"

ImageOperations::ImageOperations() {
	vectorOfImagesLoaded = false;
}

ImageOperations::~ImageOperations() {
}



//ladowanie obrazkow i wstepne przetwarzanko

ImageOperations::ImagesErrors ImageOperations::loadVectorOfImages(std::vector<std::string> paths) {

	if (paths.size() == 0)
		return ImageOperations::NO_PATH;

	std::vector<UMat> loadedImages;
	std::vector<UMat> loadedImagesGrayscale;
	std::vector<UMat> loadedImagesWithPossibleCars;

	for (decltype(paths.size()) i = 0; i < paths.size(); i++) {
		Mat next = imread(paths.at(i), IMREAD_COLOR);
		if (next.data == NULL) {
			vectorOfImagesLoaded = false;
			loadedImages.clear();
			loadedImagesGrayscale.clear();			
			loadedImagesWithPossibleCars.clear();
			return ImageOperations::OPENCV_ERROR;
		}

		if (i == 0)
			sizeOfImage = next.size();
		else if (next.size() != sizeOfImage) {
			vectorOfImagesLoaded = false;
			loadedImages.clear();
			loadedImagesGrayscale.clear();
			loadedImagesWithPossibleCars.clear();
			return ImageOperations::DIFFERENT_SIZES;
		}

		Mat nextGray = imread(paths.at(i), IMREAD_GRAYSCALE);
		loadedImages.push_back(next.getUMat(ACCESS_RW));
		loadedImagesGrayscale.push_back(nextGray.getUMat(ACCESS_RW));
		loadedImagesWithPossibleCars.push_back(loadedImages[i].clone());
	}

	vectorOfImagesLoaded = true;
	this->loadedImages.clear();
	this->loadedImagesGrayscale.clear();
	this->loadedImages = loadedImages;
	this->loadedImagesGrayscale = loadedImagesGrayscale;
	this->loadedImagesWithPossibleCars = loadedImagesWithPossibleCars;


	//stwarzamy od razu obrazek sredni
	createMeanImage();
	Mat jakas = meanImageGrayscale.getMat(ACCESS_RW);

	//oraz obrazki-ró¿nice
	for (decltype(loadedImages.size()) i = 0; i < loadedImages.size(); ++i) {
		UMat next = loadedImagesGrayscale[i].clone();
		absdiff(next, meanImageGrayscale, next);
		imagesDifferences.push_back(next);

		//tworzymy tez wektory na prostokaty
		vectorsOfRectsFound.push_back(std::vector<Rect>());
		vectorsOfRectsGeneratedByUnions.push_back(std::vector<Rect>());
		vectorsOfRectsAll.push_back(std::vector<Rect>());

	}

	return ImageOperations::OK;
}

bool ImageOperations::isVectorOfImagesLoaded() {
	return vectorOfImagesLoaded;
}

void ImageOperations::createMeanImage() {

	meanImageGrayscale = UMat::zeros(sizeOfImage, CV_32FC1);
	UMat convertedToFloat;

	decltype(loadedImages.size()) sizeOfVector = loadedImages.size();

	//dodajemy obrazki do siebie, musza byc floatami, po inaczej unsigned char sie przekreci chyba
	for (decltype(sizeOfVector) i = 0; i < sizeOfVector; i++) {
		loadedImagesGrayscale.at(i).convertTo(convertedToFloat, CV_32FC1);
		accumulate(convertedToFloat, meanImageGrayscale);
	}

	//mamy juz sume wszystkich, trzeba tylko podzielic przez liczbe obrazkow
	UMat jedynki = UMat::ones(sizeOfImage, CV_32FC1);
	divide(meanImageGrayscale, jedynki, meanImageGrayscale, 1.0 / sizeOfVector);

	//musimy zamienic na nie float
	meanImageGrayscale.convertTo(meanImageGrayscale, CV_8U);

	Mat dopod = meanImageGrayscale.getMat(ACCESS_READ);

	//a teraz algorytm wyrzucania tych z wiekszym odchyleniem niz odchylenie standardowym

	//musimy stworzyc zwykle Mat, nie UMat, bo inaczej nie dziala mat.at()
	Mat meanGrayscaleMat = meanImageGrayscale.getMat(ACCESS_RW);
	std::vector<Mat> maty;
	for (decltype(sizeOfVector) i = 0; i < sizeOfVector; i++) {
		maty.push_back(loadedImagesGrayscale.at(i).getMat(ACCESS_RW));
	}

	//dla kazdego piksela...
	for (int y = 0; y < meanImageGrayscale.rows; y++) {
		for (int x = 0; x < meanImageGrayscale.cols; x++) {

			float standardDeviation = 0;
			float average = (float)(meanGrayscaleMat.at<unsigned char>(cv::Point(x, y)));

			//liczymy standardowe odchylenie czy cos w tym stylu bo to najprostsze jakie moze byc
			for (decltype(sizeOfVector) i = 0; i < sizeOfVector; i++)
				standardDeviation += abs((float)(maty.at(i).at<unsigned char>(cv::Point(x, y))) - average);
			standardDeviation /= sizeOfVector;

			//najpierw zakladamy ze wszystkie sa ok, tzn ich odchylenie jest ponizej standardowego (co oczywiscie jest raczej niemal niemozliwe)
			int notSuspected = sizeOfVector;
			float averageChanged = average;

			//dla kazdego obrazka sprawdzay piksel na pozycji x,y, czy nie jest podejrzany
			for (decltype(sizeOfVector) i = 0; i < sizeOfVector; i++) {

				if (abs((float)(maty.at(i).at<unsigned char>(cv::Point(x, y))) - average) > standardDeviation) {
					//jesli jest to odpowiednio zmieniamy srednia
					averageChanged = averageChanged - ((float)(maty.at(i).at<unsigned char>(cv::Point(x, y)))) / notSuspected;
					averageChanged *= ((float)notSuspected);
					notSuspected--;
					//raczej nie bêdzie asserta, bo to niemo¿liwe, ¿e wszystkie bêdê mia³y odchylenie wiêksze ni¿ standardowe
					assert(notSuspected != 0);
					averageChanged /= ((float)notSuspected);
				}

			}

			//wpisujemy nowy kolor 
			meanGrayscaleMat.at<unsigned char>(cv::Point(x, y)) = (unsigned char)averageChanged;

		}
	}


	return;
}

std::vector<UMat>& ImageOperations::getLoadedImages() {
	return loadedImages;
}



//przetwarzanko prywatne

void ImageOperations::findContoursAddRects(UMat &mat, std::vector<Rect> &rects) {

	std::vector<Vec4i> hierarchy;
	std::vector<std::vector<Point> > contours;

	UMat tmp;
	int thresh = 100;
	int minArea = 100;	//minimalny obszar protok¹ta opisanego na konturze
	int minLength = 20;		//minimalna d³ugoœæ konturu (z ilu pikseli sie sk³ada)


	Canny(mat, tmp, thresh, thresh * 2, 3);		//doprowadza do tego, ¿e mamy zaznaczone bia³ymi liniami kontury
	findContours(tmp, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));	//na obrazku z bia³ymi liniami obczaja które tworz¹ kontury

	//wyrzucamy kontury o ma³ej d³ugoœci
	std::vector< std::vector<Point> >::iterator iter;
	for (iter = contours.begin(); iter != contours.end();) {
		if ((*iter).size() > 20)
			iter++;
		else
			iter = contours.erase(iter);
	}


	//tworzymy dla ka¿dego konturu prostok¹t i jeœli powierzchnia jest odpowiednia to zachowujemy ten kontur, a prostokat dodajemy do rects
	for (iter = contours.begin(); iter != contours.end(); iter++) {
		Rect rect = boundingRect(*iter);
		if (rect.area() > minArea && rect.area() < 0.25*sizeOfImage.area() && rect.width < 0.5*sizeOfImage.width && rect.height < 0.5*sizeOfImage.height && rect.height * 2.5 > rect.width && rect.width*2.5 > rect.height)
			rects.push_back(rect);
	}


}

void ImageOperations::deleteDuplicatesAddUnions(std::vector<Rect> &rects, std::vector<Rect> &rectsUnions) {


	//usuwanie "duplikatów"
	double jakaCzescDuzego = 0.85;

	std::vector<Rect>::iterator iter1;
	for (iter1 = rects.begin(); iter1 != rects.end(); iter1++) {

		std::vector<Rect>::iterator iter2;

		for (iter2 = iter1 + 1; iter2 != rects.end();) {

			Rect intersect = (*iter1) & (*iter2);
			if ((*iter1) == (*iter2) || (intersect.area() > jakaCzescDuzego*(*iter1).area() && intersect.area() > jakaCzescDuzego*(*iter2).area())) {
				//zakladamy ¿e to duplikat, jeœli sa równe albo ich przeciêcie stanowi duuu¿¹ czêœæ obu (jakaCzescDuzego)
				iter2 = rects.erase(iter2);
			}
			else
				iter2++;
		}

	}

	//a teraz sumy prostok¹tów, teoretycznie kiedy mog¹ stanowiæ ten sam samochód
	double jakaCzesc = 0.5;
	for (iter1 = rects.begin(); iter1 != rects.end(); iter1++) {

		std::vector<Rect>::iterator iter2;
		for (iter2 = iter1 + 1; iter2 != rects.end(); iter2++) {

			Rect intersect = (*iter1) & (*iter2);

			if (intersect.area() > 0.5*(*iter1).area() || intersect.area() > 0.5*(*iter2).area()) {
				rectsUnions.push_back((*iter1) | (*iter2));		//dodajemy unie
			}

		}
	}

}



//learning

ImageOperations::ImagesErrors ImageOperations::loadVectorOfImagesToLearn(std::vector<std::string> paths) {

	learningImages.clear();

	if (paths.size() == 0)
		return ImageOperations::NO_PATH;

	for (decltype(paths.size()) i = 0; i < paths.size(); i++) {
		Mat next = imread(paths.at(i), IMREAD_GRAYSCALE);
		if (next.data == NULL) {
			return ImageOperations::OPENCV_ERROR;
		}
		
		learningImages.push_back(next);
	}

	return OK;
}

std::vector<Mat>& ImageOperations::getLearningImagesScaledTo(int width, int height) {

	for (decltype(learningImages.size()) j = 0; j < learningImages.size(); j++) {

		Mat tmp = Mat(height, width, CV_8U);
		tmp = cv::Scalar(128);	//taki szary pomiedzy czarnym i bia³ym ;)

		

		if ((double)learningImages[j].rows / learningImages[j].cols >(double)height / width) {
			//ciemne pasy po lewej i prawej
			double ratio = (double)height / learningImages[j].rows;
			Mat resized;
			int widthAfterResize = (int)(ratio*learningImages[j].cols);
			resize(learningImages[j], resized, Size(widthAfterResize, height));
			resized.copyTo(Mat(tmp, Rect((int)((width - widthAfterResize) / 2.0), 0, widthAfterResize, height)));
		}
		else {
			//a tu u góry i na dole
			double ratio = (double)width / learningImages[j].cols;
			Mat resized;
			int heightAfterResize = (int)(ratio*learningImages[j].rows);
			resize(learningImages[j], resized, Size(width, heightAfterResize));
			resized.copyTo(Mat(tmp, Rect(0, (int)((height - heightAfterResize) / 2.0), width, heightAfterResize)));
		}

		learningImages[j] = tmp;

	}

	return learningImages;

}



//dodawanko prostokatow, ustalanie prostokata jako auto, zaznaczanie prostokatow

void ImageOperations::addRectsWithOptions(int size, int numberOfBlurs, std::vector<int> threshes) {

	Mat meant = meanImageGrayscale.getMat(ACCESS_READ);

	for (decltype(loadedImages.size()) i = 0; i < loadedImages.size(); ++i) {
		UMat temp;
		UMat temp2;
		temp = imagesDifferences[i].clone();

		for (int j = 0; j < numberOfBlurs; j++)
			medianBlur(temp, temp, size);

		for (decltype(threshes.size()) k = 0; k < threshes.size(); k++) {
			cv::threshold(temp, temp2, threshes[k], 255, THRESH_BINARY);	//255 = bia³y
			Mat temp4 = temp.getMat(ACCESS_READ);
			Mat temp3 = temp2.getMat(ACCESS_READ);
			findContoursAddRects(temp2, vectorsOfRectsFound[i]);
		}
	}


}

void ImageOperations::markAllPossibleCars() {

	for (decltype(loadedImages.size()) i = 0; i < loadedImages.size(); ++i) {

		deleteDuplicatesAddUnions(vectorsOfRectsFound[i], vectorsOfRectsGeneratedByUnions[i]);

		//³¹czymy te pocz¹tkowo znalezione z tymi wygenerowanymi w jeden wektor
		vectorsOfRectsAll[i].reserve(vectorsOfRectsFound[i].size() + vectorsOfRectsGeneratedByUnions[i].size()); // preallocate memory
		vectorsOfRectsAll[i].insert(vectorsOfRectsAll[i].end(), vectorsOfRectsFound[i].begin(), vectorsOfRectsFound[i].end());
		vectorsOfRectsAll[i].insert(vectorsOfRectsAll[i].end(), vectorsOfRectsGeneratedByUnions[i].begin(), vectorsOfRectsGeneratedByUnions[i].end());


		Scalar color = Scalar(0, 140, 255);

		Mat mat = loadedImagesWithPossibleCars[i].getMat(ACCESS_RW);
		for (decltype(vectorsOfRectsAll[i].size()) j = 0; j < vectorsOfRectsAll[i].size(); j++) 
			rectangle(mat, vectorsOfRectsAll[i][j], color);		

	}

}

std::vector<std::vector<Mat>>& ImageOperations::getMatsScaledTo(int width, int height) {


	for (decltype(loadedImages.size()) i = 0; i < loadedImages.size(); ++i) {

		std::vector<Mat> toAdd;
		Mat noUMat = loadedImagesGrayscale[i].getMat(ACCESS_READ);

		for (decltype(vectorsOfRectsAll[i].size()) j = 0; j < vectorsOfRectsAll[i].size(); j++) {

			Mat tmp = Mat(height, width, CV_8U);
			tmp = cv::Scalar(128);	//taki szary pomiedzy czarnym i bia³ym ;)

			if ((double)vectorsOfRectsAll[i][j].height / vectorsOfRectsAll[i][j].width >(double)height / width) {
				//ciemne pasy po lewej i prawej
				double ratio = (double)height / vectorsOfRectsAll[i][j].height;
				Mat resized;
				int widthAfterResize = (int)(ratio*vectorsOfRectsAll[i][j].width);
				resize(Mat(noUMat, vectorsOfRectsAll[i][j]), resized, Size(widthAfterResize, height));
				resized.copyTo(Mat(tmp, Rect((int)((width - widthAfterResize) / 2.0), 0, widthAfterResize, height)));
			}
			else {
				//a tu u góry i na dole
				double ratio = (double)width / vectorsOfRectsAll[i][j].width;
				Mat resized;
				int heightAfterResize = (int)(ratio*vectorsOfRectsAll[i][j].height);
				resize(Mat(noUMat, vectorsOfRectsAll[i][j]), resized, Size(width, heightAfterResize));
				resized.copyTo(Mat(tmp, Rect(0, (int)((height - heightAfterResize) / 2.0), width, heightAfterResize)));
			}


			toAdd.push_back(tmp);
		}

		rectsChangedForAnn.push_back(toAdd);

		//przyda nam sie przy ustalaniu któe s¹ samochodami a które nie
		std::vector < uint > setAsCars;
		rectsSetAsCars.push_back(setAsCars);

	}

	return rectsChangedForAnn;
}

void ImageOperations::setRectAsCar(uint image, uint rect) {
	rectsSetAsCars[image].push_back(rect);
}

void ImageOperations::markRealCars() {

	Scalar color = Scalar(0, 255, 10);

	for (decltype(loadedImages.size()) i = 0; i < loadedImages.size(); ++i) {
		Mat mat = loadedImagesWithPossibleCars[i].getMat(ACCESS_RW);

		for (decltype(rectsSetAsCars[i].size()) j = 0; j < rectsSetAsCars[i].size(); j++) {
			rectangle(mat, vectorsOfRectsAll[i][rectsSetAsCars[i][j]], color);
		}

	}
}

std::vector<UMat>& ImageOperations::getLoadedImagesWithPossibleCars() {
	return loadedImagesWithPossibleCars;
}




