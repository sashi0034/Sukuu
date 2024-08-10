#pragma once

namespace Util
{
#if _DEBUG
	void InitVisualStudioHotReloadDetectorAddon();

	/// @brief Visual Studio のホットリロードが行われたかを検出する (検出が数フレームずれる可能性あり)
	bool IsVisualStudioHotReloaded();
#endif
}
