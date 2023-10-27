#pragma once

namespace Play
{
	enum class ConsumableItem
	{
		None,
		Wing,
		Helmet,
		Pin,

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
