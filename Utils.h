#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <string>
#include <fstream>
#include <bitset>
#include <cmath>
#include <vector>
#include <cstring>

using namespace std;

class Section
{
private:
	string sec;
	int len;
public:
	Section(string sec = "", int len = 0) : sec(sec), len(len) {}

	friend class Decoder;
};

class Decoder
{
private:
	string fName;
	ifstream GRIBfile;
	vector<Section> sections;
	int GRIBsize;
	bool GDS;
	bool BMS;
protected:
	int SectionLength(string section);
	void ReadSection(ifstream& GRIBfile_, const int secNum);
	int ByToInt(string bytes);
	int ByToIntExBitFlag(string bytes);
	float ByToFloat(string bytes);
	bool Bit1Value(char byte);
public:
	Decoder(string fName);
	void CreateOutputFile(string fName);
};

Decoder::Decoder(string fName) : sections(6)
{
	GRIBfile.open(fName, ios::binary);
	string str;
	char ch;
	if (!GRIBfile)
	{
		cerr << "File error.";
		exit(1);
	}
	while (str != "GRIB")
	{
		if (GRIBfile.eof())
		{
			cerr << "No GRIB found.\n";
			exit(1);
		}
		str = "";
		for (int i = 0; i < 4; i++)
		{
			GRIBfile.read(&ch, 1);
			str.push_back(ch);
		}
		GRIBfile.seekg(-3, ios::cur);
	}
	GRIBfile.seekg(-1, ios::cur);
	sections[0].len = 8;
	sections[0].sec.resize(sections[0].len);
	GRIBfile.read(&sections[0].sec[0], 8);

	GRIBsize = (unsigned char(sections[0].sec[4]) << 16) | (unsigned char(sections[0].sec[5]) << 8) | (unsigned char(sections[0].sec[6]));

	ReadSection(GRIBfile, 1);

	if (sections[1].sec.size() < 8) {
		cerr << "Error S1 B8\n";
		exit(1);
	}

	GDS = (unsigned char(sections[1].sec[7]) >> 7) & 1;
	BMS = (unsigned char(sections[1].sec[7]) >> 6) & 1;

	if (GDS) ReadSection(GRIBfile, 2);

	if (BMS) ReadSection(GRIBfile, 3);

	ReadSection(GRIBfile, 4);

	sections[5].len = 4;
	sections[5].sec.resize(sections[5].len);
	GRIBfile.read(&sections[5].sec[0], 4);
}

int Decoder::SectionLength(string sec)
{
	int num = ((unsigned char)sec[0] << 16) | ((unsigned char)sec[1] << 8) | ((unsigned char)sec[2]);
	return num;
}

void Decoder::ReadSection(ifstream& GRIBfile_, const int secNum)
{
	string length;
	length.resize(3);
	GRIBfile_.read(&length[0], 3);
	sections[secNum].len = SectionLength(length);
	sections[secNum].sec.resize(sections[secNum].len);
	GRIBfile_.seekg(-3, ios::cur);
	GRIBfile_.read(&sections[secNum].sec[0], sections[secNum].len);
}

