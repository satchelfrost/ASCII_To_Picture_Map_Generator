#include <SFML/Graphics.hpp>
#include "TGA.h"
#include <map>
#include <sstream>
#include <string>

void PNGtoTGA(string, string, float, float);
void buildHorizontal(vector<string>&, map<char, string>&, vector<TGA>&);
void buildVertical(vector<TGA>&);
void TGAtoPNG(string);
void createTGAs(map<char, string>&);
void readLevel(vector<string>&, string);

int main()
{
	
	//create a map that translates characters to PNG file names
	map<char, string> fileMap {
		{'.',"grass"},
		{'#', "wall"},
		{'/',"forward_slope"},
		{'\\',"back_slope"},
		{'*',"stone_of_truth"},
		{'+', "health"}
	};

	//store level text into a vector of strings
	vector<string> level;
	readLevel(level, "level01");

	//create tga files; note the string portion of the fileMap is the file name
	createTGAs(fileMap);

	//translate the ascii world into the picture world
	vector<TGA> TGAfiles;
	buildHorizontal(level, fileMap, TGAfiles);
	buildVertical(TGAfiles);

	//Give user some feedback
	cout << "Creating PNG file in png_output folder..." << endl;
	TGAtoPNG("final_map");
	cout << "Map built!" << endl;

	return 0;
}

void PNGtoTGA(string filePath, string fileName, float tileWidth, float tileHeight)
{
	sf::RectangleShape tile(sf::Vector2f(tileWidth, tileHeight));
	sf::Texture tileText;
	tileText.loadFromFile(filePath + "/" + fileName + ".png");
	tile.setTexture(&tileText);
	tile.getTexture()->copyToImage().saveToFile("tga_files/" + fileName + ".tga");
}

void buildHorizontal(vector<string>& level, map<char, string>& fileMap, 
	vector<TGA>& TGAfiles)
{
	unsigned int width = level[0].size();
	unsigned int height = level.size();

	for (unsigned int i = 0; i < height; i++) {
		vector<TGA> fileRow;
		char firstChar = level[i][0];
		TGA file1("tga_files/" + fileMap[firstChar] + ".tga");
		for (unsigned int j = 1; j < width; j++) {
			char key = level[i][j];
			TGA file2("tga_files/" + fileMap[key] + ".tga");
			fileRow.push_back(file2);
		}
		file1.stitchMultiRight(fileRow);
		TGAfiles.push_back(file1);
		cout << "line " << i << " built" << endl;
	}
}

void buildVertical(vector<TGA>& TGAfiles)
{
	TGAfiles[0].stitchMultiUp(TGAfiles);
	cout << "Stiching complete. Writing to TGA file..." << endl;
	TGAfiles[0].output("tga_files/final_map.tga");
}

void TGAtoPNG(string fileName)
{
	sf::RectangleShape map(sf::Vector2f(32.0f,32.0f));
	sf::Texture mapText;
	mapText.loadFromFile("tga_files/" + fileName + ".tga");
	map.setTexture(&mapText);
	map.getTexture()->copyToImage().saveToFile("png_output/" + fileName + ".png");
}

void createTGAs(map<char, string>& fileMap)
{
	for (auto itr = fileMap.begin(); itr != fileMap.end(); itr++) {
		string fileName = itr->second;
		PNGtoTGA("png_input", fileName, 32.0f, 32.0f);
	}
}

void readLevel(vector<string>& level, string levelName)
{
	ifstream inputFile;
	string input;
	inputFile.open("level/" + levelName + ".txt");
	while (getline(inputFile, input)) level.push_back(input);
	inputFile.close();
}
