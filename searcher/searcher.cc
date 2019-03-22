#include "searcher.h"

namespace searcher{

const char* const DICT_PATH = "../jieba_dict/jieba.dict.utf8";
const char* const HMM_PATH = "../jieba_dict/hmm_model.utf8";
const char* const USER_DICT_PATH = "../jieba_dict/user.dict.utf8";
const char* const IDF_PATH = "../jieba_dict/idf.utf8";
const char* const STOP_WORD_PATH = "../jieba_dict/stop_words.utf8";

//用词典文件初始化 jieba 对象
Index::Index():jieba(DICT_PATH,HMM_PATH,USER_DICT_PATH,IDF_PATH,STOP_WORD_PATH)
{}

//读取 raw_input 文件，在内存中构建索引
bool Index::Build(const std::string& input_path)
{
	//1.按行读取文件内容，每一行对应一个文件
	std::ifstream file(input_path.c_str());
	if(!file.is_open())
	{
		std::cout<< "file open failed!" <<std::endl;
		return false;
	}

	//每次按行读取的内容不包含结尾的"\n"
	std::string line;
	while(std::getline(file,line)){
		//2.构造 DocInfo 对象 更新正排索引数据
		//对读到的一行文件进行解析，得到 DocInfo 对象再插入到 vector 中
		const DocInfo* doc_info = BuildForward(line);

		//3.更新倒排索引数据
		BuildInverted(*doc_info);
	}
	return true;
}

//查正排：给定 id 找到文档内容
const DocInfo* Index::GetDocInfo(uint64_t doc_id) const
{
	if(doc_id >= forward_index.size())
	{
		std::cout<< "doc_id is error!" <<std::endl;
		return NULL;
	}

	return &forward_index[doc_id];
}
//查倒排：给定词，找到这个词在 哪些文档中出现过 返回的是一个倒排拉链 std::vector<Weight>
const InvertedList* Index::GetInvertedList(const std::string key) const
{
	auto pos = inverted_index.find(key);
	if(pos == inverted_index.end())
	{
		std::cout<< "not key!" <<std::endl;
		return NULL;
	}

	return &pos->second;
}

//构建正排索引
const  DocInfo* Index::BuildForward(const std::string& line)
{
	//1.对这一行内容进行切分 \3
	std::vector<std::string> tokens;  //存放切分结果
	//借助boost来进行切分
	StringUtil::Split(line,&tokens,"\3");
	if(tokens.size() != 3)
	{
		std::cout<< "tokens not ok!" <<std::endl;
		return NULL;
	}

	//2.构造一个 DocInfo 对象
	DocInfo doc_info;
	doc_info.doc_id = forward_index.size();
	doc_info.title = tokens[0];
	doc_info.url = tokens[1];
	doc_info.content = tokens[2];

	//3.把这个结果插入到正排索引中
	forward_index.push_back(doc_info);

	return &forward_index.back();
}

//构建倒排索引
const void Index::BuildInverted(const DocInfo& doc_info)
{
	//1.先对当前的 doc_info 进行分词 正文&标题
	std::vector<std::string> title_tokens;
	CutWord(doc_info.title,&title_tokens);

	std::vector<std::string> content_tokens;
	CutWord(doc_info.content,&content_tokens);
	//2.对 doc_info 中的标题和正文进行词频统计
	//当前词在标题中出现几次 在正文中出现几次
	struct WordCnt{
		int title_cnt;
		int content_cnt;
	};

	//用一个哈希表完成词频统计
	std::unordered_map<std::string,WordCnt> word_cnt;
	for(std::string word : title_tokens){
		//统计词频是忽略大小写
		boost::to_lower(word);
		++word_cnt[word].title_cnt;
	}

	for(std::string word : content_tokens){
		boost::to_lower(word);
		++word_cnt[word].content_cnt;
	}
	
	//3.遍历分词结果，在倒排索引中查找
	for(const auto& word_pair : word_cnt){
		Weight weight;
		weight.doc_id = doc_info.doc_id;
		weight.weigth = 10*word_pair.second.title_cnt + word_pair.second.content_cnt;
		weight.key = word_pair.first;

		//4.如果该分词在倒排中不存在，就构建新的键值对
		//5.如果该分词结果在倒排中存在，找到对应的值（vector）构建一个新的 Weight 插入到 vector 中
		InvertedList& invertedlist = inverted_index[weight.key];
		invertedlist.push_back(weight);
	}	
}

void  Index::CutWord(const std::string& input,std::vector<std::string>* output){

	jieba.CutForSearch(input,*output);	
}


//搜索模块函数
bool Searcher::Init(const std::string& input_path){
	return index->Build(input_path);
}

bool Searcher::Search(const std::string& query,std::string* json_result){
	//1.分词，对搜索的字符串进行分词
	std::vector<std::string> tokens;
	index->CutWord(query,&tokens);

	//2.触发，对个分词结果在倒排索引中查找
	std::vector<Weight> all_token_result;
	for(std::string word : tokens){
		boost::to_lower(word);
		auto* inverted_list = index->GetInvertedList(word);
		if(inverted_list == NULL){  //说明当前词在倒排索引中不在，跳过即可
			continue;  
		}

		all_token_result.insert(all_token_result.end(),inverted_list->begin(),inverted_list->end());
	}

	//3.排序，根据 all_token_result 中每个元素的权重排序
	//sort 的第三个参数可以使用 仿函数/函数指针/lambda 表达式
	//lambda 表达式：匿名函数 作为比较函数
	std::sort(all_token_result.begin(),all_token_result.end(),[](const Weight& w1,const Weight& w2) \
		 {return w1.weigth > w2.weigth;});

	//4.构造结果，根据排序的先后顺序 查找正排所引 按 json 格式输出最终的内容
	Json::Value results;
	for(const auto& weight : all_token_result){
		const auto& doc_info = index->GetDocInfo(weight.doc_id);
		if(doc_info == NULL){
			continue;
		}

		Json::Value result;
		result["title"] = doc_info->title;
		result["url"] = doc_info->url;
		result["desc"] = MakeDesc(doc_info->content,weight.key);  //构建相对比较恰当的摘要
		results.append(result);
	}

	Json::FastWriter writer;
	*json_result = writer.write(results);
	return true;
}

std::string Searcher::MakeDesc(const std::string& content,const std::string& key){
		size_t pos = content.find(key);
		if(pos == std::string::npos)
		{
			if(content.size() < 80)
			{
				return content.substr(content.size());
			}
			
			return content.substr(0,80) + "...";
		}

		size_t begin = pos < 30 ? 0 : pos - 30;
		if(begin + 80 >= content.size())	
		{
			return content.substr(begin) + "...";
		}
		else
		{
			return content.substr(begin,80) + "...";
		}
}

}  //end searcher
