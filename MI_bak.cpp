#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <direct.h>
#include <algorithm>
#include <assert.h>
#include <sstream>
#include <io.h>
#include <libloaderapi.h>


#include <string>
typedef int(*Dllfun)(void);

using namespace std;
const std::string& HEX_2_NUM_MAP()
{
	static const std::string str("0123456789ABCDEF");
	return str;
}
unsigned char NUM_2_HEX(const char h, const char l)
{
	unsigned char hh = std::find(std::begin(HEX_2_NUM_MAP()), std::end(HEX_2_NUM_MAP()), h) - std::begin(HEX_2_NUM_MAP());
	unsigned char ll = std::find(std::begin(HEX_2_NUM_MAP()), std::end(HEX_2_NUM_MAP()), l) - std::begin(HEX_2_NUM_MAP());
	return (hh << 4) + ll;
}



void EncodeHexString(unsigned char* data, int len, char** out)
{
	static const char hex_digits[] = "0123456789ABCDEF";

	std::string output;
	output.reserve(len * 2);
	for (int i = 0; i < len; i++) {
		output.push_back(hex_digits[data[i] >> 4]);
		output.push_back(hex_digits[data[i] & 15]);
	}
	*out = new char[output.length() + 1];
	strcpy(*out, output.c_str());
}

//base64 转16进制
int DecodeBase64Char(unsigned int ch) {

	if (ch >= 'A' && ch <= 'Z') return ch - 'A' + 0;   // 0 range starts at 'A'
	if (ch >= 'a' && ch <= 'z') return ch - 'a' + 26;  // 26 range starts at 'a'
	if (ch >= '0' && ch <= '9') return ch - '0' + 52;  // 52 range starts at '0'
	if (ch == '+') return 62;
	if (ch == '/') return 63;
	return -1;
}

bool Base64Decode(const char* szSrc, int nSrcLen, unsigned char* pbDest,
	int* pnDestLen) {


	if (szSrc == NULL || pnDestLen == NULL) {
		assert(false);
		return false;
	}

	const char* szSrcEnd = szSrc + nSrcLen;
	int nWritten = 0;

	bool bOverflow = (pbDest == NULL) ? true : false;

	while (szSrc < szSrcEnd && (*szSrc) != 0) {
		unsigned int dwCurr = 0;
		int i;
		int nBits = 0;
		for (i = 0; i < 4; i++) {
			if (szSrc >= szSrcEnd) break;
			int nCh = DecodeBase64Char(*szSrc);
			szSrc++;
			if (nCh == -1) {
				// skip this char
				i--;
				continue;
			}
			dwCurr <<= 6;
			dwCurr |= nCh;
			nBits += 6;
		}

		if (!bOverflow && nWritten + (nBits / 8) > (*pnDestLen)) bOverflow = true;


		dwCurr <<= 24 - nBits;
		for (i = 0; i < nBits / 8; i++) {
			if (!bOverflow) {
				*pbDest = (unsigned char)((dwCurr & 0x00ff0000) >> 16);
				pbDest++;
			}
			dwCurr <<= 8;
			nWritten++;
		}
	}

	*pnDestLen = nWritten;

	if (bOverflow) {
		if (pbDest != NULL) {
			assert(false);
		}
		return false;
	}

	return true;
}

int Base64ToHex(const char* base64In, char** HexOut) {
	int nDecodeDataSize = 0;
	unsigned char* pDecodedData = NULL;
	bool bsuccess = false;
	nDecodeDataSize = (int)strlen(base64In);
	pDecodedData = new unsigned char[nDecodeDataSize];
	memset(pDecodedData, 0, sizeof(unsigned char) * nDecodeDataSize);

	bsuccess = Base64Decode(base64In, (int)strlen(base64In), pDecodedData,
		&nDecodeDataSize);
	if (!bsuccess) {
		return -1;
	}

	EncodeHexString(pDecodedData, nDecodeDataSize, HexOut);

	free(pDecodedData);

	return 0;
}


