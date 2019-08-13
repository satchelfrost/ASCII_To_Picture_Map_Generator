#include "TGA.h"

TGA::TGA(string fileName)
{
	//based on the file name read in the header file into our struct
	ifstream file{ fileName, ios_base::binary };

	if (file.is_open()) {

		//read in values into our header struct
		read_Header(file);

		//calculate the number of pixels based on height * width
		_numPixels = (_header.width) * (_header.height);

		//24 bits per pixel means only RGB
		if (_header.bitsPerPixel == 24) 
			read_RGB(file);
		
		
		//32 bits per pixel means RGBA
		if (_header.bitsPerPixel == 32) {
			decompress_RGBA(file);
			_header.dataTypeCode = 2; //2 means it is a decompressed, see TGA specs
		}
	}

	else
		cout << "No such file name: " << fileName << endl;
}

void TGA::stitchRight(TGA& file)
{
	// This method stiches two TGA images side by side.
	//
	// e.g.
	//
	//   This       Incoming      This Newly formed
	// .tga file   .tga file         .tga file 
	//                          
	//  |1,2,3|    |10,11,12|     |1,2,3,10,11,12|
	//  |4,5,6|  + |13,14,15|  =  |4,5,6,13,14,15|
	//  |7,8,9|    |16,17,18|     |7,8,9,16,17,18|
	//
	// In the above example numbers represent pixels.
	// The actual implementation in a vector is as follows:
	//
	// e.g. 
	//
	// v_1 = { 1, 2, 3, 4, 5, 6, 7, 8, 9}
	// v_2 = {10,11,12,13,14,15,16,17,18}
	// v_3 = { 1, 2, 3, 10, 11, 12, 4, 5, 6, 13, 14, 15, 7, 8, 9, 16, 17, 18}
	//
	
	//if image heights are different then we can't merge
	if (_header.height != file._header.height) {
		cout << "Stitch-Right failed! Image heights don't match."  << endl;
		cout << "This image's height: "     << _header.height      << endl;
		cout << "Incoming image's height: " << file._header.height << endl;
		return;
	}

	if (this == &file) {
		cout << "Cannot self stitch, need two separate files" << endl;
		return;
	}
	
	vector<Pixel> v_1 = _pixelsVec;        //current objects vector
	vector<Pixel> v_2 = file._pixelsVec;   //incoming objects vector
	vector<Pixel> v_3;                     //what will be the current objects new vector
		
	unsigned int SIZE_1 = v_1.size();
	unsigned int SIZE_2 = v_2.size();
	v_3.reserve(SIZE_1 + SIZE_2);          //make space for the merge
		
	unsigned int i_v1 = 0;                  //iterate through current object's vector
	unsigned int i_v2 = 0;                  //iterate through incoming object's vector
	short WIDTH_1 = _header.width;          //width of the current image object
	short WIDTH_2 = file._header.width;     //width of the incoming image object
	unsigned int v1_end_scanLine = WIDTH_1; //end of the i-th scan line for v1
	unsigned int v2_end_scanLine = WIDTH_2; //end of the i-th scan line for v2

	//Merge the two vectors v_1 and v_2 into v_3
	while (i_v1 < SIZE_1 && i_v2 < SIZE_2) {
		//put the i-th scan line in v_1 into v_3
		for (int pixel = i_v1; pixel < v1_end_scanLine; pixel++)
			v_3.push_back(v_1[pixel]);
		//put the i-th scan line in v_2 into v_3
		for (int pixel = i_v2; pixel < v2_end_scanLine; pixel++)
			v_3.push_back(v_2[pixel]);

		i_v1             += WIDTH_1;
		i_v2             += WIDTH_2; 
		v1_end_scanLine  += WIDTH_1; 
		v2_end_scanLine  += WIDTH_2; 
	}

	//make this objects new vector equal to v_3 
	_pixelsVec = v_3;

	//since we have a new image reset the width
	_header.width = WIDTH_1 + WIDTH_2;
	//since we have a new image reset the number of pixels
	_numPixels = (_header.width) * (_header.height);
	
}

