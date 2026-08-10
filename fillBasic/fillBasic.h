#ifndef __FILLBASIC_H__
#define __FILLBASIC_H__
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
int hstoi(std::string __str);
/* struct _basic
{
    int value;
    void clear(){ value = 0; }
    void show(){ std::cout << "value:(" << value << ")" << std::endl; }
    bool setField(std::string _secondLevel,std::string _field, const std::string& _value){
        try{
            if(_field == "value") { value = std::stoi(_value); }
            else { return false; }
        } catch(...){
            return false;
        }
        return true;
    }
};
*/
struct s42_Basic
{
    int uid;
    std::string name;
    std::string description;
    std::string bus;
    int bus_address;
    void clear(){
        uid = 0;
        name.clear();
        description.clear();
        bus.clear();
        bus_address = 0;
    }   
    void show(){
        std::cout << "s42_Basic: " <<
                " uid:(" << uid << ")"<<
                " name:(" << name << ")" << 
                "[" << description << "]" <<
                " bus:" << bus << ":" << bus_address << std::endl;
    }
    bool setField(std::string _secondLevel,std::string _field, const std::string& _value){
        try{
            if(_field == "uid") { uid = hstoi(_value); }
            else if(_field == "name") { name = _value; }
            else if(_field == "description") { description = _value; }
            else if(_field == "bus") { bus = _value; }
            else if(_field == "bus_address") { bus_address = hstoi(_value); }
            else { return false; }
        } catch(...){
            return false;
        }
        return true;
    }
};
struct p26_Basic
{
    int uid;
    std::string name;
    std::string description;
    std::string bus;
    int bus_address;
    int var1;
    int var2;
    void clear(){
        uid = 0;
        name.clear();
        description.clear();
        bus.clear();
        bus_address = 0;
        var1 = 0;
        var2 = 0;
    }
    void show(){
        std::cout << "p26_Basic: " <<
                " uid:(" << uid << ")" << 
                " name:(" << name << ")" <<
                " [" << description << "]" <<
                " bus:" << bus << ":" << bus_address <<
                " var1:" << var1 <<
                " var2:" << var2 << std::endl;
    }
    bool setField(std::string _secondLevel,std::string _field, const std::string& _value){
        try{
            if(_field == "uid") { uid = hstoi(_value); }
            else if(_field == "name") { name = _value; }
            else if(_field == "description") { description = _value; }
            else if(_field == "bus") { bus = _value; }
            else if(_field == "bus_address") { bus_address = hstoi(_value); }
            else if(_field == "var1") { var1 = hstoi(_value); }
            else if(_field == "var2") { var2 = hstoi(_value); }
            else { return false; }
        } catch(...){
            return false;
        }
        return true;
    }
};
struct arm_Basic
{
    int uid;
    std::string name;
    std::string description;
    s42_Basic myM;
    p26_Basic myP;
    void clear(){
        uid = 0;
        name.clear();
        description.clear();
        myM.clear();
        myP.clear();
    }
    void show(){
        std::cout << "arm_Basic: " <<
                " uid:(" << uid << ")" <<
                " name:(" << name << ")" <<
                "[" << description << "]" << std::endl;
        std::cout << "    myM: "; myM.show();
        std::cout << "    myP: "; myP.show();
    }
    bool setField(std::string _secondLevel,std::string _field, const std::string& _value){
        try{
            if (_secondLevel.empty()) { 
                if(_field == "uid") { uid = hstoi(_value); }
                else if(_field == "name") { name = _value; }
                else if(_field == "description") { description = _value; }
                else { return false; }
            }else{
                if(_secondLevel == "S42") { return myM.setField(_secondLevel, _field, _value); }
                else if(_secondLevel == "P26") { return myP.setField(_secondLevel, _field, _value); }
                else { return false; }                
            }
            
        } catch(...){
            return false;
        }
        return true;
    }
};


class fillBasic
{
public:
    template <typename T>    
    bool getBasicData(const std::string& configFile, const std::string& name, std::vector<T>& data){

        std::ifstream ifs(configFile);
        if(!ifs) return false;

        auto trim = [](std::string s){
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch){ return !std::isspace(ch); }));
            s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch){ return !std::isspace(ch); }).base(), s.end());
            return s;
        };

        std::string line;
        std::string target = "[" + name + "]";
        
        bool inSection = false;
        std::string level1, level2, currentLevel;

        T current{};
        bool hasAnyField = false;

        while(std::getline(ifs, line)){            
            std::string s = trim(line);
            //std::cout << "Line: " << s << std::endl;
            if(s.empty()){ continue; }
            if(s.size() && s[0] == '#') continue;
            if(s.size() && s.front() == '[' && s.back() == ']'){                
                if(s.rfind("[", 0) == 0){                    
                    if(s.rfind("[.", 0) == 0)                   
                    {                        
                        if(!level1.empty()){
                            level2 = s.substr(2, s.size() - 3);
                            inSection = true; 
                            currentLevel.clear();          
                        }                        
                    }
                    else
                    {
                        if(inSection && hasAnyField){ 
                            data.push_back(current); 
                            current = T{}; 
                            hasAnyField = false; level1.clear(); level2.clear(); currentLevel.clear(); 
                        }
                        if(s == target){ 
                            level1 = s.substr(1, s.size() - 2);
                            inSection = true; 
                            currentLevel.clear();                          
                            level2.clear(); // Reset level2 when a new level1 section is found           
                        }
                        else{
                            inSection = false;
                            level1.clear(); level2.clear(); currentLevel.clear();
                        }                        
                    }
                    if (!level1.empty()){
                        inSection = true;
                        currentLevel = level1;
                        if(!level2.empty()){
                            currentLevel += "." + level2;
                        }   
                    }
                }
                else{
                    inSection = false;
                }                
                continue;
            }
            if(!inSection) continue;

            auto pos = s.find('=');
            if(pos == std::string::npos) continue;
            std::string key = trim(s.substr(0,pos));
            std::string val = trim(s.substr(pos+1));
            if (level2.empty()){
                if(current.setField("", key, val)) { hasAnyField = true; } 
            }else{
                if(current.setField(level2, key, val)) { hasAnyField = true; }
            }
        }
        if(inSection && hasAnyField){ data.push_back(current); }
        return !data.empty();
    }
};

int hstoi(std::string __str) {
    try{
        if(__str.rfind("0X", 0) == 0 || __str.rfind("0x", 0) == 0){
            return std::stoi(__str, nullptr, 16);
        }else{
            return std::stoi(__str);
        }
    } catch(...){
        return 0;
    }
}
#endif