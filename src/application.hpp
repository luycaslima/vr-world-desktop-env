#ifndef APPLICATION_HPP
#define APPLICATION_HPP

//#include "base/base.h"
#include <memory>
#include <string>

class Application {
    private:
    //std::unique_ptr<Context> ctx;
    std::string  name;

    public:
    Application();
    ~Application();
	int Init();
	int Update();
	int Draw();
	void Quit();
};

#endif