void TGA::stitchMultiRight(vector<TGA>& files)
{
	// This method works much the same way as stitchRight
	// except it takes in multiple files at once and stitches them with this object
	// e.g.
	// v_this = {1,2,3,4,5,6,7,8,9}
	// v_new = {1,2,3, .... , 4,5,6, .... ,7,8,9 ....}
	// where the space for "...." depends on how many incoming files there are.

	//check to make sure all files are the same width and height
	for (unsigned int i = 0; i < files.size(); i++) {
		if (_header.height != files[i]._header.height) {
			cout << "Stitch-MULTI-Right failed! Image heights don't match." << endl;
			cout << "This image's height: " << _header.height << endl;
			cout << "Incoming image's height: " << files[i]._header.height << endl;
			return;
		}

		if (_header.width != files[i]._header.width) {
			cout << "Stitch-MULTI-Right failed! Image widths don't match." << endl;
			cout << "This image's width: " << _header.width << endl;
			cout << "Incoming image's width: " << files[i]._header.width << endl;
			return;
		}
	}

	unsigned int numFiles = files.size();
	vector<Pixel> v_this = _pixelsVec;        //current objects vector
	vector<Pixel> v_new;                      //what will be the current objects new vector
	short WIDTH = _header.width;               //width of the current image object
	unsigned int SIZE_this = v_this.size();
	unsigned int SIZE_MULTI = numFiles * (files[0]._pixelsVec.size());
	v_new.reserve(SIZE_this + SIZE_MULTI);    //make space for the merge
	
	unsigned int i = 0;
	unsigned int scanLine = WIDTH;
	unsigned int pixCount = 0;

	//Merge vector v_this into v_new, along with all of the incoming vectors
	while (pixCount != v_new.capacity()) {

		//put the i-th scan line of v_this into v_new
		for (int pixel = i; pixel < scanLine; pixel++) {
			v_new.push_back(v_this[pixel]);
			pixCount++;
		}

		//put the i-th scan line of the j-th incoming vector into v_new
		for (unsigned int j = 0; j < files.size(); j++) {
			for (int pixel = i; pixel < scanLine; pixel++) {
				v_new.push_back(files[j]._pixelsVec[pixel]);
				pixCount++;
			}
		}
		
		i        += WIDTH;
		scanLine += WIDTH;

	}


	//make this objects new vector equal to v_new
	_pixelsVec = v_new;

	//since we have a new image reset the width
	_header.width += (numFiles * WIDTH);

	//since we have a new image reset the number of pixels
	_numPixels = (_header.width) * (_header.height);
}

void TGA::stitchUp(TGA& file)
{
	// This method stiches two TGA images side by side.
	//
	// e.g.
	//
	//   This       Incoming      This Newly formed
	// .tga file   .tga file         .tga file 
	//                          
	//  |1,2,3|    |10,11,12|      |10,11,12|
	//  |4,5,6|  + |13,14,15|   =  |13,14,15|
	//  |7,8,9|    |16,17,18|      |16,17,18|
	//                             | 1, 2, 3|
	//                             | 4, 5, 6|
	//                             | 7, 8, 9|
	//
	// In the above example numbers represent pixels.
	// The actual implementation in a vector is as follows:
	//
	// e.g. 
	//
	// v_1 = { 1, 2, 3, 4, 5, 6, 7, 8, 9}
	// v_2 = {10,11,12,13,14,15,16,17,18}
	// v_3 = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18}
	//

	//if image widths are different then we can't merge
	if (_header.width != file._header.width) {
		cout << "Stitch-UP failed! Image widths don't match." << endl;
		cout << "This image's width: " << _header.width << endl;
		cout << "Incoming image's width: " << file._header.width << endl;
		return;
	}

	vector<Pixel> v_1 = _pixelsVec;        //current objects vector
	vector<Pixel> v_2 = file._pixelsVec;   //incoming objects vector
	vector<Pixel> v_3;                     //what will be the current objects new vector

	unsigned int SIZE_1 = v_1.size();
	unsigned int SIZE_2 = v_2.size();
	v_3.reserve(SIZE_1 + SIZE_2);            //make space for the merge


	short HEIGHT_1 = _header.height;          //height of the current image object
	short HEIGHT_2 = file._header.height;     //height of the incoming image object


	//put v_2 into v_3
	for (unsigned int i = 0; i < SIZE_2; i++)
		v_3.push_back(v_2[i]);

	//put v_1 into v_3
	for (unsigned int i = 0; i < SIZE_1; i++)
		v_3.push_back(v_1[i]);

	
	//make this objects new vector v_3
	_pixelsVec = v_3;

	//since we have a new image reset the width
	_header.height = HEIGHT_1 + HEIGHT_2;
	//since we have a new image reset the number of pixels
	_numPixels = (_header.width) * (_header.height);
}

