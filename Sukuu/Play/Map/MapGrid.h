#pragma once

namespace Play
{
	enum class TerrainKind
	{
		None,
		Wall,
		Floor,
		Pathway,
	};

	struct MapCell
	{
		TerrainKind kind = TerrainKind::Wall;
	};

	class MapRooms : public Array<Rect>
	{
	public:
		Point RandomRoomPoint(bool isOdd) const;
		Point RandomRoomPoint(int index, bool isOdd) const;
	};

	enum class MapCategory
	{
		Unknown,
		Dungeon,
		Maze,
	};

	class MapGrid
	{
	public:
		MapGrid() : MapGrid(Size{}) { return; };
		MapGrid(const Size& size) { m_data.resize(size); }
		MapGrid(const Size& size, MapCategory category) : m_category((category)) { m_data.resize(size); }

		MapCategory Category() const { return m_category; }
		bool IsValid() const { return m_data.size() != Size{0, 0}; }

		const Grid<MapCell>& Data() const { return m_data; }

		MapCell& At(const Point& point) { return m_data[point]; };
		const MapCell& At(const Point& point) const { return m_data[point]; };

		MapRooms& Rooms() { return m_rooms; };
		const MapRooms& Rooms() const { return m_rooms; };

	private:
		MapCategory m_category = MapCategory::Unknown;
		Grid<MapCell> m_data{};
		MapRooms m_rooms{};
	};
}
