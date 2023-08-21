#include<string>
#include<iostream>
#include <sstream>
int main(){
    std::string header("aaaa/bbbb");
    uint32_t a = header.size();
    std::cout<<a<<std::endl;

    std::cout<<sizeof('a')<<std::endl;
    char ab='a';
    std::cout<<sizeof(ab)<<std::endl;

    auto str = std::to_string(a);
    const char* str2 = str.c_str();
    std::cout<<str2<<std::endl;

    std::stringstream ss;
    ss << a;
    std::string sss = ss.str();
     
    //auto b = std::string((char*)&a,4);

    auto c = std::string("abcdefghjfalhdufioa",2,5);  //从下标2开始，往后五个字符
    auto d = std::string("abcdefg",4);  //从0开始，四个字符
    std::cout<<c<<std::endl;
    std::cout<<d<<std::endl;


    uint32_t n = 65;
    uint64_t z = 68;
    std::cout<<sizeof(n)<<std::endl;
    std::cout<<sizeof(z)<<std::endl;
    std::string m;
    auto x =std::string((char*)&z, 1);
    std::cout<<x<<std::endl;

    m.insert(0, std::string((char*)&n, 4));
    std::cout<<m<<std::endl;
    return 0;
}