void TGA::stitchMultiUp(vector<TGA>& files)
{
	// This method works much the same way as stich up but with multiple files

	//check to make sure all files are the same width and height
	for (unsigned int i = 0; i < files.size(); i++) {
		if (_header.height != files[i]._header.height) {
			cout << "Stitch-MULTI-Right failed! Image heights don't match." << endl;
			cout << "This image's height: " << _header.height << endl;
			cout << "Incoming image's height: " << files[i]._header.height << endl;
			return;
		}

		if (_header.width != files[i]._header.width) {
			cout << "Stitch-MULTI-Right failed! Image widths don't match." << endl;
			cout << "This image's width: " << _header.width << endl;
			cout << "Incoming image's width: " << files[i]._header.width << endl;
			return;
		}
	}

	vector<Pixel> v_3;                     //what will be the current objects new vector
	unsigned int SIZE = _pixelsVec.size(); //all vectors are assumed to be the same size,
								           //if they weren't then the check to make sure 
								           //all files are the same height and width would fail
	unsigned int numFiles = files.size();  //number of incoming files, which contain pixel vects
	v_3.reserve(SIZE * numFiles);          //make space for the merge
	short HEIGHT = _header.height;         //height of the current image object
	
	//push each vector into v_3 in reverse order
	for (unsigned int i = numFiles - 1; i != 0; i--) {
		for (unsigned int j = 0; j < SIZE; j++)
			v_3.push_back(files[i]._pixelsVec[j]);
		cout << "line " << i << " stitched" << endl;
	}
	for (unsigned int i = 0; i < SIZE; i++)
		v_3.push_back(files[0]._pixelsVec[i]);
	cout << "line 0 stitched" << endl;

	//make this objects new vector v_3
	_pixelsVec = v_3;

	//since we have a new image reset the width
	//NOTE: numFiles includes this object
	_header.height = HEIGHT * numFiles;

	//since we have a new image reset the number of pixels
	_numPixels = (_header.width) * (_header.height);
}

void TGA::output(string fileName)
{
	fstream file{ fileName, ios_base::out | ios_base::binary };

	file.write((char*)& _header.idLength, sizeof(_header.idLength));
	file.write((char*)& _header.colorMapType, sizeof(_header.colorMapType));
	file.write((char*)& _header.dataTypeCode, sizeof(_header.dataTypeCode));
	file.write((char*)& _header.colorMapOrigin, sizeof(_header.colorMapOrigin));
	file.write((char*)& _header.colorMapLength, sizeof(_header.colorMapLength));
	file.write((char*)& _header.colorMapDepth, sizeof(_header.colorMapDepth));
	file.write((char*)& _header.xOrigin, sizeof(_header.xOrigin));
	file.write((char*)& _header.yOrigin, sizeof(_header.yOrigin));
	file.write((char*)& _header.width, sizeof(_header.width));
	file.write((char*)& _header.height, sizeof(_header.height));
	file.write((char*)& _header.bitsPerPixel, sizeof(_header.bitsPerPixel));
	file.write((char*)& _header.imageDescriptor, sizeof(_header.imageDescriptor));

	//24 bits per pixel means only RGB
	if (_header.bitsPerPixel == 24) {
		for (unsigned int i = 0; i < _numPixels; i++) {
			file.write((char*) & _pixelsVec[i].blue, sizeof(_pixelsVec[i].blue));
			file.write((char*) & _pixelsVec[i].green, sizeof(_pixelsVec[i].green));
			file.write((char*) & _pixelsVec[i].red, sizeof(_pixelsVec[i].red));
		}
	}

	//32 bits per pixel means only RGBA
	if (_header.bitsPerPixel == 32) {
		for (unsigned int i = 0; i < _numPixels; i++) {
			file.write((char*) & _pixelsVec[i].blue, sizeof(_pixelsVec[i].blue));
			file.write((char*) & _pixelsVec[i].green, sizeof(_pixelsVec[i].green));
			file.write((char*) & _pixelsVec[i].red, sizeof(_pixelsVec[i].red));
			file.write((char*) & _pixelsVec[i].alpha, sizeof(_pixelsVec[i].alpha));
		}
	}
}

