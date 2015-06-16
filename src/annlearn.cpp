#include "annlearn.h"

void ann_learn(uint iw, uint ih, const char *filename, uint nodes_in_hidden1, uint nodes_in_hidden2) {


	const unsigned bytesPerPixel = sizeof(float);
	const unsigned bytesPerCase = iw * ih * bytesPerPixel;
	if (Opts::ann_learn_chunk_size < bytesPerCase)
	{
		sLog.log("Error: one trainging chunk isn't capable of holding one input image!");				
	}

	const unsigned casesPerRun = Opts::ann_learn_chunk_size / bytesPerCase;

	ArtificialNeuralNetwork ann;
	if (nodes_in_hidden2 == 0) 
		ann = ArtificialNeuralNetwork({ iw*ih, nodes_in_hidden1, 1 });	
	else 
		ann = ArtificialNeuralNetwork({ iw*ih, nodes_in_hidden1, nodes_in_hidden2, 1 });

	ann.init<ArtificialNeuralNetwork::FannDriver>(casesPerRun);

	Dir imgs_dir(convert<string, wstring>(Opts::imgs_learn));
	std::vector<string> pathsPos;
	std::vector<string> pathsNeg;

	for (auto image : imgs_dir.getEntries(L"pos.*"))
		pathsPos.push_back(convert<wstring, string>(image));

	for (auto image : imgs_dir.getEntries(L"neg.*"))
		pathsNeg.push_back(convert<wstring, string>(image));



	ImageOperations op;

	std::vector<float> array;
	array.reserve(Opts::ann_learn_chunk_size);
	std::vector<float> results;

	op.loadVectorOfImagesToLearn(pathsPos);
	std::vector<Mat> learn_pos = op.getLearningImagesScaledTo(iw, ih);	//szerokosæ, wysokoœæ
	op.loadVectorOfImagesToLearn(pathsNeg);
	std::vector<Mat> learn_neg = op.getLearningImagesScaledTo(iw, ih);

	vector<pair<bool, uint>> tests;
	for (uint i = 0; i < learn_pos.size(); ++i)
		tests.push_back(make_pair(true, i));
	for (uint i = 0; i < learn_neg.size(); ++i)
		tests.push_back(make_pair(false, i));
	std::random_shuffle(tests.begin(), tests.end());

	results.reserve(casesPerRun);

	for (uint i = 0; i < tests.size(); ++i)
	{
		int j = 0;
		for (; j < casesPerRun && i < tests.size(); ++j, ++i)
			if (tests[i].first)
			{
				array.insert(array.end(), learn_pos[tests[i].second].datastart, learn_pos[tests[i].second].dataend);
				results.push_back(1.0f);
			}
			else
			{
				array.insert(array.end(), learn_neg[tests[i].second].datastart, learn_neg[tests[i].second].dataend);
				results.push_back(0.0f);
			}

		ann.learn(array, results);
		array.clear();
		results.clear();
	}

	ann.saveNative(filename);

}

void ann_test_learning_images(uint iw, uint ih, const char *filename, float thresh, string imgsForLearning) {

	Dir imgs_dir(convert<string, wstring>(imgsForLearning));
	std::vector<string> pathsPos;
	std::vector<string> pathsNeg;

	for (auto image : imgs_dir.getEntries(L"pos.*"))
		pathsPos.push_back(convert<wstring, string>(image));

	for (auto image : imgs_dir.getEntries(L"neg.*"))
		pathsNeg.push_back(convert<wstring, string>(image));


	ImageOperations op;
	op.loadVectorOfImagesToLearn(pathsPos);
	std::vector<Mat> learn_pos = op.getLearningImagesScaledTo(iw, ih);
	op.loadVectorOfImagesToLearn(pathsNeg);
	std::vector<Mat> learn_neg = op.getLearningImagesScaledTo(iw, ih);

	ArtificialNeuralNetwork ann;
	ann.init<ArtificialNeuralNetwork::FannDriver>(filename, 0u);

	std::vector<float> results_pos;
	std::vector<float> results_neg;

	uint pos_gut = 0, neg_gut = 0;

	for (uint i = 0; i < learn_pos.size(); i++) {
		std::vector < float > input_array;
		input_array.assign(learn_pos[i].datastart, learn_pos[i].dataend);
		float answer = ann.run(input_array).front();
		results_pos.push_back(answer);
		if (answer >= thresh)
			pos_gut++;
	}
	sLog.log("Dla obrazkow pozytywnych siec wskazala dobrze dla ", pos_gut, "/" , pathsPos.size(), " obrazkow");


	for (uint i = 0; i < learn_neg.size(); i++) {
		std::vector < float > input_array;
		input_array.assign(learn_neg[i].datastart, learn_neg[i].dataend);
		float answer = ann.run(input_array).front();
		results_neg.push_back(answer);
		if (answer < thresh)
			neg_gut++;
	}
	sLog.log("Dla obrazkow negatywnych siec wskazala dobrze dla ", neg_gut, "/" , pathsNeg.size(), " obrazkow");

}