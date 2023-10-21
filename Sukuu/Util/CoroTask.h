#pragma once

namespace Util
{
	using CoroTask = void;
	using CoroTaskCall = boost::coroutines2::coroutine<CoroTask>::pull_type;
	using CoroTaskYield = boost::coroutines2::coroutine<CoroTask>::push_type;
	using CoroTaskFunc = std::function<CoroTask(CoroTaskYield&)>;
}