void TGA::read_Header(ifstream &file)
{
	file.read((char*)& _header.idLength, sizeof(_header.idLength));
	file.read((char*)& _header.colorMapType, sizeof(_header.colorMapType));
	file.read((char*)& _header.dataTypeCode, sizeof(_header.dataTypeCode));
	file.read((char*)& _header.colorMapOrigin, sizeof(_header.colorMapOrigin));
	file.read((char*)& _header.colorMapLength, sizeof(_header.colorMapLength));
	file.read((char*)& _header.colorMapDepth, sizeof(_header.colorMapDepth));
	file.read((char*)& _header.xOrigin, sizeof(_header.xOrigin));
	file.read((char*)& _header.yOrigin, sizeof(_header.yOrigin));
	file.read((char*)& _header.width, sizeof(_header.width));
	file.read((char*)& _header.height, sizeof(_header.height));
	file.read((char*)& _header.bitsPerPixel, sizeof(_header.bitsPerPixel));
	file.read((char*)& _header.imageDescriptor, sizeof(_header.imageDescriptor));
}

void TGA::read_RGB(ifstream& file)
{
	//for each pixel add it to our pixel vector
	Pixel pix(0, 0, 0);
	for (unsigned int i = 0; i < _numPixels; i++) {
		file.read((char*)& pix.blue, sizeof(pix.blue));
		file.read((char*)& pix.green, sizeof(pix.green));
		file.read((char*)& pix.red, sizeof(pix.red));
		_pixelsVec.push_back(pix);
	}
}

void TGA::read_RLE_RGBA(ifstream& file, int& N)
{
	//read one pixel N-times
	Pixel pix(0, 0, 0, 0);
	file.read((char*)& pix.blue, sizeof(pix.blue));
	file.read((char*)& pix.green, sizeof(pix.green));
	file.read((char*)& pix.red, sizeof(pix.red));
	file.read((char*)& pix.alpha, sizeof(pix.alpha));
	for (int i = 0; i < N; i++)
		_pixelsVec.push_back(pix);
}

void TGA::read_RGBA(ifstream& file, int& N)
{
	//read N pixels
	Pixel pix(0, 0, 0, 0);
	for (int i = 0; i < N; i++) {
		file.read((char*)& pix.blue, sizeof(pix.blue));
		file.read((char*)& pix.green, sizeof(pix.green));
		file.read((char*)& pix.red, sizeof(pix.red));
		file.read((char*)& pix.alpha, sizeof(pix.alpha));
		_pixelsVec.push_back(pix);
	}
}

void TGA::decompress_RGBA(ifstream& file)
{
	unsigned char info; //run length encoded information
	int N = 0;          //number of RLE pixels, or non RLE pixels

	for (int pixCount = 0; pixCount != _numPixels; pixCount += N) {

		file.read((char*)& info, sizeof(info)); //read RLE info
		bool RLE = (info >> 7);                 //see if RLE or non-RLE (i.e. check high bit)
		N = (info & 0b01111111) + 1;            //the lower 7 bits + 1, is stored as N

		//if RLE, read the next pixel N-times
		if (RLE)
			read_RLE_RGBA(file, N);
		
		//else read the next N pixels
		else
			read_RGBA(file, N);
	}
}