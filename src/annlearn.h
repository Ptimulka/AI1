#include <iostream>
#include <fstream>
#include <vector>
#include <regex>

#include "ImageOperations.h"
#include "cmdopt.h"
#include "utils/log.h"
#include "utils/cmdline.h"
#include "utils/strutils.h"
#include "io/dir.h"
#include "ann/cl/oclkernel.h"
#include "ann/fann_irprop_logical.h"
#include "ann/ann.h"

using namespace std;

void ann_learn(uint iw, uint ih, const char *filename, uint nodes_in_hidden1, uint nodes_in_hidden2 = 0);

void ann_test_learning_images(uint iw, uint ih, const char *filename, float thresh);