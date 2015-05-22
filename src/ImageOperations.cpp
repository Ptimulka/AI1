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

	//oraz obrazki-ró¿nice
	for (decltype(loadedImages.size()) i = 0; i < loadedImages.size(); ++i) {
		UMat next = loadedImagesGrayscale[i].clone();
		absdiff(next, meanImageGrayscale, next);
		imagesDifferences.push_back(next);

		//tworzymy tez wektory na prostokaty
		vectorsOfRectsFound.push_back(std::vector<Rect>());
		vectorsOfRectsGeneratedByUnions.push_back(std::vector<Rect>());

	}

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





void ImageOperations::addRectsWithOptions(int size, int numberOfBlurs, std::vector<int> threshes) {

	
	for (decltype(loadedImages.size()) i = 0; i < loadedImages.size(); ++i) {
		UMat temp;
		UMat temp2;
		temp = imagesDifferences[i].clone();

		for (int j = 0; j < numberOfBlurs; j++)
			medianBlur(temp, temp, size);

		for (decltype(threshes.size()) k = 0; k < threshes.size(); k++) {
			cv::threshold(temp, temp2, threshes[k], 255, THRESH_BINARY);	//255 = bia³y
			findContoursAddRects(temp2, vectorsOfRectsFound[i]);
		}
	}


}


void ImageOperations::markAllPossibleCars() {

	for (decltype(loadedImages.size()) i = 0; i < loadedImages.size(); ++i) {

		deleteDuplicatesAddUnions(vectorsOfRectsFound[i], vectorsOfRectsGeneratedByUnions[i]);


		Mat mat = loadedImages[i].getMat(ACCESS_RW);
		for (decltype(vectorsOfRectsFound[i].size()) j = 0; j < vectorsOfRectsFound[i].size(); j++) 
			rectangle(mat, vectorsOfRectsFound[i][j], Scalar(rand() % 256, rand() % 256, rand() % 256));		

		for (decltype(vectorsOfRectsGeneratedByUnions[i].size()) j = 0; j < vectorsOfRectsGeneratedByUnions[i].size(); j++) 
			rectangle(mat, vectorsOfRectsGeneratedByUnions[i][j], Scalar(rand() % 256, rand() % 256, rand() % 256));		

		//std::vector<Rect> vect;
		//vect.reserve(vectorsOfRectsFound[i].size() + vectorsOfRectsGeneratedByUnions[i].size()); // preallocate memory
		//vect.insert(vect.end(), vectorsOfRectsFound[i].begin(), vectorsOfRectsFound[i].end());
		//vect.insert(vect.end(), vectorsOfRectsGeneratedByUnions[i].begin(), vectorsOfRectsGeneratedByUnions[i].end());

	}

}



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
		if (rect.area() > minArea && rect.area() < 0.25*sizeOfImage.area() && rect.width < 0.5*sizeOfImage.width && rect.height < 0.5*sizeOfImage.height) 
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


std::vector<std::vector<Mat>> ImageOperations::getMatsScaledTo(int width, int height) {

	std::vector<std::vector<Mat>> toReturn;

	for (decltype(loadedImages.size()) i = 0; i < loadedImages.size(); ++i) {

		std::vector<Mat> toAdd;
		Mat noUMat = loadedImagesGrayscale[i].getMat(ACCESS_READ);

		for (decltype(vectorsOfRectsFound[i].size()) j = 0; j < vectorsOfRectsFound[i].size(); j++) {

			Mat tmp = Mat(height, width, CV_8U);
			tmp = cv::Scalar(128);	//taki szary pomiedzy czarnym i bia³ym ;)

			if ((double)vectorsOfRectsFound[i][j].height / vectorsOfRectsFound[i][j].width > (double)height / width) {
				//ciemne pasy po lewej i prawej
				double ratio = (double)height / vectorsOfRectsFound[i][j].height;
				Mat resized;
				int widthAfterResize = ratio*vectorsOfRectsFound[i][j].width;
				resize(Mat(noUMat, vectorsOfRectsFound[i][j]), resized, Size(widthAfterResize, height));
				resized.copyTo(Mat(tmp, Rect((width - widthAfterResize) / 2.0, 0, widthAfterResize, height)));
			}
			else {

				double ratio = (double)width / vectorsOfRectsFound[i][j].width;
				Mat resized;
				int heightAfterResize = ratio*vectorsOfRectsFound[i][j].height;
				resize(Mat(noUMat, vectorsOfRectsFound[i][j]), resized, Size(width, heightAfterResize));
				resized.copyTo(Mat(tmp, Rect(0, (height - heightAfterResize) / 2.0, width, heightAfterResize)));
			}


			toAdd.push_back(tmp);
		}

		toReturn.push_back(toAdd);


	}

	return toReturn;
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



