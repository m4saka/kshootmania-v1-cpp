﻿#pragma once
#include "update_info/update_info.hpp"
#include "note/bt_note_graphics.hpp"
#include "note/fx_note_graphics.hpp"
#include "note/laser_note_graphics.hpp"
#include "highway_tilt.hpp"

namespace MusicGame::Graphics
{
	class Highway3DGraphics
	{
	private:
		const Texture m_bgTexture;
		const Texture m_shineEffectTexture;
		const Texture m_beamTexture;

		RenderTexture m_additiveRenderTexture;
		RenderTexture m_invMultiplyRenderTexture;

		BTNoteGraphics m_btNoteGraphics;
		FXNoteGraphics m_fxNoteGraphics;
		LaserNoteGraphics m_laserNoteGraphics;

		MeshData m_meshData;
		DynamicMesh m_mesh;

		HighwayTilt m_highwayTilt;

	public:
		Highway3DGraphics();

		void update(const UpdateInfo& updateInfo);

		void draw(const UpdateInfo& updateInfo, const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget) const;
	};
}
