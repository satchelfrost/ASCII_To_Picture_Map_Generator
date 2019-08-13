#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

using namespace std;

class TGA
{
public:

	//TGA file header
	struct Header
	{
		char idLength;
		char colorMapType;
		char dataTypeCode;
		short colorMapOrigin;
		short colorMapLength;
		char colorMapDepth;
		short xOrigin;
		short yOrigin;
		short width;
		short height;
		char bitsPerPixel;
		char imageDescriptor;
	};
	
	//Type of pixel (i.e. R,G,B or R,G,B,A)
	struct Pixel {
		unsigned char blue;
		unsigned char green;
		unsigned char red;
		unsigned char alpha;

		//Three channel pixel
		Pixel(unsigned char b, unsigned char g, unsigned char r) :
			blue(b), green(g), red(r) {}

		//Four channel pixel
		Pixel(unsigned char b, unsigned char g, unsigned char r, unsigned char a) :
			blue(b), green(g), red(r), alpha(a) {}

	};

	TGA(string fileName);                 //constructor
	void stitchRight(TGA& file);          //combine two TGA images side by side
	void stitchMultiRight(vector<TGA>&);  //combine multiple TGA images side by side
	void stitchUp(TGA& file);             //combine two TGA imgaes up and down
	void stitchMultiUp(vector<TGA>&);     //combine multiple TGA imges up and down
	void output(string fileName);         //write the current TGA data to a file
	

private:
	Header _header;                                 //Store header info
	vector<Pixel> _pixelsVec;                       //pixel vector to hold each pixel
	unsigned int _numPixels;                        //number of pixels
	void read_Header(ifstream&);                    //does what you think
	void read_RGB(ifstream&);                       //reads non-RLE RGB pixels
	void read_RGBA(ifstream&, int&);                //reads non-RLE RGBA pixels
	void read_RLE_RGBA(ifstream&, int&);            //reads RLE RGBA pixels
	void decompress_RGBA(ifstream&);                //decompresses TGA, stores into _pixelsVec
};
