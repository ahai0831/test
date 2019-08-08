#include <thread>
#include <chrono>
#include <tuple>
#include <new>

#include <Assistant_v2.h>
#include <slicedownload_mastercontrol.h>
#include "electron-dll-proj.h"

struct Assist_Type
{
	int64_t nLocalAddr;
	typedef std::tuple<std::unique_ptr<std::string>, std::unique_ptr<SlicedownloadMastercontrol>> DLMapValue;
	typedef std::string DLMapKey;
	typedef std::map<DLMapKey, DLMapValue> DLMap;
	DLMap downloadInfo;

	typedef std::tuple<std::unique_ptr<std::string>, std::string> RegMapValue;
	typedef std::string RegMapKey;
	typedef std::map<RegMapKey, RegMapValue> RegMap;
	RegMap regisStrings;

	std::shared_ptr<assistant::Assistant_v2> pAssist;
	Assist_Type() : nLocalAddr(0LL), pAssist(std::make_shared<assistant::Assistant_v2>())
	{
		nLocalAddr = (int64_t)(this);
	}
	/// 复制构造、默认构造、移动构造、=号操作符
	/// 要么禁用掉，要么显式写清
private:
	Assist_Type(Assist_Type const &) = delete;
	Assist_Type &operator=(Assist_Type const &) = delete;
	Assist_Type(Assist_Type &&) = delete;
};

extern "C" int64_t CreateAst()
{
	auto ast_ptr = new (std::nothrow) Assist_Type();
	return ast_ptr->nLocalAddr;
}

extern "C" char *StartSliceDownload(char *sDownloadURL, char *sDownloadPath, int64_t nAst)
{
	auto downloadid_ptr = std::make_unique<std::string>(std::move(assistant::uuid::generate()));
	char *flag = nullptr;
	if (nullptr != sDownloadPath && nullptr != sDownloadURL && strlen(sDownloadPath) > 0 && strlen(sDownloadURL) > 0)
	{
		auto ast_ptr = (Assist_Type *)nAst;
		auto downloadctrl_ptr = std::make_unique<SlicedownloadMastercontrol>(ast_ptr->pAssist);
		flag = const_cast<char *>(downloadid_ptr->data());

		downloadctrl_ptr->AsyncProcess(sDownloadURL, sDownloadPath);
		auto piter = ast_ptr->downloadInfo.emplace(std::string(flag), std::make_tuple(std::move(downloadid_ptr), std::move(downloadctrl_ptr)));
	}
	return flag;
}

extern "C" char *RegisterSliceDownloadSubscription(f4download::OnNext fNext, f4download::OnComplete fComplete, char *sDownloadID, int64_t nAst)
{
	char *csRegID = nullptr;
	if (nullptr != sDownloadID)
	{
		auto ptrA_v2 = (Assist_Type *)nAst;
		auto pIter = ptrA_v2->downloadInfo.find(sDownloadID);
		if (pIter != ptrA_v2->downloadInfo.end())
		{
			auto &downloadCtrl = std::get<1>(pIter->second);
			auto speedNext = [fNext](uint64_t ns) {
				auto sspeed = std::to_string(ns);
				std::string sRaw = R"({"real_speed":")";
				sRaw += sspeed;
				sRaw += '\"';
				sRaw += '}';
				fNext(sRaw.c_str());
			};
			if (nullptr != downloadCtrl)
			{
				auto sRegID = downloadCtrl->RegSpeedCallback(speedNext, fComplete);
				auto reg_ptr = std::make_unique<std::string>(std::move(sRegID));
				csRegID = const_cast<char *>(reg_ptr->data());
				ptrA_v2->regisStrings.emplace(sRegID, std::make_tuple(std::move(reg_ptr), sDownloadID));
			}
		}
	}
	return csRegID;
}

extern "C" bool CancelSubscription(char *sRegisterID, int64_t nAst)
{
	bool bSucceed = false;
	auto ptrA_v2 = (Assist_Type *)nAst;
	auto pIter = ptrA_v2->regisStrings.find(sRegisterID);
	if (pIter != ptrA_v2->regisStrings.end())
	{
		Assist_Type::RegMapValue &tuple_strings = pIter->second;
		auto &sDownloadID = std::get<1>(tuple_strings);
		auto pIter_downloadid = ptrA_v2->downloadInfo.find(sDownloadID);
		if (pIter_downloadid != ptrA_v2->downloadInfo.end())
		{
			auto &downloadCtrl = std::get<1>(pIter_downloadid->second);
			if (nullptr != downloadCtrl)
			{
			}
		}
	}
	return bSucceed;
}

extern "C" void StopDownload(char *sDownloadID, int64_t nAst)
{
	auto ptrA_v2 = (Assist_Type *)nAst;
	auto pIter = ptrA_v2->downloadInfo.find(sDownloadID);
	if (pIter != ptrA_v2->downloadInfo.end())
	{
		auto& downloadCtrl = std::get<1>(pIter->second);
		auto another_thread = std::thread([&downloadCtrl]() {
			std::this_thread::sleep_for(std::chrono::milliseconds(5000));
			downloadCtrl->AsyncStop();
		});
		if (another_thread.joinable())
		{
			another_thread.join();
		}
	}
}

extern "C" void DestroyAst(int64_t nAst)
{
	auto ast_ptr = (Assist_Type *)nAst;
	delete ast_ptr;
}
