#include "ImageOperations.h"

ImageOperations::ImageOperations(std::string referenceImagePath, std::vector<std::string> paths) {

	referenceImageLoaded = false;
	referenceImage = imread(referenceImagePath, IMREAD_COLOR);
	if (referenceImage.data != NULL) {
		referenceImageLoaded = true;
		referenceImageGrayscale = imread(referenceImagePath, IMREAD_GRAYSCALE);
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
		loadedImages.push_back(next);
		loadedImagesGrayscale.push_back(nextGray);
	}
	recentOperationOnVector = loadedImages;
	recentOperationOnVectorGrayscale = loadedImagesGrayscale;
	vectorOfImagesLoaded = true;
}


ImageOperations::ImageOperations() {
	referenceImageLoaded = false;
	vectorOfImagesLoaded = false;
}


ImageOperations::~ImageOperations() {

}



bool ImageOperations::loadReferenceImage(std::string referenceImagePath) {

	Mat referenceImage = imread(referenceImagePath, IMREAD_COLOR);
	if (referenceImage.data != NULL) {
		this->referenceImage = referenceImage;
		referenceImageLoaded = true;
		referenceImageGrayscale = imread(referenceImagePath, IMREAD_GRAYSCALE);
		recentOperation = referenceImage;
		recentOperationGrayscale = referenceImageGrayscale;
		return true;
	}

	return false;
}


bool  ImageOperations::loadVectorOfImages(std::vector<std::string> paths) {

	if (paths.size() == 0)
		return false;

	std::vector<Mat> loadedImages;
	std::vector<Mat> loadedImagesGrayscale;

	for (decltype(paths.size()) i = 0; i < paths.size(); i++) {
		Mat next = imread(paths.at(i), IMREAD_COLOR);
		if (next.data == NULL) {
			loadedImages.clear();
			loadedImagesGrayscale.clear();
			return false;
		}
		Mat nextGray = imread(paths.at(i), IMREAD_GRAYSCALE);
		loadedImages.push_back(next);
		loadedImagesGrayscale.push_back(nextGray);
	}

	vectorOfImagesLoaded = true;
	this->loadedImages = loadedImages;
	this->loadedImagesGrayscale = loadedImagesGrayscale;
	recentOperationOnVector = loadedImages;
	recentOperationOnVectorGrayscale = loadedImagesGrayscale;
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


std::vector<Mat> ImageOperations::getRecentOperationOnVector(bool inColor) {
	if (inColor)
		return recentOperationOnVector;
	return recentOperationOnVectorGrayscale;
}


Mat ImageOperations::getRecentOperationOnReferenceImage(bool inColor) {
	if (inColor)
		return recentOperation;
	return recentOperationGrayscale;
}

bool ImageOperations::imagesDifference() {

	if (!areImagesLoaded())
		return false;

	for (decltype(recentOperationOnVector.size()) i = 0; i < recentOperationOnVector.size(); i++) {
		absdiff(recentOperationOnVector.at(i), referenceImage, recentOperationOnVector.at(i));
		absdiff(recentOperationOnVectorGrayscale.at(i), referenceImageGrayscale, recentOperationOnVectorGrayscale.at(i));
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


bool ImageOperations::threshold(makeOperationOn makeOn) {

	if (!isWhatShouldBeLoaded(makeOn))
		return false;

	if (makeOn == ALL || makeOn == VECTOR_OF_IMAGES) {

		for (decltype(recentOperationOnVector.size()) i = 0; i < recentOperationOnVector.size(); i++) {
			cv::threshold(recentOperationOnVectorGrayscale.at(i), recentOperationOnVectorGrayscale.at(i), 50, 255, THRESH_BINARY);	//255 = bia³y
		}
	}

	if (makeOn == ALL || makeOn == REFERNCE_IMAGE) {
		cv::threshold(recentOperation, recentOperation, 50, 200, THRESH_BINARY);
	}
	return true;

}