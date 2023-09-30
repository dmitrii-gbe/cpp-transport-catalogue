#pragma once
#include <string>
#include <vector>
#include "json.h"



namespace json {

    class StartArrayItemContext;
    class StartDictItemContext;
    class KeyItemContext;
    
    class Builder {
    
        public:
            Builder();

            enum class Method {
                Start,
                Key,
                StartDict,
                StartArray,
                EndDict,
                EndArray,
                Value
            };

            Builder& Value(Node value);

            Builder& Key(std::string str);

            StartDictItemContext StartDict();

            Builder& EndDict();

            StartArrayItemContext StartArray();

            Builder& EndArray();

            json::Node Build();     
        
        private:

            bool CheckIfKeyIsInsideOfDict();

            bool CheckIfElementIsInsideOfArray();

            json::Node root_;
            std::vector<Method> queue_;
            Array tmp_;
    };

    class BaseClass {
        public: 

        BaseClass(Builder& builder);

            KeyItemContext Key(std::string str);

            StartDictItemContext StartDict();

            Builder& EndDict();          

            StartArrayItemContext StartArray();                

            Builder& EndArray();           

            json::Node Build();

        protected:

        Builder& builder_;
    };

    class KeyItemContext : public BaseClass {

            using BaseClass::BaseClass;

            KeyItemContext Key(std::string str) = delete;

            Builder& EndDict() = delete;

            Builder& EndArray() = delete;

            json::Node Build() = delete;

            public:

            StartDictItemContext Value(Node value);

    };

    class StartDictItemContext : public BaseClass {

            using BaseClass::BaseClass;

            Builder& EndArray() = delete;

            StartDictItemContext StartDict() = delete;

            StartArrayItemContext StartArray() = delete;

            json::Node Build() = delete;

    };

    class StartArrayItemContext : public BaseClass {

            using BaseClass::BaseClass;

            Builder& EndDict() = delete;

            KeyItemContext Key() = delete;

            json::Node Build() = delete;

            public:

            StartArrayItemContext Value(Node value);

    };    
}