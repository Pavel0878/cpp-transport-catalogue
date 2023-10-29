#include "json_builder.h"

#include <variant>

using namespace std::string_literals;

namespace json {

	Builder::Builder() {
		nodes_stack_.emplace_back(&root_);
	}

	Node Builder::Build() {
		if (!nodes_stack_.empty()) {
			throw std::logic_error("No Build"s);
		}
		return root_;
	}

	KeyItemContext Builder::Key(std::string key) {
		if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
			throw std::logic_error("Invalid Key"s);
		}
		nodes_stack_.emplace_back(&const_cast<Dict&>(nodes_stack_.back()->AsDict())[key]);
		return BaseContext(*this);
	}

	BaseContext Builder::Value(Node value) {
		if (nodes_stack_.empty() || (!nodes_stack_.back()->IsNull() && !nodes_stack_.back()->IsArray())) {
			throw std::logic_error("error value"s);
		}
		if (nodes_stack_.back()->IsArray()) {
			const_cast<Array&>(nodes_stack_.back()->AsArray()).emplace_back(value);
		}
		else {
			*nodes_stack_.back() = value;
			nodes_stack_.pop_back();
		}
		return BaseContext(*this);
	}

	DictItemContext Builder::StartDict() {
		if (nodes_stack_.empty() || (!nodes_stack_.back()->IsNull() && !nodes_stack_.back()->IsArray())) {
			throw std::logic_error("error StartDict"s);
		}
		if (nodes_stack_.back()->IsArray()) {
			const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(Dict());
			nodes_stack_.emplace_back(&const_cast<Array&>(nodes_stack_.back()->AsArray()).back());
		}
		else {
			*nodes_stack_.back() = Dict();
		}
		return BaseContext(*this);
	}

	ArrayItemContext Builder::StartArray() {
		if (nodes_stack_.empty() || (!nodes_stack_.back()->IsNull() && !nodes_stack_.back()->IsArray())) {
			throw std::logic_error("error StartArray"s);
		}
		if (nodes_stack_.back()->IsArray()) {
			const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(Array());
			nodes_stack_.emplace_back(&const_cast<Array&>(nodes_stack_.back()->AsArray()).back());
		}
		else {
			*nodes_stack_.back() = Array();
		}
		return BaseContext(*this);
	}

	BaseContext Builder::EndDict() {
		if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
			throw std::logic_error("error EndDict"s);
		}
		nodes_stack_.pop_back();
		return BaseContext(*this);
	}

	BaseContext Builder::EndArray() {
		if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
			throw std::logic_error("error EndArray"s);
		}
		nodes_stack_.pop_back();
		return BaseContext(*this);
	}
	
	BaseContext::BaseContext(Builder& builder)
		: builder_(builder){}

	Node BaseContext::Build() {
		return builder_.Build();
	}

	KeyItemContext BaseContext::Key(std::string key) {
		return builder_.Key(std::move(key));
	}
	BaseContext BaseContext::Value(Node value) {
		return builder_.Value(std::move(value));
	}
	DictItemContext BaseContext::StartDict() {
		return builder_.StartDict();
	}
	ArrayItemContext BaseContext::StartArray() {
		return builder_.StartArray();
	}
	BaseContext BaseContext::EndDict() {
		return builder_.EndDict();
	}
	BaseContext BaseContext::EndArray() {
		return builder_.EndArray();
	}

	KeyItemContext::KeyItemContext(BaseContext base)
		: BaseContext(base){}

	DictItemContext KeyItemContext::Value(Node value) {
		return BaseContext::Value(std::move(value));
	}

//	ValueItemContext::ValueItemContext(BaseContext base)
//		: BaseContext(base){}

//	ValueItemContext ValueItemContext::Value(Node value) {
//		return BaseContext::Value(std::move(value));
//	}

	DictItemContext::DictItemContext(BaseContext base)
		: BaseContext(base){}

	ArrayItemContext::ArrayItemContext(BaseContext base)
		: BaseContext(base){}

	ArrayItemContext ArrayItemContext::Value(Node value) {
		return BaseContext::Value(std::move(value));
	}
	
}