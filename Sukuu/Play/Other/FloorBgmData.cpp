#include "stdafx.h"
#include "FloorBgmData.h"

#include "Assets.generated.h"

namespace Play
{
	BgmInfo GetFloorData(int floor)
	{
		// 1-6F	とけいとう
		if (floor <= 6) return {AssetBgms::tokeitou, 29.0909, 116.3636};
		// 7F	OBAKE♪Dance
		if (floor <= 7) return {AssetBgms::obake_dance, 18.6345, 113.7288};
		// 8-12F	泡沫の祈り
		if (floor <= 12) return {AssetBgms::houmatsu_no_inori, 2.4000, 163.2000};
		// 13F	慈しみの雨、白
		if (floor <= 13) return {AssetBgms::itsukushimi_no_ame_shiro, 2.1818, 98.1818};
		// 14-18F	オオカミと七匹のキノコ
		if (floor <= 18) return {AssetBgms::okami_to_nanahiki_no_kinoko, 4.0000, 116.0000};
		// 19-22F	木々と水と光
		if (floor <= 22) return {AssetBgms::kigi_to_mizu_to_hikari, 4.2000, 71.1700};
		// 23F	オオキナヨゾラ
		if (floor <= 23) return {AssetBgms::okinayozora, 4.0000, 121.3333};
		// 24-26F	月光の銀世界
		if (floor <= 26) return {AssetBgms::gekkou_no_ginsekai, 0.0000, 153.3333};
		// 27-28F	荒び舞う雪、銀
		if (floor <= 28) return {AssetBgms::arabi_mau_yuki, 0.0000, 112.0000};
		// 29-30F	錆を抱く夕陽と楓
		if (floor <= 30) return {AssetBgms::sabi_wo_idaku_yuhi_to_kaede, 12.7500, 104.0403};
		// 31F	風謳う乱葉、緋
		if (floor <= 31) return {AssetBgms::kazeutauranyou_hi, 6.0000, 126.0000};
		// 32-35F	OOOOOO
		if (floor <= 35) return {AssetBgms::oooooo, 10.0000, 115.0000};
		// 36-40F	風襲
		if (floor <= 40) return {AssetBgms::hushu, 7.5000, 93.7500};
		// 41F	釼囓禍燧黙緊弩
		if (floor <= 41) return {AssetBgms::kenzetsukasuimokukindo, 0.0000, 103.5294};
		// 42-45F	知の遺産
		if (floor <= 45) return {AssetBgms::chi_no_isan, 30.9092, 118.1818};
		// 46-50F	Dear My Rabbit
		return {AssetBgms::dear_my_rabbit, 14.5454, 130.9091};
	}
}