int char2num(char ch) /*将字符转成数字*/
{
	if (ch >= 'a')
		return ch - 'a' + 10;
	else if (ch >= 'A')
		return ch - 'A' + 10;
	else
		return ch - '0';
}
std::string Decode_2(const std::string& url)
{
	std::string ret;
	int len = url.length();
	int i;


	for (int i = 0; i < len; i += 2) {
		//if (i == 0) {
		//	continue;
		//}
		//else {
		ret.push_back(NUM_2_HEX(url[i], url[i + 1]));
		//}
	}

	return ret;
}




string to_str(char* bytes) {

	return string(bytes, strlen(bytes));

}

void create_path_if_not_exists(const std::string& path) {

	string sFullPath = path;
	string sNewFullPath = sFullPath.substr(0, sFullPath.find_last_of("\\") + 1);
	//判断前面的路径是否存在，不存在则创建
	size_t index = sFullPath.find("\\", 0);//得到第一级路径的索引
	if (index > sFullPath.length()) return;
	while ((index = sFullPath.find("\\", index + 1)) < sFullPath.length())
	{
		string tmpPath = sFullPath.substr(0, index);
		if (_access(tmpPath.c_str(), 0) != 0)
		{
			if (_mkdir(tmpPath.c_str()) != 0) break;
		}
	}
	//重命名文件夹
	if (_access(sFullPath.c_str(), 0) == 0)
	{
		int ret = rename(sFullPath.c_str(), sNewFullPath.c_str());
	}
	else
	{
		int ret = _mkdir(sNewFullPath.c_str());
	}

}



void unpack_one_file(string outdir, ifstream& infile) {

	string headers;
	getline(infile, headers);
	char* headers_c = new char[headers.length() + 1];
	strcpy(headers_c, headers.c_str());

	char* name_c = new char[headers.length() + 1];
	strcpy(name_c, headers.substr(0, headers.find(' ')).c_str());


	string tempname = to_str(name_c);

	const char* base64String = tempname.c_str();
	char* hexString = NULL;
	Base64ToHex(base64String, &hexString);



	string name = Decode_2(hexString);

	delete[] name_c;

	int fsize = stoi(headers.substr(headers.find(' ') + 1));
	cout << "get file: " << name << "; size: " << fsize << "B" << endl;


	if (outdir.find("\\") != std::string::npos || outdir.find(":") != std::string::npos)
	{
		std::replace(name.begin(), name.end(), '/', '\\');
		if (!outdir.empty() && outdir.back() != '\\')
		{
			outdir += "\\";
		}
	}
	else {
		if (outdir.back() != '/') {
			outdir += "/";
		}
	}

	// 文件输出全路径
	string fname = outdir + name;
	create_path_if_not_exists(fname);
	ofstream outfile(fname, ios::binary);
	char* buffer = new char[fsize];
	infile.read(buffer, fsize);
	outfile.write(buffer, fsize);
	outfile.flush();
	outfile.close();
	delete[] buffer;
	delete[] headers_c;
}
int main(int argc, char* argv[]) {

	//string infilepath = argv[1];
	//string outdir = argv[2];

	string infilepath = "E:\\tmep\\2023-05-16-15-15-01-小米 12 X\\4-微信(com.tencent.mm)0.baksd_pak";
	//string infilepath = "E:\\tmep\\2023-05-16-15-15-01-小米%2012%20X\\4-微信(com.tencent.mm)0.baksd_pak";
	string outdir = "C:\\ 为了贼快\\Mi\\交换\\FileSystem\\wxcache";
	cout << "start unpack: " << infilepath << endl;
	ifstream infile(infilepath, ios::binary);
	infile.seekg(0, ios::end);
	int infilesize = infile.tellg();
	infile.seekg(0, ios::beg);

	try {
		while (true) {
			if (infilesize == infile.tellg()) {
				cout << "finish" << endl;
				break;
			}
			unpack_one_file(outdir, infile);
		}
	}
	catch (exception e) {
		cout << "This file format is corrupted and cannot continue unpacking" << endl;
	}

	return 0;


}