#include "ImageOperations.h"

ImageOperations::ImageOperations(std::string referenceImagePath, std::vector<std::string> paths) {

	referenceImageLoaded = false;
	Mat referenceImageMat = imread(referenceImagePath, IMREAD_COLOR);
	if (referenceImageMat.data != NULL) {
		referenceImageLoaded = true;
		referenceImage = referenceImageMat.getUMat(ACCESS_RW);
		referenceImageGrayscale = imread(referenceImagePath, IMREAD_GRAYSCALE).getUMat(ACCESS_RW);
		recentOperation = referenceImage;
		recentOperationGrayscale = referenceImageGrayscale;
	}

	if (paths.size() == 0) {
		vectorOfImagesLoaded = false;
		return;
	}

	for (decltype(paths.size()) i = 0; i < paths.size(); i++) {
		Mat next = imread(paths.at(i), IMREAD_COLOR);
		if (next.data == NULL) {
			vectorOfImagesLoaded = false;
			loadedImages.clear();
			loadedImagesGrayscale.clear();
			return;
		}
		Mat nextGray = imread(paths.at(i), IMREAD_GRAYSCALE);
		loadedImages.push_back(next.getUMat(ACCESS_RW));
		loadedImagesGrayscale.push_back(nextGray.getUMat(ACCESS_RW));
	}

	for (decltype(loadedImages.size()) i = 0; i < loadedImages.size(); i++) {
		recentOperationOnVector.push_back(loadedImages.at(i).clone());
		recentOperationOnVectorGrayscale.push_back(loadedImagesGrayscale.at(i).clone());
	}

	vectorOfImagesLoaded = true;
}


ImageOperations::ImageOperations() {
	referenceImageLoaded = false;
	vectorOfImagesLoaded = false;
}


ImageOperations::~ImageOperations() {

}



bool ImageOperations::loadReferenceImage(std::string referenceImagePath) {

	Mat referenceImageMat = imread(referenceImagePath, IMREAD_COLOR);
	if (referenceImageMat.data != NULL) {
		this->referenceImage = referenceImage;
		referenceImageLoaded = true;
		referenceImageGrayscale = imread(referenceImagePath, IMREAD_GRAYSCALE).getUMat(ACCESS_RW);
		referenceImage = referenceImageMat.getUMat(ACCESS_RW);
		recentOperation = UMat(referenceImage);
		recentOperationGrayscale = UMat(referenceImageGrayscale);
		return true;
	}

	return false;
}


bool  ImageOperations::loadVectorOfImages(std::vector<std::string> paths) {

	if (paths.size() == 0)
		return false;

	std::vector<UMat> loadedImages;
	std::vector<UMat> loadedImagesGrayscale;

	for (decltype(paths.size()) i = 0; i < paths.size(); i++) {
		Mat next = imread(paths.at(i), IMREAD_COLOR);
		if (next.data == NULL) {
			loadedImages.clear();
			loadedImagesGrayscale.clear();
			return false;
		}
		Mat nextGray = imread(paths.at(i), IMREAD_GRAYSCALE);
		loadedImages.push_back(next.getUMat(ACCESS_RW));
		loadedImagesGrayscale.push_back(nextGray.getUMat(ACCESS_RW));
	}

	vectorOfImagesLoaded = true;
	this->loadedImages = loadedImages;
	this->loadedImagesGrayscale = loadedImagesGrayscale;

	recentOperationOnVector.clear();
	recentOperationOnVectorGrayscale.clear();
	for (decltype(loadedImages.size()) i = 0; i < loadedImages.size(); i++) {
		recentOperationOnVector.push_back(loadedImages.at(i).clone());
		recentOperationOnVectorGrayscale.push_back(loadedImagesGrayscale.at(i).clone());
	}
	return true;
}


bool ImageOperations::areImagesLoaded() {
	return (referenceImageLoaded && vectorOfImagesLoaded);
}


bool ImageOperations::isReferenceImageLoaded() {
	return referenceImageLoaded;
}


bool ImageOperations::isVectorOfImagesLoaded() {
	return vectorOfImagesLoaded;
}


bool ImageOperations::isWhatShouldBeLoaded(makeOperationOn makeOn) {
	return !((makeOn == ALL && !areImagesLoaded()) || (makeOn == VECTOR_OF_IMAGES && !isVectorOfImagesLoaded()) || (makeOn == REFERNCE_IMAGE && !isReferenceImageLoaded()));
}


void ImageOperations::pushRecentOperationOnVector() {
	savedOperationsOnVector.push_back(recentOperationOnVector);
	savedOperationsOnVectorGrayscale.push_back(recentOperationOnVectorGrayscale);
}


void ImageOperations::pushRecentOperationOnReferenceImage() {
	savedOperationsOnReferenceImage.push_back(recentOperation);
	savedOperationsOnReferenceImageGrayscale.push_back(recentOperationGrayscale);
}


