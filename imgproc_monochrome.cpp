#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/opencv.hpp"
#include <iostream>
#include <stdlib.h>
#include <cmath>


/* SUPPORTED MOTION OF MOTOR : ANTICLOCKWISE WITH INITIAL POSITION AS HORIZONTAL RIGHT */

using namespace std; 
using namespace cv;

#define pi 3.14159265
#define lli long long int 

// Structure to store the sector and region number
struct Region
{
	int sector;
	int region;
};

// Declaration of general global variables
int no_of_leds=32;
int no_of_sectors=200;
const string default_image_path="/home/yash/Desktop/test-image.jpg";

// Declaration of dynamic global variables
int xcenter;
int ycenter;
double radius;
Mat frame;

// Function to create the resulting image
Mat createResult(vector< vector< vector<int> > >,Mat);
// Function to pixellate an image
Mat pixellate(Mat);

// Function to calculate the distance between two pixels
double distanceFromCenter(int x,int y)
{
	return sqrt((x-xcenter)*(x-xcenter)+(y-ycenter)*(y-ycenter));
}

// Function to calculate the sector and region number for a given pixel
Region calcRegionNumber(int x,int y)
{
	// calculating distance from the center
	double dist=distanceFromCenter(x,y);
	// calculating angle from base line
	double angle=atan2(y-ycenter,x-xcenter);
	// converting angle to degrees [0,360]
	angle = (angle<0)? angle*180.00/pi + 360.00 : angle*180.00/pi;

	Region v;
	// calculating sector number
	double delta_theta=360.0/no_of_sectors;
	v.sector=(int)(angle/delta_theta);
	// calculating region number
	double delta_radius=radius/no_of_leds;
	v.region=(int)(dist/delta_radius);

	// applying constraints
	v.sector=(v.sector>=no_of_sectors)? (no_of_sectors-1) : v.sector;
	v.region=(v.region>=no_of_leds)? (no_of_leds-1) : v.region;

	return v;

}
int main(int argc, char const *argv[])
{
	system("clear");
	string mode="image";
	cout<<"MONOCHROME DISPLAY - BINARY INPUT (ON or OFF)\n";
	cout<<"Please select the mode :\n1. Static image\n2. Video (dynamic image)\nYour choice (1 or 2): ";
	int choice; cin>>choice;
	mode=(choice==1)? "image" : (choice==2)? "video" : "fail";
	if(mode=="fail")
	{ 
		cout<<"Wrong option!";
		return -1;
	}
	else if(mode=="image")
	{
		system("clear");
		bool pass=false;
		// Accepting image from the user
		string image_path;
		do
		{
			std::cout<<"Enter the complete path of the image - \nType \"default\" (sans quotes) for using default image :\n";
			std::cin>>image_path;
			if(image_path=="default")
			{
				image_path=default_image_path;
			// Storing the specified image as a matrix of pixels
				frame = imread(image_path, CV_LOAD_IMAGE_COLOR); 
				pass=true;
			}
			else
			{

    		// Storing the specified image as a matrix of pixels
				frame = imread(image_path, CV_LOAD_IMAGE_COLOR); 

				if(! frame.data)
				{
					cout<<"Could not find OR load specified image.\nPlease try again.\n";
					pass=false;
				}
				else
				{
	    		// Displaying the selected image
					imshow("Selected image : Press any key to continue.",frame);
					waitKey(0);
					cout<<"Was that the image you wanted? Type 'y' OR 'n'.\n";
					char ans; cin>>ans;
					pass=(ans=='y')? true : false;
				}
			}
		}while(pass==false);

		imshow("Resulting image",pixellate(frame));
		waitKey(0);
	}
	else if(mode=="video")
	{ 
		system("clear");
		cout<<"Select the type of video input:\n1. Web-cam\n2. Existing file\nYour choice (1 or 2) : ";
		int choice1; cin>>choice1;
		switch(choice1)
		{
			case 1: 
			{
				VideoCapture cap(0); // open the default camera
    			if(!cap.isOpened())  // check if we succeeded
    			{ 
    				cout<<"Error in opening camera!\n"; 
    				return -1;
    			}

    			for(int count=0;count<200;count++)
    			{
    				Mat frame;
        			cap >> frame; // get a new frame from camera
        			imshow("Pixellated video - 23 seconds",pixellate(frame));
        			waitKey(1);

        		}
        		cap.release();
        		waitKey(0);
        		break;
        	}
        	case 2: 
        	{
        		cout<<"Enter the exact path of the video file:\n";
        		string path; cin>>path;
        		VideoCapture vid_file;
        		if(!vid_file.open(path))
        		{
        			cout<<"Error in opening file.\n"; 
        			return -1; 
        		}
        		else
        		{
        			for(;;)
        			{
        				Mat frame;
        				vid_file >> frame;
        				if(frame.empty())
        				{
        					break;
        				}
        				else
        				{
        					imshow("Pixellated video", pixellate(frame));
        					waitKey(1);
        				}
        			}
        			vid_file.release();
        			waitKey(0);
        		}
        		break;
        	}
        	default: 
        	{
        		cout<<"Wrong option!\n";
        		return -1;
        	}
        }
    }

    cout<<"Thank you!\n";
    return 0;
}

