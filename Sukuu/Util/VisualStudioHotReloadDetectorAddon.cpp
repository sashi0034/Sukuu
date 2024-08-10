#include "stdafx.h"
#include "VisualStudioHotReloadDetectorAddon.h"

namespace
{
	bool s_visualStudioHotReloaded = false;

	class VisualStudioHotReloadDetectorAddon : public IAddon
	{
		DirectoryWatcher m_watcher{};

		bool m_reloadedBuffer{};
		double m_lastReloadedTimestamp{};

		bool init() override
		{
			const auto& exeFile = FileSystem::ModulePath();
			const auto exeDir = FileSystem::ParentPath(exeFile);
			m_watcher = DirectoryWatcher(FileSystem::PathAppend(exeDir, U"Intermediate"));
			return true;
		}

		bool update() override
		{
			s_visualStudioHotReloaded = false;

			// 実行直後にも retrieveChanges されてしまうようなので、数秒待ってから retrieveChanges する
			if (Scene::Time() > 1.0)
			{
				for (auto&& change : m_watcher.retrieveChanges())
				{
					// Console.writeln(U"{}: {}"_fmt(Scene::FrameCount(), change.path));

					if (FileSystem::Extension(change.path) != U"obj")
					{
						// どのファイルがホットリロードと密接に関わってるか分からなかったので、取り敢えず obj ファイルの変更を検知するようにしてみる
						continue;
					}

					// まずは m_reloadedBuffer を true にする
					m_reloadedBuffer = true;
					m_lastReloadedTimestamp = Scene::Time();
				}
			}

			if (m_reloadedBuffer)
			{
				// s_visualStudioHotReloaded を更新するのは、m_reloadedBuffer を true にしたあと数秒経過したときにする
				if (Scene::Time() - m_lastReloadedTimestamp > 1.0)
				{
					s_visualStudioHotReloaded = true;
					m_reloadedBuffer = false;
				}
			}

			return true;
		}
	};
}

namespace Util
{
	void InitVisualStudioHotReloadDetectorAddon()
	{
		Addon::Register<VisualStudioHotReloadDetectorAddon>(U"VisualStudioHotReloadDetectorAddon");
	}

	bool IsVisualStudioHotReloaded()
	{
		return s_visualStudioHotReloaded;
	}
}
