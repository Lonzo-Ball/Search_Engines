#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <boost/algorithm/string.hpp>

class FileUtil{
public:
	//这个函数首先打开文件然后将其内容拼接到 content 中
	static bool Read(const std::string& file_path,std::string* file_){
		std::ifstream file(file_path.c_str());
		if(!file.is_open())
		{
			std::cout<< "file open failed!" <<std::endl;
			return false;
		}

		//按行读取内容
		std::string line;
		while(std::getline(file,line)){
			*file_ += line + "\n";
		}

		file.close();
		return true;
	}

	//
	static bool Write(std::string& file_path,const std::string& content){
		std::ofstream file(file_path.c_str());
		if(!file.is_open())
		{
			std::cout<< "open file failed!" <<std::endl;
			return false;
		}

		file.write(content.c_str(),content.size());

		file.close();
		return true;
	}
};

class StringUtil{
public:
	//基于 boost 的字符串切分
	static void Split(const std::string& input,std::vector<std::string>* output,const std::string& split_char){
									//关闭压缩
		boost::split(*output,input,boost::is_any_of(split_char),boost::token_compress_off);
		return;
	}	
};























