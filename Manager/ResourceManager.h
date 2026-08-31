#pragma once
#include "FileSuper.h"

#include <windows.h>
#include <string>
#include <unordered_map>
#include <memory>
#include <cassert>
#include <type_traits>


// deleter と共有
struct ResourceManagerState
{
    std::unordered_map<std::string, std::weak_ptr<FileSuper>> table;
    bool alive = true;
};

template<class T>

class ResourceManager
{
    static_assert(std::is_base_of_v<FileSuper, T>,
                  "TはFileSuperから動かす必要があります");

public:
    ResourceManager() : state_(std::make_shared<ResourceManagerState>())
    {
    }

    ~ResourceManager()
    {
        state_->alive = false;
        state_->table.clear();
    }

    template<class Loader>
    std::shared_ptr<T> Load(const std::string& path, Loader&& loader)
    {
		// キャッシュ用正規化パスを取得
		std::string normalizedPath = NormalizePath(path);

        // 既存チェック
        if (auto it = state_->table.find(path); it != state_->table.end())
        {
            if (auto sp = it->second.lock())
            {
                return std::static_pointer_cast<T>(sp);
            }
        }

        // 新規ロード(ローダーには元のパスを渡す)
        int handle = loader(path);
        if (handle < 0)
        {
#ifdef _DEBUG
            OutputDebugStringA("リソースを読み込めませんでした\n");
#endif
            return nullptr;
        }

        auto state = state_;
        auto deleter = [state, normalizedPath](T* obj)
            {
                delete obj; // 仮想デストラクタ → 派生の DeleteXXX

                // ResourceManagerの削除後に解放したリソースが存在するか
                if (!state->alive)
                {
#ifdef _DEBUG
                    OutputDebugStringA(
                        "ResourceManagerが削除された後にリソースが解放されました\n");
#endif
                    return;
                }

                state->table.erase(normalizedPath);
            };

        auto sp = std::shared_ptr<T>(
            new T(path, handle), // FileSuperから派生した各ファイルへのポインタ作成
            deleter // カスタムデリータ
        );

        state->table[normalizedPath] = sp; // ResoureceManagerStateのテーブルに新規ファイルのShared_ptrを格納
        return sp;
    }

	// パスの正規化処理
	std::string NormalizePath(const std::string& path)
	{
		char fullPath[MAX_PATH];
		if (!GetFullPathNameA(path.c_str(), MAX_PATH, fullPath, nullptr))
		{
			return path; // 失敗したら元のパスを返す
		}

		std::string normalized = fullPath;

		// バックスラッシュをスラッシュに置換
		for (auto& c : normalized)
		{
			if (c == '\\') c = '/';
		}

		// 大文字小文字を統一（小文字化）
		for (auto& c : normalized) c = tolower(c);

		return normalized;
	}

private:
    std::shared_ptr<ResourceManagerState> state_;
};
