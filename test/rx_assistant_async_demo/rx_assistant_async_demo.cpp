#include <memory>
#include <thread>

#include <rx_assistant.h>

int main(void)
{
	auto assist= std::make_shared<assistant::Assistant_v2>();
	rx_assistant::rx_assistant_factory factory(assist);

	assistant::HttpRequest req_get(
		"https://postman-echo.com/get?foo1=bar1&foo2=bar2");

	{
	auto obs = factory.create_with_delay(req_get, 3000, rxcpp::observe_on_new_thread());
	auto obs_onnext = [](const rx_assistant::HttpResult &result){
		result.res;
		printf("%d\t%s\nBody:\n%s\n\n", result.res.status_code,
			result.res.effective_url.c_str(), result.res.body.c_str());
	};
	obs.subscribe(obs_onnext);
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
}




	std::this_thread::sleep_for(std::chrono::milliseconds(3000));


	return 0;
}