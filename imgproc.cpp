#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <cmath>
#include <string>
#include <vector>

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
int no_of_leds=8;
int no_of_sectors=360;
double motor_rpm=1500.00;
const string default_image_path="/home/yash/Desktop/test-image.jpg";

// Declaration of dynamic global variables
int xcenter;
int ycenter;
double radius;
Mat pixels;

// Function to create the resulting image
void createResult(vector< vector< vector<short> > >);

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
	angle = (angle<0)? angle*180/pi + 360 : angle*180/pi;

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
	// Accepting image from the user
	string image_path;
	std::cout<<"Enter the complete path of the image :";
	std::cin>>image_path;
	if(image_path==NULL)
	{
		image_path=default_image_path;
	}

    // Defining vectors to store image components     
    vector< vector< vector<lli> > > data_of_regions(no_of_sectors,vector< vector<lli> >(no_of_leds, vector<lli>(4, 0)));

    // Storing the specified image as a matrix of pixels
    pixels = imread(image_path, CV_LOAD_IMAGE_COLOR); 

    // Defining the bounding circle
    xcenter=(pixels.cols+1)/2;
    ycenter=(pixels.rows+1)/2;
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
    vector< vector< vector<short> > > averaged_pixels(no_of_sectors,vector< vector<short> >(no_of_leds,vector<short>(3,0)));

	// Averaging out the color in all regions - data stored in B,G,R format
    for(int i=0;i<no_of_sectors;i++)
    {
    	for(int j=0;j<no_of_leds;j++)
    	{
    		lli cnt = data_of_regions[i][j][3];
    		averaged_pixels[i][j][0]=(short)(data_of_regions[i][j][0]/cnt); // averaged blue color
    		averaged_pixels[i][j][1]=(short)(data_of_regions[i][j][1]/cnt); // averaged green color
    		averaged_pixels[i][j][2]=(short)(data_of_regions[i][j][2]/cnt); // averaged red color
    	}
    }

    // recreating the averaged out image
    createResult(averaged_pixels);

    cout<<"Averaging complete.";
    return 0;	
}


void createResult(vector< vector< vector<short> > > averaged_pixels)
{
	Mat res_image=pixels;
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
    imshow("Resulting image",res_image);
    waitKey(0);

}
