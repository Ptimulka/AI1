#include "ImageOperations.h"

ImageOperations::ImageOperations() {
	vectorOfImagesLoaded = false;
}


ImageOperations::~ImageOperations() {

}


ImageOperations::ImagesErrors ImageOperations::loadVectorOfImages(std::vector<std::string> paths) {

	if (paths.size() == 0)
		return ImageOperations::NO_PATH;

	std::vector<UMat> loadedImages;
	std::vector<UMat> loadedImagesGrayscale;

	for (decltype(paths.size()) i = 0; i < paths.size(); i++) {
		Mat next = imread(paths.at(i), IMREAD_COLOR);
		if (next.data == NULL) {
			vectorOfImagesLoaded = false;
			loadedImages.clear();
			loadedImagesGrayscale.clear();			
			return ImageOperations::OPENCV_ERROR;
		}

		if (i == 0)
			sizeOfImage = next.size();
		else if (next.size() != sizeOfImage) {
			vectorOfImagesLoaded = false;
			loadedImages.clear();
			loadedImagesGrayscale.clear();
			return ImageOperations::DIFFERENT_SIZES;
		}

		Mat nextGray = imread(paths.at(i), IMREAD_GRAYSCALE);
		loadedImages.push_back(next.getUMat(ACCESS_RW));
		loadedImagesGrayscale.push_back(nextGray.getUMat(ACCESS_RW));
	}

	vectorOfImagesLoaded = true;
	this->loadedImages.clear();
	this->loadedImagesGrayscale.clear();
	this->loadedImages = loadedImages;
	this->loadedImagesGrayscale = loadedImagesGrayscale;
	


	//stwarzamy od razu obrazek sredni
	createMeanImage();

	Mat jakas = meanImageGrayscale.getMat(ACCESS_RW);

	return ImageOperations::OK;
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


std::vector<UMat> ImageOperations::getLoadedImages() {
	return loadedImages;
}



bool ImageOperations::isVectorOfImagesLoaded() {
	return vectorOfImagesLoaded;
}



void ImageOperations::markCars(UMat & mat) {

	int minArea = 300;


	std::vector<Vec4i> hierarchy;
	std::vector<std::vector<Point> > contours;

	int thresh = 100;
	Canny(mat, mat, thresh, thresh * 2, 3);
	findContours(mat, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

	//wyrzucamy kontury o ma³ej d³ugoœci
	std::vector< std::vector<Point> >::iterator iter;
	for (iter = contours.begin(); iter != contours.end();) {
		if ((*iter).size() > 20)
			iter++;		
		else 
			iter = contours.erase(iter);
	}

	//tworzymy dla ka¿dego konturu prostok¹t i jeœli powierzchnia jest odpowiednia to zachowujemy ten kontur
	std::vector<Rect> rectangles;
	for (iter = contours.begin(); iter != contours.end();) {
		Rect rect = boundingRect(*iter);
		if (rect.size().area() > minArea) {
			iter++;
			rectangles.push_back(rect);
		}
		else {
			iter = contours.erase(iter);
		}
	}

	//dodajemy 
	vectorsOfRectsFound.push_back(rectangles);

	//generujemy takie bedace polaczeniem dwoch
	std::vector<Rect> rectanglesGenerated;
	bool *isAlreadyJoinedWithOther = new bool[contours.size()];
	for (decltype(contours.size()) i = 0; i < contours.size(); i++)
		isAlreadyJoinedWithOther[i] = false;

	for (decltype(contours.size()) i = 0; i < contours.size(); i++) {

		//³¹czymy z konturem innym który mo¿e byc czeœci¹ tego samego samochodu
		//if (!isAlreadyJoinedWithOther[i]) {
			for (decltype(contours.size()) j = 0; j < contours.size(); j++) {
				//if (i != j && !isAlreadyJoinedWithOther[i]) {
				if (i != j) {
					Rect rect3 = rectangles[i] & rectangles[j];
					//warunek pokrycia prostok¹tów
					if (rect3.area() > 0.5*rectangles[i].area() || rect3.area() > 0.5*rectangles[j].area()) {
						//juhuuu, pokrywaj¹ siê!
						Rect rect4 = rectangles[i] | rectangles[j];
						rectanglesGenerated.push_back(rect4);
						isAlreadyJoinedWithOther[i] = true;
						isAlreadyJoinedWithOther[j] = true;
						break;

					}

				}

			}


		//}

	}
	
	//dodajemy te stworzone z dwoch
	vectorsOfRectsGeneratedByJoining.push_back(rectanglesGenerated);


	return;
}



void ImageOperations::markCarsWithOptions(int size, int numberOfBlurs, int thresh, Scalar color) {

	UMat temp;
	for (decltype(loadedImages.size()) i = 0; i < loadedImages.size(); ++i) {
		temp = loadedImagesGrayscale[i].clone();
		absdiff(temp, meanImageGrayscale, temp);
		for (int j = 0; j < numberOfBlurs; j++)
			medianBlur(temp, temp, size);
		cv::threshold(temp, temp, thresh, 255, THRESH_BINARY);	//255 = bia³y
		markCars(temp);

		Mat mat = loadedImages.at(i).getMat(ACCESS_RW);
		for (int j = 0; j < vectorsOfRectsFound.back().size(); j++) {
			//rysujemy
			rectangle(mat, vectorsOfRectsFound.back()[j], color);
		}

		for (int j = 0; j < vectorsOfRectsGeneratedByJoining.back().size(); j++) {
			//rysujemy
			rectangle(mat, vectorsOfRectsGeneratedByJoining.back()[j], color - Scalar(50,50,50));
		}

	}


}



void ImageOperations::addToThresholdedWithOptions(int size, int numberOfBlurs, std::vector<int> threshes, Scalar color) {

	UMat temp;
	UMat temp2;
	for (decltype(loadedImages.size()) i = 0; i < loadedImages.size(); ++i) {
		temp = loadedImagesGrayscale[i].clone();
		absdiff(temp, meanImageGrayscale, temp);
		for (int j = 0; j < numberOfBlurs; j++)
			medianBlur(temp, temp, size);
		for (decltype(threshes.size()) k = 0; k < threshes.size(); k++) {
			cv::threshold(temp, temp2, threshes[k], 255, THRESH_BINARY);	//255 = bia³y
			vectorsOfThresholded[i].push_back(temp2.clone());


		}
	}



}

void ImageOperations::mixThresholded() {

	for (decltype(loadedImages.size()) i = 0; i < loadedImages.size(); ++i) {

		//dla kazdego obrazka robimy srednia thresholdow

		UMat meanImageThresholded = UMat::zeros(sizeOfImage, CV_32FC1);
		UMat convertedToFloat;

		decltype(vectorsOfThresholded[i].size()) sizeOfVector = vectorsOfThresholded[i].size();

		//dodajemy obrazki do siebie, musza byc floatami, bo inaczej unsigned char sie przekreci chyba
		for (decltype(sizeOfVector) j = 0; j < sizeOfVector; j++) {
			vectorsOfThresholded[i][j].convertTo(convertedToFloat, CV_32FC1);
			accumulate(convertedToFloat, meanImageThresholded);
		}

		//mamy juz sume wszystkich, trzeba tylko podzielic przez liczbe obrazkow
		UMat jedynki = UMat::ones(sizeOfImage, CV_32FC1);
		divide(meanImageThresholded, jedynki, meanImageThresholded, 1.0 / sizeOfVector);

		//musimy zamienic na nie float
		meanImageThresholded.convertTo(meanImageThresholded, CV_8U);

		vectorOfMeanThresholded.push_back(meanImageThresholded);

	}


}




void ImageOperations::tryTrickWithOpenCL() {

	if (!cv::ocl::haveOpenCL())
	{
		std::cout << "OpenCL is not avaiable..." << std::endl;
		return;
	}
	cv::ocl::Context context;
	if (!context.create(cv::ocl::Device::TYPE_ALL))
	{
		std::cout << "Failed creating the context..." << std::endl;
		return;
	}

	// In OpenCV 3.0.0 beta, only a single device is detected.
	std::cout << context.ndevices() << " GPU devices are detected." << std::endl;
	for (int i = 0; i < context.ndevices(); i++)
	{
		cv::ocl::Device device = context.device(i);
		std::cout << "name                 : " << device.name() << std::endl;
		std::cout << "available            : " << device.type() << std::endl;
		std::cout << "imageSupport         : " << device.imageSupport() << std::endl;
		std::cout << "OpenCL_C_Version     : " << device.OpenCL_C_Version() << std::endl;
		std::cout << std::endl;
	}

	return;
}



