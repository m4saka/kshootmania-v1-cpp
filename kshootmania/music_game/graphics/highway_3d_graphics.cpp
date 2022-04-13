﻿#include "highway_3d_graphics.hpp"

namespace
{
	constexpr StringView kHighwayBGTextureFilename = U"base.gif";
	constexpr StringView kShineEffectTextureFilename = U"lanelight.gif";
	constexpr StringView kKeyBeamTextureFilename = U"judge.gif";
	constexpr StringView kChipBTNoteTextureFilename = U"bt_chip.gif";
	constexpr StringView kLongBTNoteTextureFilename = U"bt_long.gif";
	constexpr StringView kChipFXNoteTextureFilename = U"fx_chip.gif";
	constexpr StringView kLongFXNoteTextureFilename = U"fx_long.gif";
	constexpr StringView kLaserNoteTextureFilename = U"laser.gif";
	constexpr StringView kLaserNoteMaskTextureFilename = U"laser_mask.gif";
	constexpr StringView kLaserNoteLeftStartTextureFilename = U"laserl_0.gif";
	constexpr StringView kLaserNoteRightStartTextureFilename = U"laserr_0.gif";

	// Angle of the line connecting the camera position and the judgment line from the horizontal
	// (Unconfirmed, but KSMv1 uses this value anyway)
	constexpr double kCameraToJudgmentLineRadians = -0.6125;

	constexpr Size kTextureSize = { 256, 1024 };
	constexpr double kJudgmentLineYFromBottom = 14;

	constexpr double kPlaneWidth = 304.0 * 2 / 13;
	constexpr double kPlaneHeight = 936.0 * 13 / 20;
	constexpr double kPlaneHeightBelowJudgmentLine = kPlaneHeight * kJudgmentLineYFromBottom / kTextureSize.y;
	constexpr double kPlaneHeightAboveJudgmentLine = kPlaneHeight - kPlaneHeightBelowJudgmentLine;

	// Shrink UV to remove edge pixels
	constexpr float kUVShrinkX = 0.0075f;
	constexpr float kUVShrinkY = 0.005f;

	constexpr Vec2 kLanePositionOffset = { 44.0, 0.0 };
	constexpr Vec2 kBTLanePositionDiff = { 42.0, 0.0 };
	constexpr Vec2 kFXLanePositionDiff = { 84.0, 0.0 };
	constexpr Size kBTKeyBeamTextureSize = { 40, 300 };
	constexpr Size kFXKeyBeamTextureSize = { 82, 300 };

	constexpr Size kLaserTextureSize = { 48, 48 };
	constexpr double kLaserLineWidth = static_cast<double>(kLaserTextureSize.x);
	constexpr Size kLaserStartTextureSize = { 44, 200 };

	constexpr int32 kNumShineEffects = 4;
	constexpr Vec2 kShineEffectPositionOffset = { 40.0, 0.0 };
	constexpr Vec2 kShineEffectPositionDiff = { 0.0, 300.0 };

	constexpr double kKeyBeamFullWidthSec = 0.075;
	constexpr double kKeyBeamEndSec = 0.155;
	constexpr Vec2 kKeyBeamPositionOffset = kLanePositionOffset + Vec2{ 0.0, kTextureSize.y - 300.0 };

