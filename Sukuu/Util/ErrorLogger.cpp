#include "stdafx.h"
#include "ErrorLogger.h"

namespace
{
}

namespace Util
{
	void ErrorLog(const String& message)
	{
		// TODO: 使い勝手の良いロガーを作成
		Console.writeln(message);
	}
}
