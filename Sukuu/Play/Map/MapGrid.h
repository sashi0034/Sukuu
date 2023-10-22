#pragma once

namespace Play
{
	enum class TerrainKind
	{
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
		Point RandomRoomPoint() const;
		Point RandomRoomPoint(int index) const;
	};

	class MapGrid
	{
	public:
		MapGrid() : MapGrid(Size{}) { return; };
		MapGrid(const Size& size) { m_data.resize(size); }

		bool HasData() const { return m_data.size() != Size{0, 0}; }

		const Grid<MapCell>& Data() const { return m_data; }

		MapCell& At(const Point& point) { return m_data[point]; };
		const MapCell& At(const Point& point) const { return m_data[point]; };

		MapRooms& Rooms() { return m_rooms; };
		const MapRooms& Rooms() const { return m_rooms; };

	private:
		Grid<MapCell> m_data{};
		MapRooms m_rooms{};
	};
}
