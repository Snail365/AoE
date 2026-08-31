#pragma once

#include "FileSuper.h"
#include <DxLib.h>
#include <windows.h>

class FontFile : public FileSuper
{
public:
	FontFile(const std::string& path, int handle) : FileSuper(path, handle){}
	~FontFile()override
	{
		DeleteFontToHandle(handle_);
	}
};