Mat pixellate(Mat pixels)
{
	// Defining vectors to store image components     
	vector< vector< vector<lli> > > data_of_regions(no_of_sectors,vector< vector<lli> >(no_of_leds, vector<lli>(4, 1)));


    // Defining the bounding circle
	xcenter=(int)(pixels.cols+1)/2;
	ycenter=(int)(pixels.rows+1)/2;
	radius=min(xcenter,ycenter);

    // Parsing the image array
	for(int y=0;y<pixels.rows;y++)
	{
		for(int x=0;x<pixels.cols;x++)
		{
			if(distanceFromCenter(x,y)<radius)
			{
				Vec3b pixel=pixels.at<Vec3b>(Point(x,y));
				Region temp=calcRegionNumber(x,y);
				// accumulating B,G,R values in data_of_regions     			
				data_of_regions[temp.sector][temp.region][0]+=pixel[0];
				data_of_regions[temp.sector][temp.region][1]+=pixel[1];
				data_of_regions[temp.sector][temp.region][2]+=pixel[2];
    			// incrementing the number of pixels in that region
				data_of_regions[temp.sector][temp.region][3]++;
			}
		}
	}

    // Vector storing the final data to be sent to Arduino
	vector< vector< vector<int> > > averaged_pixels(no_of_sectors,vector< vector<int> >(no_of_leds,vector<int>(3,0)));

	// Averaging out the color in all regions - data stored in B,G,R format
	for(int i=0;i<no_of_sectors;i++)
	{
		for(int j=0;j<no_of_leds;j++)
		{
			lli cnt = data_of_regions[i][j][3];
    		averaged_pixels[i][j][0]=(int)(data_of_regions[i][j][0]/cnt); // averaged blue color
    		averaged_pixels[i][j][1]=(int)(data_of_regions[i][j][1]/cnt); // averaged green color
    		averaged_pixels[i][j][2]=(int)(data_of_regions[i][j][2]/cnt); // averaged red color
    	}
    }

    cout<<"Averaging complete.\n";
    // recreating the averaged out image
    return createResult(averaged_pixels,pixels);
    
}


Mat createResult(vector< vector< vector<int> > > averaged_pixels, Mat org_img)
{
	Mat res_image=org_img;
	for(int y=0;y<res_image.rows;y++)
	{
		for(int x=0;x<res_image.cols;x++)
		{
			if(distanceFromCenter(x,y)<radius)
			{

				Region temp=calcRegionNumber(x,y);
				res_image.at<Vec3b>(Point(x,y))=Vec3b(averaged_pixels[temp.sector][temp.region][0],averaged_pixels[temp.sector][temp.region][1],averaged_pixels[temp.sector][temp.region][2]);
				
			}
			else
				res_image.at<Vec3b>(Point(x,y))=Vec3b(0,0,0);

		}
	}

	// Conversion of grayscale image to binary (black and white) image
	Mat temp;
	cvtColor(res_image,temp,CV_RGB2GRAY);
	temp=temp>127;
	return temp;

}