std::vector<UMat> ImageOperations::getRecentOperationOnVector(bool inColor) {
	if (inColor)
		return recentOperationOnVector;
	return recentOperationOnVectorGrayscale;
}


UMat ImageOperations::getRecentOperationOnReferenceImage(bool inColor) {
	if (inColor)
		return recentOperation;
	return recentOperationGrayscale;
}


bool ImageOperations::imagesDifference(bool useUserReferenceImage, bool doItWiselyButLong) {

	if ((useUserReferenceImage && !areImagesLoaded()) || (!useUserReferenceImage && !isVectorOfImagesLoaded()))
		return false;

	decltype(recentOperationOnVector.size()) sizeOfVector = recentOperationOnVector.size();


	if (useUserReferenceImage) {

		for (decltype(sizeOfVector) i = 0; i < sizeOfVector; i++) {
			absdiff(recentOperationOnVector.at(i), referenceImage, recentOperationOnVector.at(i));
			absdiff(recentOperationOnVectorGrayscale.at(i), referenceImageGrayscale, recentOperationOnVectorGrayscale.at(i));
		}
	}
	else {


		UMat mean = UMat::zeros(recentOperationOnVector.at(0).rows, recentOperationOnVector.at(0).cols, CV_32FC3);
		UMat meanGrayscale = UMat::zeros(recentOperationOnVector.at(0).rows, recentOperationOnVector.at(0).cols, CV_32FC1);
		UMat convertedToFloat;

		for (decltype(sizeOfVector) i = 0; i < sizeOfVector; i++) {
			recentOperationOnVector.at(i).convertTo(convertedToFloat, CV_32FC3);	//
			accumulate(convertedToFloat, mean);
			recentOperationOnVectorGrayscale.at(i).convertTo(convertedToFloat, CV_32FC1);
			accumulate(convertedToFloat, meanGrayscale);
		}


		//divide() - ???
		//mean = ((Mat)(mean.getMat(ACCESS_RW) / sizeOfVector)).getUMat(ACCESS_RW);	// niedziala, assert cos z pamiecia, referencja czy cos
		UMat jedynki = UMat::ones(mean.size(), CV_32FC3);
		divide(mean, jedynki, mean, 1.0 / sizeOfVector);
		mean.convertTo(mean, CV_8U);

		UMat jedynki2 = UMat::ones(mean.size(), CV_32FC1);
		//jedynki.convertTo(jedynki, CV_32FC1);
		divide(meanGrayscale, jedynki2, meanGrayscale, 1.0 / sizeOfVector);
		//meanGrayscale = ((Mat)(meanGrayscale.getMat(ACCESS_RW) / sizeOfVector)).getUMat(ACCESS_RW);
		meanGrayscale.convertTo(meanGrayscale, CV_8U);


		Mat meanGrayscaleMat = meanGrayscale.getMat(ACCESS_RW);


		std::vector < Mat > maty;
		for (decltype(sizeOfVector) i = 0; i < sizeOfVector; i++) {
			maty.push_back(recentOperationOnVectorGrayscale.at(i).getMat(ACCESS_RW));
		}



		if (doItWiselyButLong) {		//niech spadaj¹ te z podejrzanych odchyleniem

			for (int y = 0; y < meanGrayscale.rows; y++) {
				for (int x = 0; x < meanGrayscale.cols; x++) {

					//dla grayscale 
					float standardDeviation = 0;
					float average = (float)(meanGrayscaleMat.at<unsigned char>(cv::Point(x, y)));

					for (decltype(sizeOfVector) i = 0; i < sizeOfVector; i++)
						standardDeviation += abs((float)(maty.at(i).at<unsigned char>(cv::Point(x, y))) - average);

					standardDeviation /= sizeOfVector;

					int notSuspected = sizeOfVector;

					float averageChanged = average;
					for (decltype(sizeOfVector) i = 0; i < sizeOfVector; i++) {

						if (abs((float)(maty.at(i).at<unsigned char>(cv::Point(x, y))) - average) > standardDeviation) {
							averageChanged = averageChanged - ((float)(recentOperationOnVectorGrayscale.at(i).getMat(ACCESS_RW).at<unsigned char>(cv::Point(x, y)))) / notSuspected;
							averageChanged *= ((float)notSuspected);
							notSuspected--;
							//raczej nie bêdzie asserta, bo to niemo¿liwe, ¿e wszystkie bêdê mia³y odchylenie wiêksze ni¿ standardowe
							assert(notSuspected != 0);
							averageChanged /= ((float)notSuspected);
						}

					}

					meanGrayscaleMat.at<unsigned char>(cv::Point(x, y)) = (unsigned char)averageChanged;

					//dla kolorowego

					/*Vec3f average3 = mean.at<Vec3b>(Point(x, y));
					Vec3f standardDeviation3 = Vec3f(0, 0, 0);

					for (decltype(sizeOfVector) i = 0; i < sizeOfVector; i++) {
					Vec3f dif = ((Vec3f)(recentOperationOnVectorGrayscale.at(i).at<Vec3b>(cv::Point(x, y))) - average3);
					standardDeviation3[0] += abs(dif[0]);
					standardDeviation3[1] += abs(dif[1]);
					standardDeviation3[2] += abs(dif[2]);
					}

					standardDeviation3 = standardDeviation3 / sizeOfVector;
					notSuspected = sizeOfVector;*/

				}
			}

		}

		//recentOperationOnVectorGrayscale.clear();
		//for (decltype(sizeOfVector) i = 0; i < sizeOfVector; i++) {
		//	recentOperationOnVectorGrayscale.push_back(maty.at(i).getUMat(ACCESS_RW));
		//	//maty.at(i).copyTo(recentOperationOnVectorGrayscale.at(i));
		//}

		maty.clear();
		meanGrayscaleMat.release();

		for (decltype(recentOperationOnVector.size()) i = 0; i < recentOperationOnVector.size(); i++) {
			absdiff(recentOperationOnVectorGrayscale.at(i), meanGrayscale, recentOperationOnVectorGrayscale.at(i));
		}

	}

	return true;
}


