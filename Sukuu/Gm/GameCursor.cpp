#include "stdafx.h"
#include "GameCursor.h"

#include "Assets.generated.h"
#include "Constants.h"

namespace
{
	bool s_hide{};
}

namespace Gm
{
	void RequestHideGameCursor()
	{
		s_hide = true;
	}

	void UpdateGameCursor()
	{
		Cursor::RequestStyle(CursorStyle::Hidden);
		if (not s_hide)
		{
			TextureAsset(AssetImages::cursor).resized(Point::One() * Constants::CursorSize_64).drawAt(Cursor::PosF());
		}
		s_hide = false;
	}
}
