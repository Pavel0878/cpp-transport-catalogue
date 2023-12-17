#pragma once

#include "json.h"

#include <string>
#include <vector>

namespace json {

	class BaseContext;
	class KeyItemContext;
	class ValueItemContext;
	class DictItemContext;
	class ArrayItemContext;

	class Builder {
	public:
		Builder();
		KeyItemContext Key(std::string key);
		BaseContext Value(Node value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		BaseContext EndDict();
		BaseContext EndArray();
		Node Build();

	private:
		Node root_;
		std::vector<Node*> nodes_stack_;
	};

	class BaseContext {
	public:
		BaseContext(Builder& builder);

		Node Build();
		KeyItemContext Key(std::string key);
		BaseContext Value(Node value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		BaseContext EndDict();
		BaseContext EndArray();
	private:
		Builder& builder_;
	};

	class KeyItemContext : public BaseContext {
	public:
		KeyItemContext(BaseContext base);

		Node Build() = delete;
		KeyItemContext Key(std::string key) = delete;
		DictItemContext Value(Node value);
		BaseContext EndDict() = delete;
		BaseContext EndArray() = delete;
	};

	class DictItemContext : public BaseContext {
	public:
		DictItemContext(BaseContext base);

		Node Build() = delete;
		BaseContext Value(Node value) = delete;
		DictItemContext StartDict() = delete;
		ArrayItemContext StartArray() = delete;
		BaseContext EndArray() = delete;
	};

	class ArrayItemContext : public BaseContext {
	public:
		ArrayItemContext(BaseContext base);

		Node Build() = delete;
		KeyItemContext Key(std::string key) = delete;
		ArrayItemContext Value(Node value);
		BaseContext EndDict() = delete;
	};

} //namespace json