bool ImageOperations::detectEdges(makeOperationOn makeOn) {

	if (!isWhatShouldBeLoaded(makeOn))
		return false;

	if (makeOn == ALL || makeOn == VECTOR_OF_IMAGES) {

		for (decltype(recentOperationOnVector.size()) i = 0; i < recentOperationOnVector.size(); i++) {
			Canny(recentOperationOnVector.at(i), recentOperationOnVector.at(i), 10, 30, 3);
			Canny(recentOperationOnVectorGrayscale.at(i), recentOperationOnVectorGrayscale.at(i), 10, 30, 3);
		}
	}

	if (makeOn == ALL || makeOn == REFERNCE_IMAGE) {
		Canny(recentOperation, recentOperation, 10, 30, 3);
		Canny(recentOperationGrayscale, recentOperationGrayscale, 10, 30, 3);
	}
	return true;
}


bool ImageOperations::blurImages(makeOperationOn makeOn, int size) {

	if (!isWhatShouldBeLoaded(makeOn))
		return false;

	if (makeOn == ALL || makeOn == VECTOR_OF_IMAGES) {

		for (decltype(recentOperationOnVector.size()) i = 0; i < recentOperationOnVector.size(); i++) {
			blur(recentOperationOnVector.at(i), recentOperationOnVector.at(i), Size(size, size));
			blur(recentOperationOnVectorGrayscale.at(i), recentOperationOnVectorGrayscale.at(i), Size(size, size));
		}
	}

	if (makeOn == ALL || makeOn == REFERNCE_IMAGE) {
		blur(recentOperation, recentOperation, Size(size, size));
		blur(recentOperationGrayscale, recentOperationGrayscale, Size(size, size));
	}
	return true;
}


bool ImageOperations::medianFiltr(makeOperationOn makeOn, int size) {

	if (!isWhatShouldBeLoaded(makeOn))
		return false;

	if (makeOn == ALL || makeOn == VECTOR_OF_IMAGES) {

		for (decltype(recentOperationOnVector.size()) i = 0; i < recentOperationOnVector.size(); i++) {
			medianBlur(recentOperationOnVector.at(i), recentOperationOnVector.at(i), size);
			medianBlur(recentOperationOnVectorGrayscale.at(i), recentOperationOnVectorGrayscale.at(i), size);
		}
	}

	if (makeOn == ALL || makeOn == REFERNCE_IMAGE) {
		medianBlur(recentOperation, recentOperation, size);
		medianBlur(recentOperationGrayscale, recentOperationGrayscale, size);
	}
	return true;
}


bool ImageOperations::threshold(int thresh, makeOperationOn makeOn) {

	if (!isWhatShouldBeLoaded(makeOn))
		return false;

	if (makeOn == ALL || makeOn == VECTOR_OF_IMAGES) {

		for (decltype(recentOperationOnVector.size()) i = 0; i < recentOperationOnVector.size(); i++) {
			cv::threshold(recentOperationOnVectorGrayscale.at(i), recentOperationOnVectorGrayscale.at(i), thresh, 255, THRESH_BINARY);	//255 = bia³y
		}
	}

	if (makeOn == ALL || makeOn == REFERNCE_IMAGE) {
		cv::threshold(recentOperation, recentOperation, thresh, 255, THRESH_BINARY);
	}
	return true;

}



bool ImageOperations::isContourOk(std::vector<Point> contour, int sizeMin) {

	if (contour.size() < 20)
		return false;
	return true;

}


