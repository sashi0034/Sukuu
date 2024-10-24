#pragma once

#if SIV3D_BUILD(DEBUG)
namespace Util
{
	void InitLivePPAddon();

	/// @brief 現在のフレームで LivePP がホットリロードを行ったかどうか
	[[nodiscard]]
	bool IsLivePPHotReloaded();
}
#endif