	TiledTexture ApplyAlphaToNoteTexture(const Texture& texture, const TiledTextureSizeInfo& sizeInfo)
	{

		const PixelShader ps = HLSL{ U"shaders/multi_texture_mask.hlsl", U"PS" } | GLSL{ U"shaders/multi_texture_mask.frag", {{U"PSConstants2D", 0}} };
		if (!ps)
		{
			throw Error(U"Failed to load shader file 'shaders/multi_texture_mask.hlsl' or 'shaders/multi_texture_mask.frag'");
		}

		const Size textureSize = { sizeInfo.sourceSize.x * sizeInfo.column / 2, sizeInfo.sourceSize.y * sizeInfo.row };

		RenderTexture renderTextureRGB(textureSize);
		{
			const ScopedRenderTarget2D renderTarget(renderTextureRGB.clear(Palette::Black));
			for (int32 row = 0; row < sizeInfo.row; ++row)
			{
				for (int32 column = 0; column < sizeInfo.column / 2; ++column)
				{
					texture(sizeInfo.sourceSize.x * (column * 2), sizeInfo.sourceSize.y * row, sizeInfo.sourceSize)
						.draw(sizeInfo.sourceSize.x * column, sizeInfo.sourceSize.y * row);
				}
			}
		}

		RenderTexture renderTextureA(textureSize);
		{
			const ScopedRenderTarget2D renderTarget(renderTextureA.clear(Palette::Black));
			for (int32 row = 0; row < sizeInfo.row; ++row)
			{
				for (int32 column = 0; column < sizeInfo.column / 2; ++column)
				{
					texture(sizeInfo.sourceSize.x * (column * 2 + 1), sizeInfo.sourceSize.y * row, sizeInfo.sourceSize)
						.draw(sizeInfo.sourceSize.x * column, sizeInfo.sourceSize.y * row);
				}
			}
		}

		RenderTexture renderTextureMerged(textureSize);
		{
			Graphics2D::SetPSTexture(1, renderTextureA);
			const ScopedCustomShader2D customShader(ps);
			const ScopedRenderTarget2D renderTarget(renderTextureMerged.clear(ColorF{ 0.0, 0.0 }));
			const ScopedRenderStates2D renderState(BlendState(
				true,
				Blend::SrcAlpha,
				Blend::InvSrcAlpha,
				BlendOp::Add,
				Blend::One));
			renderTextureRGB.draw();
		}

		TiledTextureSizeInfo sizeInfoHalfColumn = sizeInfo;
		sizeInfoHalfColumn.column /= 2;
		return TiledTexture(renderTextureMerged, sizeInfoHalfColumn);
	}

	double ChipNoteHeight(double yRate)
	{
		constexpr int32 kTableSize = 8;
		constexpr std::array<double, kTableSize> kHeightTable = {
			14, 19, 23, 26, 28, 30, 32, 35
		};

		return kHeightTable[Clamp(static_cast<int32>(yRate * kTableSize), 0, kTableSize - 1)];
	}

	Quad LaserLineQuad(const Vec2& positionStart, const Vec2& positionEnd)
	{
		return Quad{
			positionStart + Vec2{ kLaserLineWidth / 2, 0.0 },
			positionStart + Vec2{ -kLaserLineWidth / 2, 0.0 },
			positionEnd + Vec2{ -kLaserLineWidth / 2, 0.0 },
			positionEnd + Vec2{ kLaserLineWidth / 2, 0.0 }
		};
	}

	Quad LaserSlamLineQuad(const Vec2& positionStart, const Vec2& positionEnd)
	{
		if (Abs(positionEnd.x - positionStart.x) <= kLaserLineWidth)
		{
			// Too short to draw line
			return Quad{ Vec2::Zero(), Vec2::Zero(), Vec2::Zero(), Vec2::Zero() };
		}

		const int diffXSign = Sign(positionEnd.x - positionStart.x);
		return Quad{
			positionStart + Vec2{ diffXSign * kLaserLineWidth / 2, -kLaserLineWidth },
			positionEnd + Vec2{ -diffXSign * kLaserLineWidth / 2, -kLaserLineWidth },
			positionEnd + Vec2{ -diffXSign * kLaserLineWidth / 2, 0.0 },
			positionStart + Vec2{ diffXSign * kLaserLineWidth / 2, 0.0 }
		};
	}
}

