#pragma once
#include <string>
#include <vector>
#include "json.h"



namespace json {

    class StartArrayItemContext;
    class StartDictItemContext;
    class KeyItemContext;
    class ValueItemContext;
    class ValueItemContext1;
    
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

            KeyItemContext Key(std::string str);

            StartDictItemContext StartDict();

            Builder& EndDict();

            StartArrayItemContext StartArray();

            Builder& EndArray();

            json::Node Build();     
        
        private:

            bool CheckIfKeyIsInsideOfDict();

            bool CheckIfElementIsInsideOfArray();

            json::Node root_;
            std::vector<std::string> tmp_key_;
            std::vector<Method> queue_;
            std::vector<Dict> tmp_dict_;
            std::vector<Array> tmp_array_;
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

            ValueItemContext Value(Node value);

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

            ValueItemContext1 Value(Node value);

    };

    

    class ValueItemContext : public BaseClass {

            using BaseClass::BaseClass;

            StartDictItemContext StartDict() = delete;      

            StartArrayItemContext StartArray() = delete;                

            Builder& EndArray() = delete;           

            json::Node Build() = delete;

    };

    class ValueItemContext1 : public BaseClass {

        using BaseClass::BaseClass;

        Builder& EndDict() = delete;

        KeyItemContext Key() = delete;

        json::Node Build() = delete;

        public:

        ValueItemContext1 Value(Node value);

    };
    
}