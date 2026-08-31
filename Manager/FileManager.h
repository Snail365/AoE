#pragma once
#include "ResourceManager.h"
#include "ImageFile.h"
#include "SoundFile.h"
#include "FontFile.h"
#include "ScreenFile.h"
#include <unordered_set>

class FileManager
{
public:
    std::shared_ptr<ImageFile> LoadImageFM(const std::string& path)
    {
        return image_.Load(path,
                           [](const std::string& p)
                           {
                               return LoadGraph(p.c_str());
                           });
    }

    std::shared_ptr<SoundFile> LoadSoundFM(const std::string& path)
    {
        return sound_.Load(path,
                           [](const std::string& p)
                           {
                               return LoadSoundMem(p.c_str());
                           });
    }
	std::shared_ptr<FontFile> CreateFontFM(
		const std::string& fontPath,
		const std::string& fontName,
		int size,
		int thick = 3)
	{
		std::string key =
			fontPath + "_" +
			std::to_string(size) + "_" +
			std::to_string(thick);

		return font_.Load(
			key,
			[this, fontPath, fontName, size, thick]
			(const std::string&)
			{
				RegisterFont(fontPath);

				return CreateFontToHandle(
					fontName.c_str(),
					size,
					thick);
			});
	}
	std::shared_ptr<ScreenFile> CreateScreenFM(
		const std::string& name,
		int width,
		int height,
		int useAlphaChannel)
	{
		std::string key =
			name + "_" +
			std::to_string(width) + "x" +
			std::to_string(height) + "_useAlpha" + 
			(useAlphaChannel ? "true" : "false");

		return screen_.Load(
			key,
			[=](const std::string&)
			{
				return MakeScreen(
					width,
					height,
					useAlphaChannel);
			});
	}



    void RegisterFont(const std::string& fontPath)
    {
					if (registeredFonts_.find(fontPath) != registeredFonts_.end())
					{
						return;
					}

					int result = AddFontResourceExA(
						fontPath.c_str(),
						FR_PRIVATE,
						nullptr);

					assert(result != 0);

					registeredFonts_.insert(fontPath);
    }

	~FileManager()
	{
		for (const auto& font : registeredFonts_)
		{
			RemoveFontResourceExA(
				font.c_str(),
				FR_PRIVATE,
				nullptr);
		}
	}

private:
    ResourceManager<ImageFile> image_;
    ResourceManager<SoundFile> sound_;
    ResourceManager<FontFile> font_;
    ResourceManager<ScreenFile> screen_;

	std::unordered_set<std::string> registeredFonts_;
};
