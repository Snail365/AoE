#pragma once
#include "FileSuper.h"
#include <DxLib.h>
#include <cassert>

class ImageFile final : public FileSuper
{
public:
    ImageFile(const std::string& path, int handle)
        : FileSuper(path, handle)
    {
        int w = 0;
        int h = 0;
        int ok = GetGraphSize(handle_, &w, &h);
        assert(ok == 0);

        width = w;
        height = h;
    }

    ~ImageFile() override
    {
        DeleteGraph(handle_);
    }

    int GetWidth()  const { return width; }
    int GetHeight() const { return height; }

private:
    int width = 0;
    int height = 0;
};
