#include "searcher.h"

//在这个文件中先调用 Index 模块，进行验证
#if 0
int main(){
	searcher::Index index;
	bool ret = index.Build("../data/tmp/raw_input");
	if(!ret){
		std::cout<< "Build failed!" <<std::endl;
		return 1;
	}

	auto* inverted_list = index.GetInvertedList("filesystem");
	if(inverted_list == NULL){
		std::cout<< "GetInvertedList failed!" <<std::endl;
		return 2;
	}

	for(auto& weight : *inverted_list){
		std::cout<< "doc_id: " << weight.doc_id << " weigth: " << weight.weigth << " key: " << weight.key <<std::endl;  
	}
	return 0;
}
#endif

//调用 Search 模块，进行验证
int main(){
	
	searcher::Searcher searcher;
	bool ret = searcher.Init("../data/tmp/raw_input");
	if(!ret)
	{
		std::cout << "Init failed!" << std::endl;
		return 1;
	}

	std::string query = "filesystem";
	std::string result;
	searcher.Search(query,&result);
	std::cout << result <<std::endl;
	return 0;
}
