#pragma once
//构建索引模块和搜索模块

#include <string>
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <jsoncpp/json/json.h>
#include "/home/lonzo/workspace/Project/cppjieba/include/cppjieba/Jieba.hpp"
#include "../common/util.hpp"

namespace searcher{
struct DocInfo{
	uint64_t doc_id;
	std::string title;
	std::string content;
	std::string url;
};
struct Weight{
	uint64_t doc_id;
	int weigth;  //权重，排序用

	std::string key;
};

//类型重命名
typedef std::vector<Weight> InvertedList;
//索引模块
class Index{
private:
	//正排索引：知道 id 获取到对应文档内容 使用 vector 下标表示文档 id
	std::vector<DocInfo> forward_index;
	//倒排索引：知道词，获取到对应文档 id 列表
	//Weight 的第一个成员就是文档 id  unordered_map 底层是哈希表
	//typedef std::vector<Weight> InvertedList;
	std::unordered_map<std::string,InvertedList> inverted_index;

	cppjieba::Jieba jieba;  //使用外部的结巴分词
public:
	Index();

	//读取 raw_input 文件，在内存中构建索引
	bool Build(const std::string& input_path);
	//查正排：给定 id 找到文档内容
	const DocInfo* GetDocInfo(uint64_t doc_id) const;
	//查倒排：给定词，找到这个词在 哪些 文档中出现过
	const InvertedList* GetInvertedList(const std::string key) const;

	void  CutWord(const std::string& input,std::vector<std::string>* output);
private:
	const  DocInfo* BuildForward(const std::string& line);

	const void BuildInverted(const DocInfo& doc_info);
}; //end index

//搜索模块
}  //end searcher
