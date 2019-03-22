//在这个文件中调用 Index 模块，进行验证
#include "searcher.h"

int main(){
	
	searcher::Index index;
	bool ret = index.Build("../data/tmp/raw_input");
	if(!ret)
	{
		std::cout<< "Build failed!" <<std::endl;
		return 1;
	}

	auto* inverted_list = index.GetInvertedList("filesystem");
	if(inverted_list == NULL)
	{
		std::cout<< "GetInvertedList failed!" <<std::endl;
		return 2;
	}

	for(auto& weight : *inverted_list)
	{
		std::cout<< "doc_id: " << weight.doc_id << " weigth: " << weight.weigth << " key: " << weight.key <<std::endl;  
	}
	return 0;
}