MusicGame::Graphics::Highway3DGraphics::Highway3DGraphics()
	: m_bgTexture(TextureAsset(kHighwayBGTextureFilename))
	, m_shineEffectTexture(TextureAsset(kShineEffectTextureFilename))
	, m_beamTexture(TextureAsset(kKeyBeamTextureFilename))
	, m_chipBTNoteTexture(ApplyAlphaToNoteTexture(TextureAsset(kChipBTNoteTextureFilename),
		{
			.column = 9 * 2,
			.sourceSize = { 40, 14 },
		}))
	, m_longBTNoteTexture(TextureAsset(kLongBTNoteTextureFilename))
	, m_chipFXNoteTexture(ApplyAlphaToNoteTexture(TextureAsset(kChipFXNoteTextureFilename),
		{
			.column = 2,
			.sourceSize = { 82, 14 },
		}))
	, m_longFXNoteTexture(TextureAsset(kLongFXNoteTextureFilename))
	, m_laserNoteTexture(TextureAsset(kLaserNoteTextureFilename))
	, m_laserNoteMaskTexture(TextureAsset(kLaserNoteMaskTextureFilename))
	, m_laserNoteLeftStartTexture(kLaserNoteLeftStartTextureFilename,
		{
			.column = 2,
			.sourceSize = kLaserStartTextureSize,
		})
	, m_laserNoteRightStartTexture(kLaserNoteRightStartTextureFilename,
		{
			.column = 2,
			.sourceSize = kLaserStartTextureSize,
		})
	, m_additiveRenderTexture(kTextureSize)
	, m_invMultiplyRenderTexture(kTextureSize)
	, m_meshData(MeshData::Grid({ 0.0, 0.0, 0.0 }, { kPlaneWidth, kPlaneHeight }, 2, 2, { 1.0f - kUVShrinkX, 1.0f - kUVShrinkY }, { kUVShrinkX / 2, kUVShrinkY / 2 }))
	, m_mesh(m_meshData) // <- this initialization is important because DynamicMesh::fill() does not dynamically resize the vertex array
{
}

void MusicGame::Graphics::Highway3DGraphics::update(const UpdateInfo& updateInfo)
{
	// TODO: Calculate vertex position from cameraState

	m_updateInfo = updateInfo;

	assert(m_updateInfo.pChartData != nullptr);
}