UMat ImageOperations::markCars(int whichImage, unsigned char red, unsigned char green, unsigned char blue) {

	int minArea = 300;

	if (!isVectorOfImagesLoaded() || !(recentOperationOnVectorGrayscale.size() > whichImage))
		return UMat();

	if (loadedImagesGrayscale.size() <= whichImage)
		return UMat();

	std::vector<Vec4i> hierarchy;
	std::vector<std::vector<Point> > contours;
	int thresh = 100;
	Canny(recentOperationOnVectorGrayscale.at(whichImage), recentOperationOnVectorGrayscale.at(whichImage), thresh, thresh * 2, 3);

	findContours(recentOperationOnVectorGrayscale.at(whichImage), contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

	//wyrzucamy kontury o ma³ej d³ugoœci
	std::vector< std::vector<Point> >::iterator iter;
	for (iter = contours.begin(); iter != contours.end();) {
		if (isContourOk(*iter)) {
			iter++;
		}
		else {
			iter = contours.erase(iter);
		}
	}

	//tworzymy dla ka¿dego konturu prostok¹t i jeœli powierzchnia jest odpowiednia to zachowujemy ten kontur
	std::vector<Rect> rectangles;
	for (iter = contours.begin(); iter != contours.end();) {
		//Rect rect = Rect(Point(minsmaxs.minX, minsmaxs.minY), Point(minsmaxs.maxX, minsmaxs.maxY));
		Rect rect = boundingRect(*iter);
		if (rect.size().area() > minArea) {
			iter++;
			rectangles.push_back(rect);
		}
		else {
			iter = contours.erase(iter);
		}
	}


	bool *isAlreadyJoinedWithOther = new bool[contours.size()];
	for (decltype(contours.size()) i = 0; i < contours.size(); i++)
		isAlreadyJoinedWithOther[i] = false;
	std::vector<UMat> samochody;

	for (decltype(contours.size()) i = 0; i < contours.size(); i++) {

		auto img = loadedImagesGrayscale.at(whichImage)(rectangles[i]);
		rectangle(loadedImages.at(whichImage).getMat(ACCESS_RW), rectangles[i], Scalar(red, green, blue));

		//³¹czymy z konturem innym który mo¿e byc czeœci¹ tego samego samochodu
		if (!isAlreadyJoinedWithOther[i]) {
			for (decltype(contours.size()) j = 0; j < contours.size(); j++) {
				if (i != j && !isAlreadyJoinedWithOther[i]) {
					Rect rect3 = rectangles[i] & rectangles[j];
					//warunek pokrycia prostok¹tów
					if (rect3.area() > 0.5*rectangles[i].area() && rect3.area() > 0.5*rectangles[3].area()) {
						//juhuuu, pokrywaj¹ siê!
						Rect rect4 = rectangles[i] | rectangles[j];
						rectangle(loadedImages.at(whichImage).getMat(ACCESS_RW), rect4, Scalar(10));
						isAlreadyJoinedWithOther[i] = true;
						isAlreadyJoinedWithOther[j] = true;
						break;

					}

				}

			}


		}
		samochody.push_back(std::move(img));

	}

	//RNG rng(12345);	//generator losowych liczb 
	//for (int i = 0; i< contours.size(); i++) {
	//	Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
	//	drawContours(loadedImagesGrayscale.at(whichImage), contours, i, color, 2, 8, hierarchy, 0, Point());
	//}

	return loadedImages.at(whichImage);
}

std::vector<UMat> ImageOperations::markCars(unsigned char red, unsigned char green, unsigned char blue)
{
	std::vector<UMat> ret;
	for (int i = 0; i < loadedImages.size(); ++i)
		ret.push_back(markCars(i, red, green, blue));

	return ret;
}


void ImageOperations::robCosZebyZajacGPU() {

	int size = 9;
	for (int j = 0; j < loadedImages.size(); ++j){

		/*for (int i = 0; i < 100; i++) {
			medianBlur(recentOperationOnVector.at(j), recentOperationOnVector.at(j), size);
			blur(recentOperationOnVector.at(j), recentOperationOnVector.at(j), Size(size, size));
		}*/
		
		/*for (int i = 0; i < 100; i++)
			addWeighted(recentOperationOnVector.at(j), 0.2, recentOperationOnVector.at(j), 0.5, 0.1, recentOperationOnVector.at(j));
		*/
		for (int i = 0; i < 3000; i++)
			Sobel(recentOperationOnVectorGrayscale.at(j), recentOperationOnVectorGrayscale.at(j), CV_16S, 1, 0);

		/*for (int i = 0; i < 100; i++)
			multiply(recentOperationOnVector.at(j), recentOperationOnVector.at(j), recentOperationOnVector.at(j), 0.5);
		*/
	}



}