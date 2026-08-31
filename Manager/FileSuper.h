#pragma once
#include<string>

class FileSuper
{
public:
	// 暗黙的型変換を禁止
	explicit FileSuper(const std::string& path, int handle)
		: path_(path), handle_(handle)
	{
	}

	// 派生先のデストラクタで自動的に削除
	virtual ~FileSuper() = default;

	// パスの取得
	const std::string& GetPath() const
	{
		return path_;
	}

	int GetHandle() const
	{

		return handle_;
   	}

private:
	std::string path_;
protected:
	int handle_;
};
