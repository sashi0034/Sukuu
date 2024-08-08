#include "stdafx.h"
#include "Asserts.h"

#define BOOST_STACKTRACE_USE_WINDBG
#include <boost/stacktrace.hpp>

bool Util::AssertStrongly(bool condition, const std::source_location& location)
{
	if (condition)
	{
		[[likely]] return condition;
	}

	// スタックトレース取得
	std::ostringstream oss{};
	oss << boost::stacktrace::stacktrace() << std::endl;
	const auto stacktraces = Unicode::Widen(oss.str()).split_lines();
	String stMessage{};
	for (int i = 0; i < stacktraces.size(); ++i)
	{
		const int atIndex = stacktraces[i].indexOf(U" at ");
		stMessage += stacktraces[i].substr(0, atIndex) + U"\n\t" + stacktraces[i].substr(atIndex) + U"\n";

		// エントリーポイント Main まで取得する
		if (stacktraces[i].includes(U"# Main")) break;
	}

	// ファイル位置
	const String locationMessage = U"{}:{}\n\t{}"_fmt(
		Unicode::Widen(location.file_name()),
		location.line(),
		Unicode::Widen(location.function_name()
		));

	const Font font{24};

	// 1フレーム待ってからメッセージ表示
	System::Sleep(1);

	const ScopedRenderTarget2D rt{none};
	const Transformer2D tf{Mat3x2::Identity()};

	while (System::Update())
	{
		const String s = U"Assertion failed (Press 'Escape' to continue)\n\nlocation: {}\n\nstacktraces:\n{}"_fmt(
			locationMessage,
			stMessage
		);

		font(s).draw(Arg::topLeft = Vec2{32, 64}, Palette::Red);

		if (KeyEscape.pressed()) break;
	}

	return false;
}
