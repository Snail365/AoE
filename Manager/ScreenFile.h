#pragma once

#include "FileSuper.h"
#include <DxLib.h>

class ScreenFile : public FileSuper
{
public:
	ScreenFile(const std::string& name, int handle) : FileSuper(name, handle) {}

	~ScreenFile() override
	{
		DeleteGraph(handle_);
	}
};