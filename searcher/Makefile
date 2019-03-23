#FLAG=-std=c++11 -lboost_filesystem -lboost_system -I ~/third_part/include -ljsoncpp

#test:test.cc searcher.h searcher.cc
#	g++ test.cc searcher.cc -o test $(FLAG)

#libsearcher.so:searcher.cc searcher.h
#	g++ searcher.cc -shared -fPIC -o libsearcher.so $(FLAG)



FLAG=-std=c++11 -lpthread -I ~/third_part/include
httpserver:httpserver.cpp libsearcher.so                                                              
	g++ httpserver.cpp libsearcher.so -o httpsever $(FLAG)