void MusicGame::Graphics::Highway3DGraphics::draw(const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget) const
{
	const ScopedRenderStates2D samplerState(SamplerState::ClampNearest);
	Shader::Copy(m_bgTexture(0, 0, kTextureSize), m_additiveRenderTexture);
	Shader::Copy(m_bgTexture(kTextureSize.x, 0, kTextureSize), m_invMultiplyRenderTexture);

	// Draw shine effect
	{
		const ScopedRenderTarget2D renderTarget(m_additiveRenderTexture);
		const ScopedRenderStates2D renderState(BlendState::Additive);

		for (int32 i = 0; i < kNumShineEffects; ++i)
		{
			m_shineEffectTexture.draw(kShineEffectPositionOffset + kShineEffectPositionDiff * i + kShineEffectPositionDiff * MathUtils::WrappedFmod(m_updateInfo.currentTimeSec, 0.2) / 0.2);
		}
	}

	// Draw BT/FX notes
	if (m_updateInfo.pChartData != nullptr)
	{
		const ScopedRenderTarget2D renderTarget(m_additiveRenderTexture);

		const ksh::ChartData& chartData = *m_updateInfo.pChartData;

		// FX notes
		for (std::size_t laneIdx = 0; laneIdx < ksh::kNumFXLanes; ++laneIdx)
		{
			const auto& lane = chartData.note.fxLanes[laneIdx];
			for (const auto& [y, note] : lane)
			{
				if (y + note.length < m_updateInfo.currentPulse - chartData.beat.resolution)
				{
					continue;
				}

				const double positionStartY = static_cast<double>(kTextureSize.y) - static_cast<double>(y - m_updateInfo.currentPulse) * 480 / chartData.beat.resolution;
				if (positionStartY < 0)
				{
					break;
				}

				if (note.length == 0)
				{
					// Chip FX notes
					const double yRate = (static_cast<double>(kTextureSize.y) - positionStartY) / static_cast<double>(kTextureSize.y);
					m_chipFXNoteTexture()
						.resized(82, ChipNoteHeight(yRate))
						.draw(kLanePositionOffset + kFXLanePositionDiff * laneIdx + Vec2::Down(positionStartY));
				}
				else
				{
					// Long FX notes
					const double positionEndY = static_cast<double>(kTextureSize.y) - static_cast<double>(y + note.length - m_updateInfo.currentPulse) * 480 / chartData.beat.resolution;
					m_longFXNoteTexture(0, 0, 82, 1)
						.resized(82, note.length * 480 / chartData.beat.resolution)
						.draw(kLanePositionOffset + kFXLanePositionDiff * laneIdx + Vec2::Down(positionEndY));
				}
			}
		}

		// BT notes
		for (std::size_t laneIdx = 0; laneIdx < ksh::kNumBTLanes; ++laneIdx)
		{
			const auto& lane = chartData.note.btLanes[laneIdx];
			for (const auto& [y, note] : lane)
			{
				if (y + note.length < m_updateInfo.currentPulse - chartData.beat.resolution)
				{
					continue;
				}

				const double positionStartY = static_cast<double>(kTextureSize.y) - static_cast<double>(y - m_updateInfo.currentPulse) * 480 / chartData.beat.resolution;
				if (positionStartY < 0)
				{
					break;
				}

				if (note.length == 0)
				{
					// Chip BT notes
					const double yRate = (static_cast<double>(kTextureSize.y) - positionStartY) / static_cast<double>(kTextureSize.y);
					m_chipBTNoteTexture()
						.resized(40, ChipNoteHeight(yRate))
						.draw(kLanePositionOffset + kBTLanePositionDiff * laneIdx + Vec2::Down(positionStartY));
				}
				else
				{
					// Long BT notes
					const ScopedRenderStates2D renderState(BlendState::Additive);
					const double positionEndY = static_cast<double>(kTextureSize.y) - static_cast<double>(y + note.length - m_updateInfo.currentPulse) * 480 / chartData.beat.resolution;
					for (int32 i = 0; i < 2; ++i)
					{
						const ScopedRenderTarget2D renderTarget((i == 0) ? m_additiveRenderTexture : m_invMultiplyRenderTexture);
						m_longBTNoteTexture(40 * i, 0, 40, 1)
							.resized(40, note.length * 480 / chartData.beat.resolution)
							.draw(kLanePositionOffset + kBTLanePositionDiff * laneIdx + Vec2::Down(positionEndY));
					}
				}
			}
		}
	}

	// Draw key beam
	{
		const ScopedRenderTarget2D renderTarget(m_additiveRenderTexture);
		const ScopedRenderStates2D renderState(BlendState::Additive);

		for (std::size_t i = 0; i < ksh::kNumBTLanes + ksh::kNumFXLanes; ++i)
		{
			const bool isBT = (i < ksh::kNumBTLanes);
			const std::size_t laneIdx = isBT ? i : (i - ksh::kNumBTLanes);
			const LaneState& laneState = isBT ? m_updateInfo.btLaneState[laneIdx] : m_updateInfo.fxLaneState[laneIdx];

			const double sec = m_updateInfo.currentTimeSec - laneState.keyBeamTimeSec;
			if (sec < 0.0 || kKeyBeamEndSec < sec)
			{
				continue;
			}

			double widthRate = 1.0;
			double alpha = 1.0;
			if (sec < kKeyBeamFullWidthSec)
			{
				widthRate = sec / kKeyBeamFullWidthSec;
			}
			else
			{
				alpha = 1.0 - (sec - kKeyBeamFullWidthSec) / (kKeyBeamEndSec - kKeyBeamFullWidthSec);
			}

			const TextureRegion beamTextureRegion = m_beamTexture(
				kBTKeyBeamTextureSize.x * (laneState.keyBeamType + 0.5 - widthRate / 2),
				0,
				kBTKeyBeamTextureSize.x * widthRate,
				kBTKeyBeamTextureSize.y);

			if (isBT)
			{
				beamTextureRegion
					.draw(kKeyBeamPositionOffset + kBTLanePositionDiff * (laneIdx + 0.5 - widthRate / 2), ColorF{ 1.0, alpha });
			}
			else
			{
				beamTextureRegion
					.resized(kFXKeyBeamTextureSize.x * widthRate, kFXKeyBeamTextureSize.y)
					.draw(kKeyBeamPositionOffset + kFXLanePositionDiff * laneIdx + kFXKeyBeamTextureSize * (0.5 - widthRate / 2), ColorF{ 1.0, alpha });
			}
		}
	}

	// Draw laser notes
	if (m_updateInfo.pChartData != nullptr)
	{
		const ScopedRenderStates2D renderState(BlendState::Additive);

		const ksh::ChartData& chartData = *m_updateInfo.pChartData;

		for (std::size_t laneIdx = 0; laneIdx < ksh::kNumLaserLanes; ++laneIdx)
		{
			const auto& lane = chartData.note.laserLanes[laneIdx];
			for (const auto& [y, laserSection] : lane)
			{
				const double positionSectionStartY = static_cast<double>(kTextureSize.y) - static_cast<double>(y - m_updateInfo.currentPulse) * 480 / chartData.beat.resolution;
				if (positionSectionStartY + kLaserStartTextureSize.y < 0)
				{
					// Laser section is above the drawing area
					break;
				}

				for (auto itr = laserSection.points.begin(); itr != laserSection.points.end(); ++itr)
				{
					const auto [ry, point] = *itr;

					// Draw laser start texture
					if (itr == laserSection.points.begin())
					{
						for (int32 i = 0; i < 2; ++i)
						{
							const ScopedRenderTarget2D renderTarget((i == 0) ? m_additiveRenderTexture : m_invMultiplyRenderTexture);
							const Vec2 positionStart = {
								point.v * (kTextureSize.x - kLaserLineWidth) + kLaserLineWidth / 2,
								static_cast<double>(kTextureSize.y) - static_cast<double>(y + ry - m_updateInfo.currentPulse) * 480 / chartData.beat.resolution
							};
							const TiledTexture& startTexture = (laneIdx == 0) ? m_laserNoteLeftStartTexture : m_laserNoteRightStartTexture;
							startTexture(0, i).draw(Arg::topCenter = positionStart);
						}
					}

					const double positionStartY = static_cast<double>(kTextureSize.y) - static_cast<double>(y + ry - m_updateInfo.currentPulse) * 480 / chartData.beat.resolution;
					if (positionStartY < 0)
					{
						// Laser point is above the drawing area
						break;
					}

					// Draw laser slam
					if (point.v != point.vf)
					{
						const Vec2 positionStart = {
							point.v * (kTextureSize.x - kLaserLineWidth) + kLaserLineWidth / 2,
							static_cast<double>(kTextureSize.y) - static_cast<double>(y + ry - m_updateInfo.currentPulse) * 480 / chartData.beat.resolution
						};

						const Vec2 positionEnd = {
							point.vf * (kTextureSize.x - kLaserLineWidth) + kLaserLineWidth / 2,
							static_cast<double>(kTextureSize.y) - static_cast<double>(y + ry - m_updateInfo.currentPulse) * 480 / chartData.beat.resolution
						};

						const bool isLeftToRight = (point.v < point.vf);
						const Quad quad = LaserSlamLineQuad(positionStart, positionEnd);

						for (int32 i = 0; i < 2; ++i)
						{
							const ScopedRenderTarget2D renderTarget((i == 0) ? m_additiveRenderTexture : m_invMultiplyRenderTexture);
							const Texture& texture = (i == 0) ? m_laserNoteTexture : m_laserNoteMaskTexture;
							texture(kLaserTextureSize.x * laneIdx, 0, kLaserTextureSize).mirrored(isLeftToRight).drawAt(positionStart + Vec2{ 0.0, -kLaserLineWidth / 2 });
							texture(kLaserTextureSize.x * laneIdx, 0, kLaserTextureSize).mirrored(!isLeftToRight).flipped().drawAt(positionEnd + Vec2{ 0.0, -kLaserLineWidth / 2 });
							quad(texture(kLaserTextureSize.x * laneIdx, 0, 1, kLaserTextureSize.y)).draw();
						}
					}

					// Last laser point does not create laser line
					const auto nextItr = std::next(itr);
					if (nextItr == laserSection.points.end())
					{
						// If the final note is a laser slam, draw its extension
						if (point.v != point.vf)
						{
							const Vec2 positionStart = {
								point.vf * (kTextureSize.x - kLaserLineWidth) + kLaserLineWidth / 2,
								static_cast<double>(kTextureSize.y) - static_cast<double>(y + ry - m_updateInfo.currentPulse) * 480 / chartData.beat.resolution
							};
							const Quad quad = LaserLineQuad(positionStart + Vec2{ 0.0, -kLaserTextureSize.y }, positionStart + Vec2{ 0.0, -kLaserTextureSize.y - 80.0 });

							for (int32 i = 0; i < 2; ++i)
							{
								const ScopedRenderTarget2D renderTarget((i == 0) ? m_additiveRenderTexture : m_invMultiplyRenderTexture);
								const Texture& texture = (i == 0) ? m_laserNoteTexture : m_laserNoteMaskTexture;
								quad(texture(kLaserTextureSize.x * laneIdx, kLaserTextureSize.y - 1, kLaserTextureSize.x, 1)).draw();
							}
						}

						break;
					}

					// Draw laser line by two laser points
					{
						const auto [nextRy, nextPoint] = *nextItr;
						if (y + nextRy < m_updateInfo.currentPulse - chartData.beat.resolution)
						{
							continue;
						}

						const Vec2 positionStart = {
							point.vf * (kTextureSize.x - kLaserLineWidth) + kLaserLineWidth / 2,
							static_cast<double>(kTextureSize.y) - static_cast<double>(y + ry - m_updateInfo.currentPulse) * 480 / chartData.beat.resolution
						};

						const Vec2 positionEnd = {
							nextPoint.v * (kTextureSize.x - kLaserLineWidth) + kLaserLineWidth / 2,
							static_cast<double>(kTextureSize.y) - static_cast<double>(y + nextRy - m_updateInfo.currentPulse) * 480 / chartData.beat.resolution
						};

						const Quad quad = LaserLineQuad(positionStart + ((point.v != point.vf) ? Vec2{ 0.0, -kLaserTextureSize.y } : Vec2::Zero()), positionEnd);

						for (int32 i = 0; i < 2; ++i)
						{
							const ScopedRenderTarget2D renderTarget((i == 0) ? m_additiveRenderTexture : m_invMultiplyRenderTexture);
							const Texture& texture = (i == 0) ? m_laserNoteTexture : m_laserNoteMaskTexture;
							quad(texture(kLaserTextureSize.x * laneIdx, kLaserTextureSize.y - 1, kLaserTextureSize.x, 1)).draw();
						}
					}
				}
			}
		}
	}

	{
		const ScopedRenderTarget3D renderTarget(invMultiplyTarget);
		m_mesh.draw(m_invMultiplyRenderTexture);
	}

	{
		const ScopedRenderTarget3D renderTarget(additiveTarget);
		m_mesh.draw(m_additiveRenderTexture);
	}
}
