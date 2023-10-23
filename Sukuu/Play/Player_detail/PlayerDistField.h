#pragma once
#include "PlayerDefinition.h"
#include "Play/Chara/CharaUtil.h"
#include "Play/Map/MapGrid.h"

namespace Play
{
	class PlayerDistFieldInternal
	{
	public:
		const PlayerDistField& Field() const { return m_field; }
		void Clear();
		void Refresh(
			const MapGrid& map,
			const CharaVec2& playerPos);

		void Resize(const Size& size)
		{
			m_field.resize(size);
		}

	private:
		class Impl;
		PlayerDistField m_field{};
		Array<Point> m_wroteHistory{};
	};

	void RefreshPlayerDistField(
		PlayerDistField& distField,
		const MapGrid& map,
		const CharaVec2& oldPos,
		const CharaVec2& newPos);
}
