#include "stdafx.h"
#include "ErrorLogger.h"

#include "Asserts.h"

namespace
{
	constexpr StringView ErrorLogFileName = U"error_log.txt";
	constexpr int maxErrorLogCount = 10000;

	class ErrorLoggerAddon;
	ErrorLoggerAddon* s_instance{};

	class ErrorLoggerAddon : public IAddon
	{
		TextWriter m_writer{};
		bool m_outputToFile{};
		int m_count{};

		bool init() override
		{
			Util::AssertStrongly(s_instance == nullptr);
			s_instance = this;

#if  _DEBUG
			m_outputToFile = false;
#else
			m_outputToFile = true;
#endif

			if (m_outputToFile)
			{
				m_writer.open(ErrorLogFileName);
			}
			return true;
		}

	public:
		~ErrorLoggerAddon() override
		{
			m_writer.close();
			s_instance = nullptr;
		}

		void Write(const String& message)
		{
			m_count++;

			if (not m_outputToFile)
			{
				Console.writeln(message);
				return;
			}

			if (m_count == maxErrorLogCount)
			{
				m_writer.writeln(U"The log file is too big. It will be truncated.");
				return;
			}
			else if (m_count > maxErrorLogCount)
			{
				return;
			}

			m_writer.writeln(message);
		}
	};
}

namespace Util
{
	void InitErrorLoggerAddon()
	{
		Addon::Register<ErrorLoggerAddon>(U"ErrorLoggerAddon");
	}

	void ErrorLog(const String& message)
	{
		if (not AssertStrongly(s_instance != nullptr)) return;

		s_instance->Write(message);
	}
}