void Decoder::CreateOutputFile(string fName)
{
	ofstream oFile(fName);
	bool negScFac = false;
	string str;

	oFile << "SECTION 0:" << endl;
	oFile << "\t1. MSG LENGTH:\t" << GRIBsize << "\t=>\t" << sections[0].len << " + " << sections[1].len << " + " << sections[2].len << " + " << sections[3].len << " + "
		  << sections[4].len << " + " << sections[5].len;
	
	oFile << "\nSECTION 1:" << endl;
	oFile << "\t1. LENGTH:\t" << sections[1].len << endl;
	oFile << "\t2. B8:\tGDS:\t" << (GDS ? "Included" : "Omitted") << ",\tBMS:\t" << (BMS ? "Included" : "Omitted") << endl;
	oFile << "\t3. B13-17(DATE):\t" << unsigned int(sections[1].sec[14]) << "." << unsigned int(sections[1].sec[13]) << "." 
		  << unsigned int(sections[1].sec[12]) << "\t" << (unsigned int(sections[1].sec[15]) < 10 ? "0" : "") 
		  << unsigned int(sections[1].sec[15]) << ":" << (unsigned int(sections[1].sec[15]) < 10 ? "0" : "") << unsigned int(sections[1].sec[16]) << endl;
	oFile << "\t4. B27-28 SCALE FACTOR:\t";
	oFile << (Bit1Value(sections[1].sec[26]) ? ByToIntExBitFlag(sections[1].sec.substr(26, 2)) * -1 : ByToIntExBitFlag(sections[1].sec.substr(26, 2)));
	str.clear();

	if (GDS)
	{
		oFile << "\nSECTION 2:" << endl;
		oFile << "\t1. LENGTH:\t" << sections[2].len << endl;
		oFile << "\t2. La1:\t" << (Bit1Value(sections[2].sec[10]) ? "SOUTH " : "NORTH ") << ByToIntExBitFlag(sections[2].sec.substr(10, 3)) << endl;
		oFile << "\t3. Lo1:\t" << (Bit1Value(sections[2].sec[13]) ? "WEST " : "EAST ") << ByToIntExBitFlag(sections[2].sec.substr(13, 3)) << endl;
		oFile << "\t4. La2:\t" << (Bit1Value(sections[2].sec[17]) ? "SOUTH " : "NORTH ") << ByToIntExBitFlag(sections[2].sec.substr(17, 3)) << endl;
		oFile << "\t5. Lo2:\t" << (Bit1Value(sections[2].sec[20]) ? "WEST " : "EAST ") << ByToIntExBitFlag(sections[2].sec.substr(20, 3)) << endl;
		oFile << "\t6. B29-33 NUMBERS:\t";
		for (int i = 0; i < 5; i++) oFile << unsigned int(sections[2].sec[28 + i]) << " ";
		oFile << endl;
	}

	if (BMS)
	{
		oFile << "\nSECTION 3:" << endl;
		oFile << "\t1. LENGTH:\t" << sections[3].len << endl;
	}

	oFile << "\nSECTION 4:" << endl;
	oFile << "\t1. LENGTH:\t" << sections[4].len << endl;
	oFile << "\t2. B5-6 BIN SCALE FACTOR:\t" << (Bit1Value(sections[4].sec[4]) ? ByToIntExBitFlag(sections[4].sec.substr(4, 2)) * -1 : ByToIntExBitFlag(sections[4].sec.substr(4, 2))) << endl;
	oFile << "\t3. B7-10 REF VALUE:\t" << ByToFloat(sections[4].sec.substr(6, 4)) << endl;
	oFile << "\t4. B11 NUM OF BITS:\t" << unsigned int(sections[4].sec[10]) << endl;
	oFile << "\t5. B12-... VALUES:\t";
	for (int i = 0; i < 3; i++)
	{
		oFile << unsigned int(sections[4].sec[10 + i]) << " ";
	}
	oFile << "\n\nEND OF MESSAGE." << endl;
}

int Decoder::ByToInt(string bytes)
{
	int num = 0;
	for (int i = 0; i < bytes.size(); i++)
	{
		num |= unsigned char(bytes[i]) << 8 * i;
	}
	return num;
}

bool Decoder::Bit1Value(char byte)
{
	return (byte & 0x80) != 0; //0x80 w zapisie binarnym oznacza 10000000
}

int Decoder::ByToIntExBitFlag(string bytes)
{
	int num = 0;
	for (int i = 0; i < bytes.size(); i++)
	{
		if (i == 0)
		{
			num |= (unsigned char(bytes[i]) & 0x7F) << 8 * i;  // 0x7F w zapisie binarnym oznacza 01111111
		}
		else
		{
			num |= unsigned char(bytes[i]) << 8 * i;
		}
	}
	return num;
}

float Decoder::ByToFloat(string bytes)
{
	uint32_t val = 0;
    for (int i = 0; i < 4; ++i)
    {
        val = (val << 8) | static_cast<unsigned char>(bytes[i]);
    }

    float result;
    memcpy(&result, &val, sizeof(float));
    return result;
}


#endif