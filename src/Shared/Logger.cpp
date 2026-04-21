#include "pch.h"
#include "Logger.h"
#include "StrHelper.h"
//#include <spdlog/sinks/rotating_file_sink.h>

bool Logger::Initialize(int logLevel, std::wstring logFileName, int logArchiveAboveSize, int logMaxArchiveFiles) noexcept {

	return true;
}

void Logger::SetLevel(int logLevel) noexcept {
	assert(false);

// _logger->flush();
// _logger->set_level(logLevel);

	static const char* LOG_LEVELS[7] = {
		"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL", "OFF"
	};
	Info(""); // ""format("当前日志级别: {}", LOG_LEVELS[logLevel]));
}

void Logger::_Log(int logLevel, std::string_view msg, const SourceLocation& location) noexcept {
	if (msg.empty())return;

	// 只检查一次是否附加了调试器
	static const bool isDebuggerPresent = IsDebuggerPresent();
	if (isDebuggerPresent && logLevel >= 2) {
		// 警告或更高等级的日志也记录到调试器
		if (msg.back() == '\n') {
			OutputDebugString(StrHelper::Concat(L"[LOG] ", StrHelper::UTF8ToUTF16(msg)).c_str());
		} else {
			OutputDebugString(StrHelper::Concat(L"[LOG] ", StrHelper::UTF8ToUTF16(msg), L"\n").c_str());
		}
	}

}
