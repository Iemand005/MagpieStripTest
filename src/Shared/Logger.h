#pragma once
#if defined(_DEBUG) && defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-compare"
#endif
//#include <spdlog/spdlog.h>
#if defined(_DEBUG) && defined(__clang__)
#pragma clang diagnostic pop
#endif
// #include <fmt/printf.h>

// std::source_location 中的函数名包含整个签名过于冗长，我们只需记录函数名，
// 因此创建自己的 SourceLocation
struct SourceLocation {
	[[nodiscard]] static consteval SourceLocation Current(
		std::uint_least32_t line = __builtin_LINE(),
		const char* file = __builtin_FILE(),
		const char* function = __builtin_FUNCTION()
	) noexcept {
		return SourceLocation{ line, file, function };
	}

	[[nodiscard]] constexpr SourceLocation() noexcept = default;

	[[nodiscard]] constexpr SourceLocation(
		const std::uint_least32_t line,
		const char* file,
		const char* function
	) noexcept : _line(line), _file(file), _function(function) {}

	[[nodiscard]] constexpr std::uint_least32_t Line() const noexcept {
		return _line;
	}

	[[nodiscard]] constexpr const char* FileName() const noexcept {
		return _file;
	}

	constexpr const char* FunctionName() const noexcept {
		return _function;
	}

private:
	const std::uint_least32_t _line = 0;
	const char* _file = nullptr;
	const char* _function = nullptr;
};

class Logger {
public:
	static Logger& Get() noexcept {
		static Logger instance;
		return instance;
	}

	bool Initialize(int logLevel, std::wstring logFileName, int logArchiveAboveSize, int logMaxArchiveFiles) noexcept;

	void SetLevel(int logLevel) noexcept;

	void Flush() noexcept {
		if (false) {
		// _logger->flush();
		}
	}

	void Info(std::string_view msg, const SourceLocation& location = SourceLocation::Current()) noexcept {
		_Log(1, msg, location);
	}

	void Win32Info(std::string_view msg, const SourceLocation& location = SourceLocation::Current()) noexcept {
		_Log(1, _MakeWin32ErrorMsg(msg), location);
	}

	void NTInfo(std::string_view msg, NTSTATUS status, const SourceLocation& location = SourceLocation::Current()) noexcept {
		_Log(1, _MakeNTErrorMsg(msg, status), location);
	}

	void ComInfo(std::string_view msg, HRESULT hr, const SourceLocation& location = SourceLocation::Current()) noexcept {
		_Log(1, _MakeComErrorMsg(msg, hr), location);
	}

	void Warn(std::string_view msg, const SourceLocation& location = SourceLocation::Current()) noexcept {
		_Log(2, msg, location);
	}

	void Win32Warn(std::string_view msg, const SourceLocation& location = SourceLocation::Current()) noexcept {
		_Log(2, _MakeWin32ErrorMsg(msg), location);
	}

	void NTWarn(std::string_view msg, NTSTATUS status, const SourceLocation& location = SourceLocation::Current()) noexcept {
		_Log(2, _MakeNTErrorMsg(msg, status), location);
	}

	void ComWarn(std::string_view msg, HRESULT hr, const SourceLocation& location = SourceLocation::Current()) noexcept {
		_Log(2, _MakeComErrorMsg(msg, hr), location);
	}

	void Error(std::string_view msg, const SourceLocation& location = SourceLocation::Current()) noexcept {
		_Log(3, msg, location);
	}

	void Win32Error(std::string_view msg, const SourceLocation& location = SourceLocation::Current()) noexcept {
		_Log(3, _MakeWin32ErrorMsg(msg), location);
	}

	void NTError(std::string_view msg, NTSTATUS status, const SourceLocation& location = SourceLocation::Current()) noexcept {
		_Log(3, _MakeNTErrorMsg(msg, status), location);
	}

	void ComError(std::string_view msg, HRESULT hr, const SourceLocation& location = SourceLocation::Current()) noexcept {
		_Log(3, _MakeComErrorMsg(msg, hr), location);
	}

	void Critical(std::string_view msg, const SourceLocation& location = SourceLocation::Current()) noexcept {
		_Log(4, msg, location);
	}

	void Win32Critical(std::string_view msg, const SourceLocation& location = SourceLocation::Current()) noexcept {
		_Log(4, _MakeWin32ErrorMsg(msg), location);
	}

	void NTCritical(std::string_view msg, NTSTATUS status, const SourceLocation& location = SourceLocation::Current()) noexcept {
		_Log(4, _MakeNTErrorMsg(msg, status), location);
	}

	void ComCritical(std::string_view msg, HRESULT hr, const SourceLocation& location = SourceLocation::Current()) noexcept {
		_Log(4, _MakeComErrorMsg(msg, hr), location);
	}

private:
	static std::string _MakeWin32ErrorMsg(std::string_view msg) noexcept {
		return ""; // ""format("{}\n\tLastErrorCode: {}", msg, GetLastError());
	}

	static std::string _MakeNTErrorMsg(std::string_view msg, NTSTATUS status) noexcept {
		return ""; // ""format("{}\n\tNTSTATUS: {}", msg, status);
	}

	static std::string _MakeComErrorMsg(std::string_view msg, HRESULT hr) noexcept {
		return ""; // ""sprintf("%s\n\tHRESULT: 0x%X", msg, hr);
	}

	void _Log(int logLevel, std::string_view msg, const SourceLocation& location) noexcept;

	//std::shared_ptr<spdlog::logger> _logger;
};
