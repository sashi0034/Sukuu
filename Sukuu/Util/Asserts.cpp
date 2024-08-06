#include "stdafx.h"
#include "Asserts.h"

bool Util::AssertStrongly(bool condition, const std::source_location& location)
{
	if (condition)
	{
		[[likely]] return condition;
	}

	const Font font{24};

	System::Sleep(1);

	while (System::Update())
	{
		const String s = U"Assertion failed at\n{} ({})\n{}\n\nPress 'Escape' to continue"_fmt(
			Unicode::Widen(location.file_name()),
			location.line(),
			Unicode::Widen(location.function_name())
		);

		font(s).drawAt(Scene::Center(), Palette::Red);

		if (KeyEscape.pressed()) break;
	}

	return false;
}
