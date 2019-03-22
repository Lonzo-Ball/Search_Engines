//数据处理模块
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "../common/util.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"


const std::string input_path = "../data/input/";
const std::string output_path = "../data/tmp/raw_input";

//doc 指的是文档，就是待搜索的 html
struct DocInfo{
	std::string title;    //标题
	std::string content;  //正文
	std::string url;      //url
};

bool EnumFile(const std::string& input_path,std::vector<std::string>* file_list){
	
	//input_path 路径下不仅有 .html 文档，还有很多目录和 .png 文件
	//我们需要将目录和 .png 文件过滤掉
	//但是这些目录下也包含我们想要的 .html 文档，所以我们需要递归遍历

	//fs 是一个命名空间
	namespace fs = boost::filesystem;
	//input_path 是 html 文档的路径 本质上是一个字符串，根据这个字符串构造出一个 path 对象
	fs::path root_path(input_path);
	if(!fs::exists(root_path))  //构建失败
	{
		std::cout<< "input_path not exists!" <<std::endl;
		return false;
	}

	//boost 递归遍历目录，借助一个特殊的迭代器即可 构造一个未初始化的迭代器作为遍历结束标记
	fs::recursive_directory_iterator end_iter;
	for(fs::recursive_directory_iterator iter(root_path); iter != end_iter; ++iter)
	{
		if (!fs::is_regular_file(*iter))//目录
			continue;
		if (iter->path().extension() != ".html")//不是.html后缀
			continue;

		file_list->push_back(iter->path().string());
	}
	return true;

}

bool ParseTitle(const std::string& html,std::string* title){
	//先查找 <title> 标签
	size_t begin = html.find("<title>");
	if(begin == std::string::npos)
	{
		std::cout << "<title> not find!" <<std::endl;
		return false;
	}

	//再查找 </title>
	size_t end = html.find("</title>");
	if(end == std::string::npos)
	{
		std::cout<< "</title> not find!" <<std::endl;
		return false;
	}

	begin += std::string("<title>").size();
	if(begin > end)
	{
		std::cout<< "title begin and end failed!" <<std::endl;
		return false;
	}

	*title = html.substr(begin,end-begin);
	return true;
}

bool ParseContent(const std::string& html,std::string* content){
	bool is_content = true;
	for(auto c : html){
		if(is_content)
		{
			//当前为正文状态
			if(c == '<')
			{
				//进入标签状态
				is_content = false;
			}
			else
			{
				//当前字符就是普通的正文字符，需要加入到结果中
				if(c == '\n')
					c = ' ';

				content->push_back(c);
			}
		}
		else
		{
			//当前不是正文状态
			if(c == '>')
				is_content = true;
		}
	}

	return true;
}

bool ParseUrl(const std::string& file_path,std::string* url){
	//https://www.boost.org/doc/libs/1_53_0/doc/
	//url 的后半部分可通过 html 文档路径解析出来
	//home/lonzo/workspace/Project/doc_searcher/data/input/html/Transform.html	
	std::string prefix = "https://www.boost.org/doc/libs/1_53_0/doc/";
	std::string tail = file_path.substr(input_path.size());

	*url = prefix + tail;
	return true;
}

bool ParseFile(const std::string& file_path,DocInfo* doc_info){

	//1.打开文件并读取 html 文件
	std::string html;
	bool ret = FileUtil::Read(file_path,&html);
	if(!ret)
	{
		std::cout<< "Read file_path failed!" <<std::endl;
		return false;	
	}

	//2.解析标题
	ret = ParseTitle(html,&doc_info->title);
	if(!ret)
	{
		std::cout<< "ParseTile failed!" <<std::endl;
		return false;	
	}	

	//3.解析正文，并出去 html 标签
	ret = ParseContent(html,&doc_info->content);
	if(!ret)
	{
		std::cout<< "ParseContent failed!" <<std::endl;
		return false;
	}

	//4.解析 url
	ret = ParseUrl(file_path,&doc_info->url);
	if(!ret)
	{
		std::cout<< "ParseUrl failed!" <<std::endl;
		return false;
	}

	return true;	

}

bool WriteOutput(const DocInfo& doc_info,std::ofstream& file){
	std::string line = doc_info.title + "\3" + doc_info.url + "\3" + doc_info.content + "\n";
	file.write(line.c_str(),line.size());

	return true;
}

int main(){

	//第一步：枚举出所有 html 文档的路径 vector 中存放的是 html 文档的路径 相当于一个路径池
	std::vector<std::string> file_list;
	//EnumFile 这个函数用于构造路径池
	bool ret = EnumFile(input_path,&file_list);
	if(!ret)
	{
		std::cout<< "EnumFile failed!" <<std::endl;
		exit(1);
	}

	//打印路径
	for(const auto& path : file_list ){
		std::cout << path <<std::endl;
	}

	//打开输出文件
	std::ofstream output_file(output_path.c_str());
	if(!output_file.is_open())
	{
		std::cout<< "open output_file failed!" <<std::endl;
		exit(2);
	}

	//第二步：依次处理每个枚举出的路径，对该路径下的文档进行分析,分析出文件的标题、url、正文并且进行去标签
	//auto 推导出的类型是 string
	for(const auto& path : file_list){
		//每次进来，会创建一个 DocInfo 对象，对应从头至尾的一个个 html 文件
		DocInfo info;
		ret = ParseFile(path,&info);
		if(!ret)
		{
			std::cout<< "ParseFile failed!" <<std::endl;
			continue;
		}
		
	//第三步：把分析结果按照一行的形式写入到输出文件中
		WriteOutput(info,output_file);
	}

	output_file.close();
	return 0;

}
