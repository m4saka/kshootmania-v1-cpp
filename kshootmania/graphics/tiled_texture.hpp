#pragma once

struct TiledTextureSizeInfo
{
	static constexpr int kAutoDetect = 0;
	static constexpr Size kAutoDetectSize = { 0, 0 };

	// Set to kAutoDetect to automatically set based on sourceSize
	int32 row = 1;
	int32 column = 1;

	Point offset = { 0, 0 };
	ScreenUtils::SourceScale sourceScale = ScreenUtils::SourceScale::kNoScaling;

	// Set to kAutoDetectSize to automatically set based on row/column
	Size sourceSize = kAutoDetectSize;
};

class TiledTexture
{
private:
	const Texture m_texture;
	const TiledTextureSizeInfo m_sizeInfo;
	const Size m_scaledSize;

#ifndef NDEBUG
	const String m_textureAssetKey;
#endif

public:
	TiledTexture(Texture&& texture, const TiledTextureSizeInfo& sizeInfo);
	TiledTexture(const Texture& texture, const TiledTextureSizeInfo& sizeInfo);
	TiledTexture(StringView textureAssetKey, const TiledTextureSizeInfo& sizeInfo);
	TiledTexture(StringView textureAssetKey, ScreenUtils::SourceScale scale);

	const Size& scaledSize() const
	{
		return m_scaledSize;
	}

	TextureRegion operator()(int32 row = 0, int32 column = 0) const;

	int32 row() const
	{
		return m_sizeInfo.row;
	}

	int32 column() const
	{
		return m_sizeInfo.column;
	}
};
