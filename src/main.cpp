#include <iostream>
#include <vector>

#include "ImageOperations.h"

using namespace cv;
using namespace std;


int main(int argc, char** argv)
{

	std::vector<std::string> imageNames;

	imageNames.push_back("zdj000001.jpg");
	imageNames.push_back("zdj000002.jpg");
	imageNames.push_back("zdj000003.jpg");
	imageNames.push_back("zdj000004.jpg");
	imageNames.push_back("zdj000005.jpg");
	imageNames.push_back("zdj000006.jpg");

	/*
	imageNames.push_back("zdj000007.jpg");
	imageNames.push_back("zdj000008.jpg");
	imageNames.push_back("zdj000009.jpg");
	imageNames.push_back("zdj000010.jpg");
	imageNames.push_back("zdj000011.jpg");
	imageNames.push_back("zdj000012.jpg");
	imageNames.push_back("zdj000013.jpg");
	*/

	ImageOperations op;

	op.loadReferenceImage("zdj0.jpg");
	op.loadVectorOfImages(imageNames);

	op.medianFiltr();
	op.imagesDifference();
	//op.detectEdges();

	std::vector<Mat> mats = op.getRecentOperationOnVector(false);

	for (int i = 0; i < mats.size(); i++) {
		namedWindow("window" + i, WINDOW_AUTOSIZE);
		imshow("window" + i, mats.at(i));
	}

	waitKey();//without this image won't be shown


	return 0;
}