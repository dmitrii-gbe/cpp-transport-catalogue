#include "json_builder.h"

#include <string>
#include <vector>
#include <algorithm>




using namespace json;
    

        Builder::Builder() : queue_{Builder::Method::Start}{}

         Builder& Builder::Value(Node value){
                if (queue_.empty()){
                    throw std::logic_error("The_object_is_complete");
                }
                if (queue_.back() == Builder::Method::StartDict){
                    throw std::logic_error("Invalid_Value_Builder::Method_Invocation");
                }
                if (queue_.back() == Builder::Method::Key){
                    tmp_.push_back(value);
                    queue_.push_back(Builder::Method::Value);
                }
                else if (queue_.back() == Builder::Method::StartArray || (queue_.back() == Builder::Method::Value && CheckIfElementIsInsideOfArray()) || ((queue_.back() == Builder::Method::EndDict || queue_.back() == Builder::Method::EndArray) && CheckIfElementIsInsideOfArray())){
                    tmp_.push_back(value);
                    queue_.push_back(Builder::Method::Value);
                }
                else if (queue_.back() == Builder::Method::Start) {
                    root_ = value;
                    queue_.pop_back();
                }
                else {
                    throw std::logic_error("Invalid_Value_Builder::Method_Invocation");
                }   
                return *this;
            }

            Builder& Builder::Key(std::string str){
                if (queue_.back() == Builder::Method::StartDict || queue_.back() == Builder::Method::Value || ((queue_.back() == Builder::Method::EndDict || queue_.back() == Builder::Method::EndArray) && CheckIfKeyIsInsideOfDict())){
                    tmp_.push_back(str);
                    queue_.push_back(Builder::Method::Key);
                    return *this;
                }
                else {
                    throw std::logic_error("Invalid_Key_Builder::Method_Invocation");
                }
                if (queue_.empty()){
                    throw std::logic_error("The_object_is_complete");
                }    

            }

            StartDictItemContext Builder::StartDict(){
                if (queue_.back() == Builder::Method::StartDict){
                    throw std::logic_error("Invalid_StartDict_Builder::Method_Invocation");
                }
                if ((queue_.back() == Builder::Method::Value && CheckIfElementIsInsideOfArray()) || queue_.back() == Builder::Method::Key || queue_.back() == Builder::Method::Start || queue_.back() == Builder::Method::StartArray || ((queue_.back() == Builder::Method::EndDict || queue_.back() == Builder::Method::EndArray) && CheckIfElementIsInsideOfArray())){
                    Dict dict{};
                    tmp_.push_back(dict);
                    queue_.push_back(Builder::Method::StartDict);
                    return StartDictItemContext(*this);
                }
                else {
                    throw std::logic_error("Invalid_StartDict_Builder::Method_Invocation");
                }
                if (queue_.empty()){
                    throw std::logic_error("The_object_is_complete");
                }    
            }

            Builder& Builder::EndDict(){
                if (queue_.empty()){
                    throw std::logic_error("The_object_is_complete");
                }
                while (queue_.back() != Builder::Method::StartDict){
                    if (queue_.back() == Builder::Method::StartArray){
                        throw std::logic_error("Invalid_EndDict_Builder::Method_Invocation");
                    }
                    else {
                        queue_.pop_back();
                    }
                }
                queue_.pop_back();
                Dict dict{};
                while (!tmp_.back().IsMap()){
                    Node value = tmp_.back();
                    tmp_.pop_back();
                    dict[tmp_.back().AsString()] = value;
                    tmp_.pop_back();
                }
                tmp_.back() = dict;
                if (queue_.back() == Builder::Method::Key || queue_.back() == Builder::Method::StartDict || queue_.back() == Builder::Method::Value || queue_.back() == Builder::Method::EndDict || queue_.back() == Builder::Method::EndArray || queue_.back() == Builder::Method::StartArray){
                    queue_.push_back(Builder::Method::EndDict);
                }
                else if (queue_.back() == Builder::Method::Start){
                    root_ = tmp_.back();
                    queue_.pop_back();
                }
                else {
                    throw std::logic_error("Invalid_EndDict_Builder::Method_Invocation");
                }    
                return *this;
            }

            StartArrayItemContext Builder::StartArray(){
                if (queue_.back() == Builder::Method::StartDict){
                    throw std::logic_error("Invalid_StartArray_Builder::Method_Invocation");
                }
                if ((queue_.back() == Builder::Method::Value && CheckIfElementIsInsideOfArray()) || queue_.back() == Builder::Method::Key || queue_.back() == Builder::Method::Start || queue_.back() == Builder::Method::StartArray || ((queue_.back() == Builder::Method::EndDict || queue_.back() == Builder::Method::EndArray) && CheckIfElementIsInsideOfArray())){
                    Array array{};
                    tmp_.push_back(array);
                    queue_.push_back(Builder::Method::StartArray);
                    return StartArrayItemContext(*this);
                }
                else {
                    throw std::logic_error("Invalid_StartArray_Builder::Method_Invocation");
                }
                if (queue_.empty()){
                    throw std::logic_error("The_object_is_complete");
                }    
            }

            Builder& Builder::EndArray(){
                if (queue_.empty()){
                    throw std::logic_error("The_object_is_complete");
                } 
                while (queue_.back() != Builder::Method::StartArray){
                    if (queue_.back() == Builder::Method::StartDict){
                        throw std::logic_error("Invalid_EndArray_Builder::Method_Invocation");
                    }
                    else {
                       queue_.pop_back();
                    }
                }
                queue_.pop_back();
                Array array{};
                while (!tmp_.back().IsArray()){
                    array.push_back(tmp_.back());
                    tmp_.pop_back();
                }
                Array rev{array.rbegin(), array.rend()};
                tmp_.back() = rev;
                if (queue_.back() == Builder::Method::Value || queue_.back() == Builder::Method::StartArray || queue_.back() == Builder::Method::Key){
                    queue_.push_back(Builder::Method::EndArray);
                }
                else if (queue_.back() == Builder::Method::Start){
                    root_ = tmp_.back();
                    queue_.pop_back();
                }
                else {
                    throw std::logic_error("Invalid_EndArray_Builder::Method_Invocation");
                }               
                return *this;
            }

            json::Node Builder::Build(){
                if (queue_.empty()){
                    queue_.clear();
                    return root_;
                }
                else {
                    throw std::logic_error("Incomlete_object");
                }  
            }       

            bool Builder::CheckIfKeyIsInsideOfDict(){
                int counter = 0;
                for (auto it = queue_.end(); it != queue_.begin(); --it){
                    if (*it == Builder::Method::EndDict){
                        --counter;
                    }
                    if (*it == Builder::Method::StartDict){
                        ++counter;
                    }
                }
                return counter >= 0;
            } 

            bool Builder::CheckIfElementIsInsideOfArray(){
                if (none_of(queue_.begin(), queue_.end(), [](auto mark){ return mark == Builder::Method::StartArray; })){
                    return false;
                }
                int counter = 0;
                for (auto it = queue_.end(); it != queue_.begin(); --it){
                    if (*it == Builder::Method::EndArray){
                        --counter;
                    }
                    if (*it == Builder::Method::StartArray){
                        ++counter;
                    }
                }
                return counter >= 0;
            } 


            BaseClass::BaseClass(Builder& builder) : builder_(builder){}

            StartDictItemContext KeyItemContext::Value(Node value){
                return {builder_.Value(value)};
            }
            

            KeyItemContext BaseClass::Key(std::string str){
                return builder_.Key(str);
            }

            StartDictItemContext BaseClass::StartDict(){ 
                return builder_.StartDict();
            }

            Builder& BaseClass::EndDict(){
                builder_.EndDict();
                return builder_; 
            }

            StartArrayItemContext BaseClass::StartArray(){                
                return builder_.StartArray();
            }

            StartArrayItemContext StartArrayItemContext::Value(Node value){
                return {builder_.Value(value)};
            }

            Builder& BaseClass::EndArray(){
                builder_.EndArray();
                return builder_;
            }

            json::Node BaseClass::Build(){
                return builder_.Build();
            }
