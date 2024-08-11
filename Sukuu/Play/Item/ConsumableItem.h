#pragma once

namespace Play
{
	enum class ConsumableItem
	{
		None = 0,
		Wing = 1,
		Helmet = 2,
		Pin = 3,
		Mine = 4,
		LightBulb = 5,
		Magnet = 6,
		Bookmark = 7,
		Explorer = 8,
		Grave = 9,
		Sun = 10,
		Tube = 11,
		Solt = 12,
		Rocket = 13,

		Max,
	};

	struct ItemProps
	{
		ConsumableItem item;
		AssetNameView emoji;
		StringView desc;
	};

	const ItemProps& GetItemProps(ConsumableItem item);

	class ItemAttackerAffair
	{
	public:
		explicit ItemAttackerAffair(ConsumableItem kind): m_kind(kind) { return; }

		ConsumableItem Kind() const { return m_kind; }
		int AttackedCount() const { return m_attackedCount; }
		void IncAttacked() { m_attackedCount++; };

	private:
		ConsumableItem m_kind{};
		int m_attackedCount{};
	};
}
