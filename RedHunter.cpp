/////////////////////////////////////////////////////////////////////////////////////////
// RedHunter.cpp  by Chris Hjelmfelt  Spring 2014
//
// Detects clusters of red pixils in a single image, finds center point of largest cluster 
// and gives a direction to turn (up, down, right, left) and a distance to turn
// (a percentage measured out from the center of the image)
//
// Seems to be working needs to be put through test cases
//////////////////////////////////////////////////////////////////////////////////////////


#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>
#include <cmath>

using namespace cv;
using namespace std;

// Function Declarations
void findRed(Mat);


int main(int argc, char** argv ){
	// Variables
	Mat frame; // a single frame of video from our source	
	VideoCapture inputVideo; // Input Video from default camera	
	
	if(argc != 2){  // If no filename is given use the default camera
		inputVideo.open(0);  // open the default camera  
		if(!inputVideo.isOpened()){  // check if we succeeded
			cout << "Could not open default camera " << endl;
			return -1;
		}
	} 
	else{  // If a filename is given open and use it  
		const string source = argv[1];  // the source file name  
		inputVideo.open(source);  // open input    
		if(!inputVideo.isOpened()){  // check if we succeeded
			cout  << "Could not open the input video: " << source << endl;
			return -1;
		}
	}
	
	// Get a frame of video
	inputVideo >> frame; // get a new frame from camera 
	if (frame.empty()){
		cout << "The video frame is empty";
	}
	else {
		// Keep getting and processing new frames of video
		for(;;){   
			inputVideo >> frame; // get a new frame from camera 
			if (frame.empty()) break;	
			
			
			findRed(frame);
			if(waitKey(30) >= 0) break; // end program when user presses a key
		}
	}
	return 1;
}

	
// @function findRed 
// Uses an array to locate red pixil clusters
void findRed(Mat src){
	// Variables	
	//char* window_name = "Balloon"; // name for image display window
	int radius = 20; // the radius of pixils we want to search around a red pixil
	int row, col;  // keeps track of placement in grid around a pixil in red search	
	int** gridred; // an array of the pixils within our picture
	int** gridcluster; // an array of the number of red pixils surrounding each pixil
	int colorB = 0, colorG = 0, colorR = 0; // hold color values for blue, green, red
	int center = 0; // counts the number of pixils with the max value
	int mrow = 0, mcol = 0; // holds the i and j of the pixil with the highest num of red pixils around it
	float turnw, turnh; // distance from center to pixil with max red
	int max = 0; // counts the highest number of red pixils in a radius
	int maxold = 0; // holds the value of the max from previous loop through while
	int rp = 0, cp = 0; // points in the row and column around the target pixil
	int safe = 0; // a counter to prevent endless looping if the while statement doesn't work
	
	int height = src.size().height;
	int width = src.size().width;
	
	//namedWindow( window_name, CV_WINDOW_AUTOSIZE ); // Make a window to display results
	
	// Array to keep track of which pixils are red
	gridred = new int*[width];
	for(int i = 0; i < width; ++i){
		 gridred[i] = new int[height];
	}
	
	// Array to keep track of how many surrounding pixils are red
	gridcluster = new int*[width];
	for(int i = 0; i < width; ++i){
		 gridcluster[i] = new int[height];
	}		 
	
	// Convert image to 1's and 0's where 1 indicates a red pixil
	for(int i=0; i < width; i++){
		for(int j=0; j < height; j++){    			
		   Vec3b bgrPixel = src.at<Vec3b>(j,i); // Make a vector of pixils
			colorB = bgrPixel.val[0]; // Blue value
			colorG = bgrPixel.val[1]; // Green value
			colorR = bgrPixel.val[2]; // Red value
    		if (colorB < 100 && colorG < 100 && colorR > 100){  // red pixils marked as 1
    			gridred[i][j] = 1;
    			gridcluster[i][j] = 1; 	 
    		}
			else{ // other pixils marked as 0
				gridred[i][j] = 0;
				gridcluster[i][j] = 0; 	 
			}						
		}
	}	
	
	// Recursive loop to find the largest cluster of red pixils
	while(center < 2 && safe < 5){
		max = 0, cp = 0, rp = 0; // reset variables
		// iterate through all pixils in gridred
		for(int i=0; i < width; i++){
			for(int j=0; j < height; j++){
			
				// only search around red pixils	(after 1st while loop this will be largest clusters)				
				if(gridcluster[i][j] > maxold){ 
					// check pixils in area around target pixil
					for(int k=0; k < 2*radius+1; k++){ 
						for(int l=0; l < 2*radius+1; l++){
							// test for out of bounds
							if((i-radius+k) >= 0 && (j-radius+l) >= 0 && (i-radius+k) < width && (j-radius+l) < height){ 
								cp = i-radius+k;	
								rp = j-radius+l;	
							}								
							if(gridred[cp][rp] == 1){									
						 		gridcluster[i][j]= gridcluster[i][j] + 1; // red pixil found add one to this point
						 	}
						}
					}
				}
				// Check for highest red value
				if(gridcluster[i][j] > max){ 
					max = gridcluster[i][j]; // set max to number of red pixils around this point
					// keep track of cluster with highest count
					mcol = i; 
					mrow = j;
					center = 1; // currently a single max point
				}	
				// Check to see if more than one pixil has the max value	
				else if(gridcluster[i][j] == max){ 
					center++; // currently multiple max points
				}						
			}
		}		
	maxold = max; // keep track of current max 
	radius = radius +10; // increase search radius		 
	safe++;  // loop count
	}
	
	// Display some useful information
	cout << "Image width: " << width << " height: " << height << endl;
	// if center is more than 1 we have multiple pixils with max value
	cout << "# of center points: " << center << endl; 
	// The highest number of red pixils in the area around a single pixil and the pixils location
	cout << "Max number of pixils in one cluster: " << max << "   Located at column: " << mcol << "   row: " << mrow << endl; 	
	
	// imshow( window_name, gridcluster ); // Display results in window
	
	// Give direction to go to intercept balloon
	turnw = ((float)width/2 - mcol)/((float)width/2); // gives a +/- percent left or right
	turnh = ((float)height/2 - mrow)/((float)height/2); // gives a +/- percent up or down
	if (turnw < 0){ cout << "right  " << turnw << " percent" << endl; }
	else { cout << "left  " << turnw << " percent" << endl; }
	if (turnh < 0){ cout << "down  " << turnh << " percent" << endl; }
	else { cout << "up  " << turnh << " percent" << endl; }	
	
	// Free up memory
	delete gridred; 
	delete gridcluster;
}


