#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include "dirent.h"
#include <cstring>
#include "json.hpp"
#include <fstream>
//#include <thread>
using namespace std;
using namespace cv;
using json = nlohmann::json;
/**
* @function main
*/

json histogramGray(string path,json& output);
struct phase
{
	float phase1 = 0.2;
	float phase2 = 0.3;
	float phase3 = 0.2;
	float phase4 = 0.15;
	bool phase2reached = false;
	int whichPhase(float middleColor)
	{
		int ret = 1;
		if (!phase2reached)
		{
			if (middleColor > phase2)
			{
				phase2reached = true;
				return 2;
			}
		}
		else
		{
			if (middleColor > phase3)
			{
				return 3;
			}
			else
			{
				return 4;
			}
		}
		return ret;
	}
}p;
bool showPictures;
int main(int argc,char** argv)
{
	DIR *dir;
	showPictures = false;
	p.phase2reached= false;
	struct dirent *ent;
	string path;
	if (argc < 2)
	{
		cout << "eyeAnalyser[path] [pic(to show processed pictures)]";
		//path = "C:\\oko\\dane_testowe1\\D_0ccf910f7fafc450a7c23785278eec2a\\2016-03-03";
	}
	else
	{
		path = argv[1];
	}
	if (argc==3)
	{
		cout << argv[2] << endl;
		if (string("pic") .compare(argv[2])==0)
		{
			
			showPictures = true;
		}
	}
	
	
	if ((dir = opendir(path.c_str())) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
			if (ent->d_name)
			{
				
				int l = ent->d_namlen;
				if (ent->d_name[l-1]=='g' && ent->d_name[l-2] == 'p' && ent->d_name[l - 3] == 'j')
				{
					std::ofstream myfile;
					myfile.open(path+"\\description."+std::string(ent->d_name)+".json");
					json j;
					j["name"] = ent->d_name;
					cout<< ent->d_name<<"\n";
					string img_path = path + string("\\") + string(ent->d_name);
					
					cout << img_path << "\n";
					histogramGray(img_path,j);
					myfile<<j.dump();
					myfile.close();

				}
			}

		}
		closedir(dir);
	}
	else {
		/* could not open directory */
		perror("");
		return EXIT_FAILURE;
	}
} 
json histogramGray(string path,json& output)
{

	Mat src, dst;
	/// Load image
	src = imread(path.c_str(), 1);
	std::vector<float> ret;
	Size size= src.size();

	/*if (!src.data)
	{
		return null;
	}
	*/

	cvtColor(src, src, cv::COLOR_RGB2GRAY);


	/// Establish the number of bins
	int histSize = 256;

	/// Set the ranges ( for B,G,R) )
	float range[] = { 0, 256 };
	const float* histRange = { range };

	bool uniform = true; bool accumulate = false;

	Mat b_hist;

	/// Compute the histograms:

	calcHist(&src, 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate);


	// Draw the histograms for B, G and R
	int hist_w = 512; int hist_h = 400;
	int bin_w = cvRound((double)hist_w / histSize);

	Mat histImage(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));

	/// Normalize the result to [ 0, histImage.rows ]
	//normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());

	/// Draw for each channel
	json hist;
	cout << "histSize: "<<histSize << std::endl;
	float middleColor = 0;
	float histSum = 0;
	//equalizeHist(b_hist, b_hist);
	int pixels = size.height*size.width;
	for (int i = 1; i < histSize; i++)
	{
		float curr = b_hist.at<float>(i - 1)/pixels;
		//cout << curr;
		middleColor += i*curr;
		histSum += curr;
		hist.push_back(curr);
		line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(curr*hist_h)),
			Point(bin_w*(i), hist_h - cvRound(b_hist.at<float>(i)*hist_h /pixels)),
			Scalar(255, 0, 0), 2, 8, 0);
	}
	middleColor /= histSize;
	int _phase= p.whichPhase(middleColor);
	cout << std::endl << "middleColor: " << middleColor;
	cout << std::endl << "histSum: " << histSum;
	cout << std::endl << "phase" << _phase;
	output["middleColor"] = middleColor;
	output["histogram"] = hist;
	output["size"] = { size.height,size.width };
	output["phase"] = _phase;
	/// Display
	if (showPictures)
	{
		namedWindow("histogram", CV_WINDOW_AUTOSIZE);
		imshow("histogram", histImage);
		namedWindow("source", WINDOW_NORMAL);
		resizeWindow("source", 600, 600);
		imshow("source", src);
		waitKey(0);
	}

	return hist;
}

