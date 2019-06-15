#include <vector>
#include <string>
#include <dirent.h>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <memory.h>
#include <SFML/Graphics/Color.hpp>
#include "decoder.h"
#include "util.h"
#include "YCbCr.h"

using namespace std;

vector<string> split(const string& str) {
	vector<string> ret;
	string strs = str + " ";
	size_t pos;
	size_t size = strs.size();

	for (int i = 0; i < (int)size; ++i) {
		pos = strs.find(" ", i);
		if( pos < size) {
			std::string s = strs.substr(i, pos - i);
			ret.push_back(s);
			i = pos;
		}

	}
	return ret;
}
vector<string> dir_list(string dir_name) {
	vector<string> ret;
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(dir_name.c_str())) != nullptr) {
		while ((ent = readdir(dir)) != nullptr) {
			ret.emplace_back(ent->d_name);
		}
		closedir(dir);
		sort(ret.begin(), ret.end());
		return ret;
	} else {
		perror("");
		throw "can't open dir"s;
	}
}

double c(int i) {
	static double x = 1.0 / sqrt(2.0);
	if (i == 0) {
		return x;
	}
	else {
		return 1.0;
	}
}

inline void idct(double res[8], double mat[8]) {
    double c2 = 2*cos(M_PI/8);
    double c4 = 2*cos(2*M_PI/8);
    double c6 = 2*cos(3*M_PI/8);
    double sq8 = sqrt(8);

    // B1
    double a0 = (1./8*mat[0])*sq8;
    double a1 = (1./8*mat[4])*sq8;
    double a2 = (1./8*mat[2] - 1./8*mat[6])*sq8;
    double a3 = (1./8*mat[2] + 1./8*mat[6])*sq8;
    double a4 = (1./8*mat[5] - 1./8*mat[3])*sq8;
    double temp1 = (1./8*mat[1] + 1./8*mat[7])*sq8;
    double temp2 = (1./8*mat[3] + 1./8*mat[5])*sq8;
    double a5 = temp1 - temp2;
    double a6 = (1./8*mat[1] - 1./8*mat[7])*sq8;
    double a7 = temp1+temp2;

    // M
    double b0 = a0;
    double b1 = a1;
    double b2 = a2*c4;
    double b3 = a3;
    double Q = c2-c6, R = c2+c6, temp4 = c6*(a4+a6);
    double b4 = -Q*a4 - temp4;
    double b5 = a5*c4;
    double b6 = R*a6 - temp4;
    double b7 = a7;

    // A1
    double temp3 = b6 - b7;
    double n0 = temp3 - b5;
    double n1 = b0 - b1;
    double n2 = b2 - b3;
    double n3 = b0 + b1;
    double n4 = temp3;
    double n5 = b4;
    double n6 = b3;
    double n7 = b7;

    // A2
    double m0 = n7;
    double m1 = n0;
    double m2 = n4;
    double m3 = n1 + n2;
    double m4 = n3 + n6;
    double m5 = n1 - n2;
    double m6 = n3 - n6;
    double m7 = n5 - n0;

    // A3
    res[0] = m4 + m0;
    res[1] = m3 + m2;
    res[2] = m5 - m1;
    res[3] = m6 - m7;
    res[4] = m6 + m7;
    res[5] = m5 + m1;
    res[6] = m3 - m2;
    res[7] = m4 - m0;
    return;
}

inline void transpose(double mat[8][8]) {
    for(int i=0; i<7; ++i)
        for(int j=i+1; j<8; ++j)
            std::swap(mat[i][j], mat[j][i]);
    return;
}

inline void idct2d(double mat[8][8]) {
    double row[8][8];
    memcpy(row, mat, sizeof(row));
    for(int i=0; i<8; ++i)
        idct(row[i], mat[i]);
    transpose(row);
    for(int i=0; i<8; ++i)
        idct(mat[i], row[i]);
    transpose(mat);
    return;
}

void idct(double dest[8][8], int source[8][8]) {
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			dest[i][j] = (double)source[i][j];
		}
	}
	idct2d(dest);
	// static bool init = false;
	// static double cos_cache[200] = {};
	// if (!init) {
	// 	init = true;
	// 	for (int i = 0; i < 200; i++) {
	// 		cos_cache[i] = cos(i * M_PI / 16.0);
	// 	}
	// }
	// double tmp[8][8] = {};
	// double s[8][8] = {};

	// for (int j = 0; j < 8; j++) {
	// 	for (int x = 0; x < 8; x++) {
	// 		for (int y = 0; y < 8; y++) {
	// 			s[j][x] += c (y) * source[x][y] * cos_cache[(j + j + 1) * y];
	// 		}
	// 		s[j][x] = s[j][x] / 2.0;
	// 	}
	// }
	// for (int i = 0; i < 8; i++) {
	// 	for (int j = 0; j < 8; j++) {
	// 		for (int x = 0; x < 8; x++) {
	// 			tmp[i][j] += c(x) * s[j][x] * cos_cache[(i + i + 1) * x];
	// 		}
	// 		tmp[i][j] = tmp[i][j] / 2.0;
	// 	}
	// }
	// for (int i = 0; i < 8; i++) {
	// 	for (int j = 0; j < 8; j++) {
	// 		dest[i][j] = tmp[i][j];
	// 	}
	// }
}   


unsigned char chomp(double x) {
	if (x > 255.0) {
		return 255;
	} else if (x < 0) {
		return 0;
	} else {
		return (unsigned char) round(x);
	}
}