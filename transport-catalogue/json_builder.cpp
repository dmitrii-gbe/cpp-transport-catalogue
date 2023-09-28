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
                    tmp_dict_.back()[tmp_key_.back()] = value;
                    tmp_key_.pop_back();
                    queue_.push_back(Builder::Method::Value);
                }
                else if (queue_.back() == Builder::Method::StartArray || (queue_.back() == Builder::Method::Value && CheckIfElementIsInsideOfArray()) || ((queue_.back() == Builder::Method::EndDict || queue_.back() == Builder::Method::EndArray) && CheckIfElementIsInsideOfArray())){
                    tmp_array_.back().push_back(value);
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

            KeyItemContext Builder::Key(std::string str){
                if (queue_.back() == Builder::Method::StartDict || queue_.back() == Builder::Method::Value || ((queue_.back() == Builder::Method::EndDict || queue_.back() == Builder::Method::EndArray) && CheckIfKeyIsInsideOfDict())){
                    tmp_key_.push_back(str);
                    queue_.push_back(Builder::Method::Key);
                    return KeyItemContext(*this);
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
                    tmp_dict_.push_back(dict);
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
                if (queue_.back() == Builder::Method::Key || queue_.back() == Builder::Method::StartDict){
                    tmp_dict_[tmp_dict_.size() - 2][tmp_key_.back()] = tmp_dict_.back();
                    tmp_key_.pop_back();
                    queue_.push_back(Builder::Method::EndDict);
                }
                else if (queue_.back() == Builder::Method::Value || queue_.back() == Builder::Method::EndDict){
                    tmp_array_.back().push_back(tmp_dict_.back());
                    queue_.push_back(Builder::Method::EndDict);
                }
                else if (queue_.back() == Builder::Method::EndArray){
                    tmp_dict_.back()[tmp_key_.back()] = tmp_array_.back();
                    tmp_key_.pop_back();
                    queue_.push_back(Builder::Method::EndDict); 
                }
                else if (queue_.back() == Builder::Method::Start){
                    root_ = tmp_dict_.back();
                    queue_.pop_back();
                }
                else if (queue_.back() == Builder::Method::StartArray){
                    tmp_array_.back().push_back(tmp_dict_.back());
                    queue_.push_back(Builder::Method::EndDict); //
                }
                else {
                    throw std::logic_error("Invalid_EndDict_Builder::Method_Invocation");
                }    
                tmp_dict_.pop_back();
                return *this;
            }

            StartArrayItemContext Builder::StartArray(){
                if (queue_.back() == Builder::Method::StartDict){
                    throw std::logic_error("Invalid_StartArray_Builder::Method_Invocation");
                }
                if ((queue_.back() == Builder::Method::Value && CheckIfElementIsInsideOfArray()) || queue_.back() == Builder::Method::Key || queue_.back() == Builder::Method::Start || queue_.back() == Builder::Method::StartArray || ((queue_.back() == Builder::Method::EndDict || queue_.back() == Builder::Method::EndArray) && CheckIfElementIsInsideOfArray())){
                    Array array{};
                    tmp_array_.push_back(array);
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
                if (queue_.back() == Builder::Method::Value || queue_.back() == Builder::Method::StartArray){
                    tmp_array_[tmp_array_.size() - 2].push_back(tmp_array_.back());
                    tmp_array_.pop_back();
                    queue_.push_back(Builder::Method::EndArray);
                }
                else if (queue_.back() == Builder::Method::Key){
                    tmp_dict_.back()[tmp_key_.back()] = tmp_array_.back();
                    tmp_key_.pop_back();
                    tmp_array_.pop_back();
                    queue_.push_back(Builder::Method::EndArray);
                }
                else if (queue_.back() == Builder::Method::Start){
                    root_ = tmp_array_.back();
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
                if (none_of(queue_.begin(), queue_.end(), [](auto mark){ return mark == Builder::Method::StartArray/* || mark == Builder::Method::EndArray*/; })){
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

            ValueItemContext KeyItemContext::Value(Node value){
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

            ValueItemContext1 StartArrayItemContext::Value(Node value){
                return {builder_.Value(value)};
            }

            ValueItemContext1 ValueItemContext1::Value(Node value){
                return {builder_.Value(value)};
            }

            Builder& BaseClass::EndArray(){
                builder_.EndArray();
                return builder_;
            }

            json::Node BaseClass::Build(){
                return builder_.Build();
            }
