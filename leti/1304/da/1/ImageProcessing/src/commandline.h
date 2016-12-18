//
//  commandline.h
//  ImageProcessing
//
//  Created by Anton Davydov on 18/12/2016.
//
//

#ifndef commandline_h
#define commandline_h

#include <string>
#include <map>
#include <iostream>
using namespace std;

/**
 CommandLineParser allows to define list of expected and optional parameters of CLI.
 It validates by given parameters and returns template argument as output object.
 Can throw a stringified error if validation fails.
 */
template<typename T>
class CommandLineParser {
protected:
    map<string, function<void(T&, string) throw(string)>> mandatoryRules;
    map<string, function<void(T&, string) throw(string)>> optionalRules;
    
    map<string, function<void(T&)>> mandatoryFlagRules;
    map<string, function<void(T&)>> optionalFlagRules;
    
public:
    
    /**
     Add a mandatory field with value to stored list. Return self.
     Parameters:
        @key - CLI key, for example --input.
        @parsing - a closure to parse expected value by given CLI arg. Can throw an error if parsing fails.
        @binding - use to bind a parsed argument to your CLI model.
     */
    template<typename R>
    CommandLineParser<T>& mandatory(string key, R(*parsing)(string) throw(string), void(*binding)(T&, const R&) ) {
        
        this->mandatoryRules[key] = [parsing, binding](T& r, string value) throw(string) {
            auto parsed = parsing(value);
            binding(r, parsed);
        };
        
        return *this;
    }
    
    /**
     Add an optional field if needed. Return self.
     Parameters:
        @key - CLI key, for example --input.
        @parsing - a closure to parse expected value by given CLI arg. Can throw an error if parsing fails.
        @binding - use to bind a parsed argument to your CLI model.
     */
    template<typename R>
    CommandLineParser<T>& optional(string key, R(*parsing)(string v) throw(string), void(*binding)(T&, R)) {
        
        this->optionalRules[key] = [parsing, binding](T& r, string value) throw(string) {
            binding(r, parsing(value));
        };

        return *this;
    }

    /**
     Add a mandatory flag to stored list. Return self.
     Parameters:
        @key - CLI key, for example --verbose.
        @binding - use to bind a parsed argument to your CLI model.
     */
    CommandLineParser<T>& mandatoryFlag(string key, void(*binding)(T&)) {
        this->mandatoryFlagRules[key] = binding;
    
        return *this;
    }
    
    /**
     Add an optional flag to stored list. Return self.
     Parameters:
        @key - CLI key, for example --verbose.
        @binding - use to bind a parsed argument to your CLI model.
     */
    CommandLineParser<T>& optionalFlag(string key, void(*binding)(T&)) {
        this->optionalFlagRules[key] = binding;
        
        return *this;
    }
    
    /**
     Check and parse CLI arguments.
     Bind all parameters to defined CLI model.
     Return CLI model if parsind/binding finishes successfully. Throw an error if it fails.
     */
    T validate(int argc, const char * argv[]) throw(string) {
        auto result = T();

        for (int i = 1; i < argc; ) {
            auto key = string(argv[i]);
            if (mandatoryRules.find(key) != mandatoryRules.end()) {
                if (i+1 >= argc)
                    throw string("Value not found for key " + key);
                auto rule = mandatoryRules.find(key)->second;
                auto value = string(argv[i+1]);
                rule(result, value);
                mandatoryRules.erase(key);
                i+=2;
            } else if (mandatoryFlagRules.find(key) != mandatoryFlagRules.end()) {
                auto rule = mandatoryFlagRules.find(key)->second;
                rule(result);
                mandatoryFlagRules.erase(key);
                i++;
            } else if (optionalRules.find(key) != optionalRules.end()) {
                try {
                    if (i+1 >= argc)
                        throw string("Value not found for key " + key);
                    auto rule = optionalRules.find(key)->second;
                    rule(result, string(argv[i+1]));
                    optionalRules.erase(key);
                } catch (string e) { }
                i+=2;
            } else if (optionalFlagRules.find(key) != optionalFlagRules.end()) {
                try {
                    auto rule = optionalFlagRules.find(key)->second;
                    rule(result);
                    optionalFlagRules.erase(key);
                } catch (string e) {}
                i++;
            } else {
                throw string("Unknown arg " + key);
            }
        }
        
        for(auto it = mandatoryRules.begin(); it != mandatoryRules.end(); it++) {
            throw string("Key " + it->first + " not found");
        }
        
        for(auto it = mandatoryFlagRules.begin(); it != mandatoryFlagRules.end(); it++) {
            throw string("Key " + it->first + " not found");
        }
        
        return result;
    }
};

#endif /* commandline_h */
