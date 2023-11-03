#include "stdafx.h"
#include "PlayingTips.h"

namespace
{
	StringView randomBasicTips()
	{
		const static Array<StringView> tips{
			U"「スクう」動作は連続して使うと多くのペナルティがつく",
			U"フロアを1つクリアするたびに生存時間が少し回復し、最大値が増える",
			U"アイテムで敵を倒すと生存時間が回復する",
			U"歩きに比べて走る動作は視界が不安定になりやすい",
			U"砂時計を取ると生存時間が少し回復する",
			U"生存時間の最大値を大幅に上げるアイテムは出現階層が固定されている",
			U"部屋の中より通路のほうが敵の追跡を逃れやすい",
			U"迷路階層ではレアアイテムが多く入手しやすい",
			U"移動中に使えるアイテムと使えないアイテムがある",
			U"アイテムを拾える数には限りがある",
		};
		return tips[Random(0, static_cast<int>(tips.size()) - 1)];
	}
}

namespace Play
{
	StringView GetPlayingTips(int floorIndex)
	{
		return randomBasicTips();
	}
}
