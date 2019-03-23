#include "searcher.h"
#include "httplib.h"

int main(){
	//1.创建 searcher 对象并进行初始化
        searcher::Searcher searcher;
        bool ret = searcher.Init("../data/tmp/raw_input");
        if(!ret)
        {
                std::cout << "searcher Init failed!" << std::endl;
                return 1;
        }

        //2.创建服务器
        using namespace httplib;
	Server server;
	//search?query=filesystem
	//&s 
	server.Get("/search",[&s](const Request& req,Response& res) \
		  {
			std::string query = req.get_param_value("query");
			std::string result;
			s.Search(query,&result);
			res.set_content(result,"text/plain");
		  });
	server.listen("0,0,0,0",8080);
        return 0;
}
