//#include "CLEOScriptLib.h"
#include <common.h>
#include <Script.h>
#include "ScriptCommands.h"
#ifdef VC_CLEO

#include "Bike.h"
#include "Boat.h"
#include "CarCtrl.h"
#include "Clock.h"
#include "Coronas.h"
#include "Cranes.h"
#include "CutsceneMgr.h"
#include "Darkel.h"
#include "Explosion.h"
#include "Fire.h"
#include "GameLogic.h"
#include "Garages.h"
#include "General.h"
#include "Heli.h"
#include "Messages.h"
#include "Pad.h"
#include "ParticleObject.h"
#include "Phones.h"
#include "Pickups.h"
#include "PointLights.h"
#include "Pools.h"
#include "Population.h"
#include "ProjectileInfo.h"
#include "Radar.h"
#include "Restart.h"
#include "Stats.h"
#include "Streaming.h"
#include "User.h"
#include "Wanted.h"
#include "WaterLevel.h"
#include "Weather.h"
#include "Zones.h"

#include <ControllerConfig.h>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <ranges>
#include <string>
#include <vector>
// 编码转换函数声明
std::wstring
Utf8ToWide1(const std::string &utf8);

struct ScriptInfo {
	std::shared_ptr<CRunningScript> sc;

	// CRunningScript *sc = 0;
	int filesize = 0;
};
std::vector<ScriptInfo> CusomScripts; // 存储所有添加的脚本
std::map<std::string, std::wstring> ScriptsFxt;

// 返回 key 开始的索引，若无则返回 std::string::npos
size_t
StrFindKeyBeginPos(const std::string &str)
{
	for(size_t i = 0; i < str.size(); ++i) {
		char c = str[i];
		if(c == '\n' || c == '\r' || c == ';' || c == '#') break;
		if(c != ' ' && c != '\t') return i;
	}
	return std::string::npos;
}

// 返回 key 结束的索引（即第一个空白/注释位置），若无则返回 str.size()
size_t
StrFindKeyEndPos(const std::string &str)
{
	for(size_t i = 0; i < str.size(); ++i) {
		char c = str[i];
		if(c == '\n' || c == '\r' || c == ';' || c == '#') break;
		if(c == ' ' || c == '\t') return i;
	}
	return str.size();
}

// 返回文本开始索引（支持 \$）
size_t
StrFindTextBeginPos(const std::string &str)
{
	for(size_t i = 0; i < str.size(); ++i) {
		char c = str[i];
		if(c == '\n' || c == '\r') break;
		if(c == '\\' && i + 1 < str.size() && str[i + 1] == '$') {
			return i + 2; // 跳过 "\$"
		}
		if(c != ' ' && c != '\t') { return i; }
	}
	return std::string::npos;
}

// 返回行尾索引（\n 或 \r 位置）
size_t
StrFindTextEndPos(const std::string &str)
{
	for(size_t i = 0; i < str.size(); ++i) {
		if(str[i] == '\n' || str[i] == '\r') { return i; }
	}
	return str.size();
}

// 辅助函数：从 startPos 开始找 key 的结束位置
size_t
StrFindKeyEndPosFrom(const std::string &str, size_t startPos)
{
	for(size_t i = startPos; i < str.size(); ++i) {
		char c = str[i];
		if(c == '\n' || c == '\r' || c == ';' || c == '#') break;
		if(c == ' ' || c == '\t') return i;
	}
	return str.size(); // 到行尾都是 key
}

// 辅助函数：从 startPos 开始找 text 的开始位置
size_t
StrFindTextBeginPosFrom(const std::string &str, size_t startPos)
{
	for(size_t i = startPos; i < str.size(); ++i) {
		char c = str[i];
		if(c == '\n' || c == '\r') break;
		if(c == '\\' && i + 1 < str.size() && str[i + 1] == '$') {
			return i + 2; // 跳过 "\$"
		}
		if(c != ' ' && c != '\t') { return i; }
	}
	return std::string::npos;
}
void
LoadFix(std::string &line)
{
	// 主逻辑
	size_t keyBegin = StrFindKeyBeginPos(line);
	if(keyBegin != std::string::npos) {
		size_t keyEnd = StrFindKeyEndPosFrom(line, keyBegin);
		std::string key = line.substr(keyBegin, keyEnd - keyBegin);

		size_t textBegin = StrFindTextBeginPosFrom(line, keyEnd);
		if(textBegin != std::string::npos) {
			size_t textEnd = StrFindTextEndPos(line); // 你可以复用之前的 StrFindTextEndPos
			std::string text = line.substr(textBegin, textEnd - textBegin);
			ScriptsFxt[key] = Utf8ToWide1(text);
			TheText.AddKeyValue(key.c_str(), (wchar *)ScriptsFxt[key].c_str());
			// 创建条目（传入 std::string，内部可转为 const char* 如果需要）
			// CustomTextEntry *entry = new CustomTextEntry(key.c_str(), text.c_str());
		}
	}
}

// Fxt文本加载
void
CTheScripts::LoadCustomScriptsFxt()
{
	namespace fs = std::filesystem;
	std::string scriptDir = "CLEO\\CLEO_TEXT";

	// 清空旧数据（可选）
	// g_CleoFileBuffers.clear();

	fs::create_directories(scriptDir);

	for(const auto &entry : fs::directory_iterator(scriptDir)) {
		if(entry.is_regular_file()) {
			auto ext = entry.path().extension().string();
			// 转为小写比较（防止 .CS 或 .Cs）
			std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
			if(ext == ".fxt") {
				std::ifstream file(entry.path(), std::ios::binary);
				if(!file) continue;

				// 获取文件大小
				// file.seekg(0, std::ios::end);
				// size_t fileSize = static_cast<size_t>(file.tellg());
				// file.seekg(0, std::ios::beg);

				// 读取全部内容
				// std::vector<uint8> buffer(fileSize);
				// file.read(reinterpret_cast<char *>(buffer.data()), fileSize);
				std::string line;
				while(std::getline(file, line)) {
					LoadFix(line);
					// 处理每一行（line 不包含换行符）
					// std::cout << "读取到: " << line << '\n';
				}

				// g_CleoFileBuffers.push_back(std::move(buffer));
			}
		}
	}
}
// 自定义cleo加载
void
CTheScripts::LoadCustomScripts()
{
	namespace fs = std::filesystem;
	std::string scriptDir = "CLEO";

	fs::create_directories(scriptDir);

	for(const auto &entry : fs::directory_iterator(scriptDir)) {
		if(entry.is_regular_file()) {
			auto ext = entry.path().extension().string();
			// 转为小写比较
			std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
			if(ext == ".cs") {
				std::ifstream file(entry.path(), std::ios::binary);
				if(!file) continue;

				// 获取文件大小
				file.seekg(0, std::ios::end);
				size_t fileSize = static_cast<size_t>(file.tellg());
				file.seekg(0, std::ios::beg);

				// 读取全部内容
				std::vector<uint8> buffer(fileSize);
				file.read(reinterpret_cast<char *>(buffer.data()), fileSize);

				uint32 adderss = 0;

				// this->m_dwBaseIp = (unsigned int)this->m_pCodeData - (unsigned int)game.Scripts.Space;
				// adderss = (unsigned int)buffer.data() - (unsigned int)ScriptSpace.data(;
				adderss = static_cast<uint32>(ScriptSpace.size());

				ScriptSpace.insert(ScriptSpace.end(), buffer.begin(), buffer.end());
				auto scrip = std::make_shared<CRunningScript>();
				// scrip->RemoveScriptFromList(&CTheScripts::pActiveScripts); // 确保脚本不在活动列表中

				// 脚本初始化
				scrip->Init();
				scrip->SetIP(adderss);
				scrip->base_nIp = scrip->m_nIp;
				scrip->sctype = SCRIPT_TYPE_CUSTOM;

				strcpy(scrip->m_bScriptFileName, entry.path().stem().string().c_str());
				scrip->m_bScriptFileName[63] = '\0'; // 确保字符串终止
				scrip->m_abScriptName[7] = '\0';     // 确保字符串终止
				scrip->m_bIsActive = true;

				ScriptInfo sca;
				sca.sc = scrip;
				sca.filesize = fileSize;
				// 添加到记录列表
				CusomScripts.push_back(sca);
				// 添加到激活脚本列表
				scrip->AddScriptToList(&CTheScripts::pActiveScripts);
			}
		}
	}
	// 加载fxt文本
	LoadCustomScriptsFxt();
}

#include <Windows.h>

// 脚本op命令执行
int8
CRunningScript::ProcessCleoScripts(int32 command)
{
	if(command != COMMAND_CLEO_IS_KEY_PRESSED) printf("");
	// int index = command - 0x0A00;
	switch(command) {
	// ────────────────────────────────
	// 所有 DUMMY 系列：只有 DUMMY19 返回 1
	// ────────────────────────────────
	case COMMAND_CLEO_DUMMY1:
	case COMMAND_CLEO_DUMMY2:
	case COMMAND_CLEO_DUMMY3:
	case COMMAND_CLEO_DUMMY4:
	case COMMAND_CLEO_DUMMY5:
	case COMMAND_CLEO_DUMMY6:
	case COMMAND_CLEO_DUMMY7:
	case COMMAND_CLEO_DUMMY8:
	case COMMAND_CLEO_DUMMY9:
	case COMMAND_CLEO_DUMMY10:
	case COMMAND_CLEO_DUMMY11:
	case COMMAND_CLEO_DUMMY12:
	case COMMAND_CLEO_DUMMY13:
	case COMMAND_CLEO_DUMMY14:
	case COMMAND_CLEO_DUMMY15:
	case COMMAND_CLEO_DUMMY16:
	case COMMAND_CLEO_DUMMY17:
	case COMMAND_CLEO_DUMMY18:
	case COMMAND_CLEO_DUMMY19: return 0;

	// ────────────────────────────────
	// MEMORY_WRITE / MEMORY_READ 系列
	// ────────────────────────────────
	case COMMAND_CLEO_MEMORY_WRITE:
	case COMMAND_CLEO_MEMORY_WRITE_2:

	case COMMAND_CLEO_MEMORY_READ:
	case COMMAND_CLEO_MEMORY_READ_2:
		char buffer[256];
		sprintf(buffer, "该脚本使用了读写内存的指令，无法使用 脚本名:%s", m_bScriptFileName);
		MessageBoxA(NULL, buffer, "错误", MB_OK);

		if(next) m_nIp = next->base_nIp;
		RemoveScriptFromList(&CTheScripts::pActiveScripts);

		return 0;

	// ────────────────────────────────
	// TERMINATE_CUSTOM_THREAD 系列
	// ────────────────────────────────
	case COMMAND_CLEO_TERMINATE_CUSTOM_THREAD:
	case COMMAND_CLEO_TERMINATE_CUSTOM_THREAD_2:
		RemoveScriptFromList(&CTheScripts::pActiveScripts);
		CusomScripts.erase(std::remove_if(CusomScripts.begin(), CusomScripts.end(), [this](const ScriptInfo &info) { return info.sc.get() == this; }),
		                   CusomScripts.end());

		return 0;

	// ────────────────────────────────
	// START_CUSTOM_THREAD_VSTRING 系列
	// （注意：0x0600 是 CLEO2 版本，视为旧版）
	// ────────────────────────────────
	case COMMAND_CLEO_START_CUSTOM_THREAD_VSTRING:   // 0x0600
	case COMMAND_CLEO_START_CUSTOM_THREAD_VSTRING_2: // 0x0A92
		return 0;

	// ────────────────────────────────
	// TERMINATE_NAMED_CUSTOM_THREAD 系列
	// ────────────────────────────────
	case COMMAND_CLEO_TERMINATE_NAMED_CUSTOM_THREAD:
	case COMMAND_CLEO_TERMINATE_NAMED_CUSTOM_THREAD_2: {
		uint8 ret = 0;
		char name[8];
		CTheScripts::ReadTextLabelFromScript(&m_nIp, name);
		name[7] = '\0';
		m_nIp += 8;
		bool found = false;
		CRunningScript *serch;
		for(auto &info : CusomScripts) {
			serch = info.sc.get();
			if(strcmp(serch->m_abScriptName, name) == 0) {
				ret = 1;
				serch->RemoveScriptFromList(&CTheScripts::pActiveScripts);
				CusomScripts.erase(std::remove_if(CusomScripts.begin(), CusomScripts.end(),
				                                  [serch](const ScriptInfo &info) { return info.sc.get() == serch; }),
				                   CusomScripts.end());
				found = true;
				break;
			}
		}
		if(found)
			UpdateCompareFlag(true);
		else
			UpdateCompareFlag(false);
		return ret;
	}
		return 0;
	case COMMAND_CLEO_START_CUSTOM_THREAD:
		// 感觉没必要
		return 0;
	// ────────────────────────────────
	// CALL 系列（4 个变体）
	// ────────────────────────────────
	case COMMAND_CLEO_CALL:
	case COMMAND_CLEO_CALL_2: return 0;
	case COMMAND_CLEO_CALL_FUNCTION:
	case COMMAND_CLEO_CALL_FUNCTION_2: return 0;
	case COMMAND_CLEO_CALL_METHOD:
	case COMMAND_CLEO_CALL_METHOD_2: return 0;

	case COMMAND_CLEO_CALL_FUNCTION_METHOD:
	case COMMAND_CLEO_CALL_FUNCTION_METHOD_2: return 0;

	// ────────────────────────────────
	// GET_STRUCT 系列
	// ────────────────────────────────
	case COMMAND_CLEO_GET_CHAR_STRUCT:
	case COMMAND_CLEO_GET_CHAR_STRUCT_2:
		CollectParameters(&m_nIp, 1);
		ScriptParams[0] = (int32)CPools::GetPedPool()->GetAt(ScriptParams[0]);
		StoreParameters(&m_nIp, 1);
		return 0;
	case COMMAND_CLEO_GET_CAR_STRUCT:
	case COMMAND_CLEO_GET_CAR_STRUCT_2:
		CollectParameters(&m_nIp, 1);
		ScriptParams[0] = (int32)CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		StoreParameters(&m_nIp, 1);
		return 0;

	case COMMAND_CLEO_GET_OBJECT_STRUCT:
	case COMMAND_CLEO_GET_OBJECT_STRUCT_2:
		CollectParameters(&m_nIp, 1);
		ScriptParams[0] = (int32)CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		StoreParameters(&m_nIp, 1);
		return 0;

	// ────────────────────────────────
	// GET_HANDLE 系列
	// ────────────────────────────────
	case COMMAND_CLEO_GET_CHAR_HANDLE:
	case COMMAND_CLEO_GET_CHAR_HANDLE_2:
		CollectParameters(&m_nIp, 1);
		ScriptParams[0] = (int32)CPools::GetPedPool()->GetId(ScriptParams[0]);
		StoreParameters(&m_nIp, 1);
		return 0;

	case COMMAND_CLEO_GET_CAR_HANDLE:
	case COMMAND_CLEO_GET_CAR_HANDLE_2:
		CollectParameters(&m_nIp, 1);
		ScriptParams[0] = (int32)CPools::GetVehiclePool()->GetId(ScriptParams[0]);
		StoreParameters(&m_nIp, 1);
		return 0;

	case COMMAND_CLEO_GET_OBJECT_HANDLE:
	case COMMAND_CLEO_GET_OBJECT_HANDLE_2:
		CollectParameters(&m_nIp, 1);
		ScriptParams[0] = (int32)CPools::GetObjectPool()->GetId(ScriptParams[0]);
		StoreParameters(&m_nIp, 1);
		return 0;

	// ────────────────────────────────
	// THREAD POINTER 系列
	// ────────────────────────────────
	case COMMAND_CLEO_GET_THREAD_POINTER:
	case COMMAND_CLEO_GET_THREAD_POINTER_2:
		ScriptParams[0] = (int32)this;
		StoreParameters(&m_nIp, 1);
		return 0;
		// 这个真能用到吗
	case COMMAND_CLEO_GET_NAMED_THREAD_POINTER:
	case COMMAND_CLEO_GET_NAMED_THREAD_POINTER_2: return 0;

	// ────────────────────────────────
	// IS_KEY_PRESSED 系列
	// ────────────────────────────────
	case COMMAND_CLEO_IS_KEY_PRESSED:
	case COMMAND_CLEO_IS_KEY_PRESSED_2:
		CollectParameters(&m_nIp, 1);

		UpdateCompareFlag(ControlsManager.GetIsKeyboardKeyDown((RsKeyCodes)ScriptParams[0]));

		return 0;

	// ────────────────────────────────
	// FIND_RANDOM 系列
	// ────────────────────────────────
	case COMMAND_CLEO_FIND_RANDOM_CHAR:
	case COMMAND_CLEO_FIND_RANDOM_CHAR_2: return 0;

	case COMMAND_CLEO_FIND_RANDOM_CAR:
	case COMMAND_CLEO_FIND_RANDOM_CAR_2: return 0;

	case COMMAND_CLEO_FIND_RANDOM_OBJECT:
	case COMMAND_CLEO_FIND_RANDOM_OBJECT_2: return 0;

	// ────────────────────────────────
	// CALL_POP_FLOAT 系列
	// ────────────────────────────────
	case COMMAND_CLEO_CALL_POP_FLOAT:
	case COMMAND_CLEO_CALL_POP_FLOAT_2: return 0;

	// ────────────────────────────────
	// MATH 系列
	// ────────────────────────────────
	case COMMAND_CLEO_MATH_EXP:
	case COMMAND_CLEO_MATH_EXP_2: return 0;

	case COMMAND_CLEO_MATH_LOG:
	case COMMAND_CLEO_MATH_LOG_2: return 0;

	// ────────────────────────────────
	// SCM FUNCTION 系列
	// ────────────────────────────────
	case COMMAND_CLEO_CALL_SCM_FUNCTION:
	case COMMAND_CLEO_CALL_SCM_FUNCTION_2: return 0;

	case COMMAND_CLEO_SCM_FUNCTION_RET:
	case COMMAND_CLEO_SCM_FUNCTION_RET_2: return 0;

	// ────────────────────────────────
	// LABEL/VAR OFFSET 系列
	// ────────────────────────────────
	case COMMAND_CLEO_GET_LABEL_OFFSET:
	case COMMAND_CLEO_GET_LABEL_OFFSET_2: return 0;

	case COMMAND_CLEO_GET_VAR_OFFSET:
	case COMMAND_CLEO_GET_VAR_OFFSET_2: return 0;

	// ────────────────────────────────
	// 所有其他 opcode（无数字后缀 or 单一版本）→ return 0
	// ────────────────────────────────
	case COMMAND_CLEO_GET_GAME_VERSION:
		ScriptParams[0] = 0; // 0 = VC  ???
		StoreParameters(&m_nIp, 1);
		return 0;

	case COMMAND_CLEO_OPCODE_0A8E: return 0;
	case COMMAND_CLEO_OPCODE_0A8F: return 0;
	case COMMAND_CLEO_OPCODE_0A90: return 0;
	case COMMAND_CLEO_OPCODE_0A91: return 0;
	case COMMAND_CLEO_OPCODE_0A99: return 0;
	case COMMAND_CLEO_OPCODE_0A9A: return 0;
	case COMMAND_CLEO_OPCODE_0A9B: return 0;
	case COMMAND_CLEO_OPCODE_0A9C: return 0;
	case COMMAND_CLEO_OPCODE_0A9D: return 0;
	case COMMAND_CLEO_OPCODE_0A9E: return 0;
	case COMMAND_CLEO_OPCODE_0AA0: return 0;
	case COMMAND_CLEO_OPCODE_0AA1: return 0;
	case COMMAND_CLEO_OPCODE_0AA2: return 0;
	case COMMAND_CLEO_OPCODE_0AA3: return 0;
	case COMMAND_CLEO_OPCODE_0AA4: return 0;
	case COMMAND_CLEO_OPCODE_0AAB: return 0;
	case COMMAND_CLEO_OPCODE_0AB3: return 0;
	case COMMAND_CLEO_OPCODE_0AB4: return 0;
	case COMMAND_CLEO_OPCODE_0AB7: return 0;
	case COMMAND_CLEO_OPCODE_0AB8: return 0;
	case COMMAND_CLEO_OPCODE_0ABD: return 0;
	case COMMAND_CLEO_OPCODE_0ABE: return 0;
	case COMMAND_CLEO_OPCODE_0ABF: return 0;
	case COMMAND_CLEO_OPCODE_0AC8: return 0;
	case COMMAND_CLEO_OPCODE_0AC9: return 0;
	case COMMAND_CLEO_OPCODE_0ACA: return 0;
	case COMMAND_CLEO_OPCODE_0ACB: return 0;
	case COMMAND_CLEO_OPCODE_0ACC: return 0;
	case COMMAND_CLEO_OPCODE_0ACE: return 0;
	case COMMAND_CLEO_OPCODE_0ACF: return 0;
	case COMMAND_CLEO_OPCODE_0AD0: return 0;
	case COMMAND_CLEO_OPCODE_0AD1: return 0;
	case COMMAND_CLEO_OPCODE_0AD3: return 0;
	case COMMAND_CLEO_OPCODE_0AD4: return 0;
	case COMMAND_CLEO_OPCODE_0AD5: return 0;
	case COMMAND_CLEO_OPCODE_0AD6: return 0;
	case COMMAND_CLEO_OPCODE_0AD7: return 0;
	case COMMAND_CLEO_OPCODE_0AD8: return 0;
	case COMMAND_CLEO_OPCODE_0AD9: return 0;
	case COMMAND_CLEO_OPCODE_0ADA: return 0;
	case COMMAND_CLEO_OPCODE_0ADB: return 0;
	case COMMAND_CLEO_OPCODE_0ADC: return 0;
	case COMMAND_CLEO_OPCODE_0ADD: return 0;
	case COMMAND_CLEO_OPCODE_0ADE: return 0;
	case COMMAND_CLEO_OPCODE_0ADF: return 0;
	case COMMAND_CLEO_OPCODE_0AE0: return 0;
	case COMMAND_CLEO_OPCODE_0AE4: return 0;
	case COMMAND_CLEO_OPCODE_0AE5: return 0;
	case COMMAND_CLEO_OPCODE_0AE6: return 0;
	case COMMAND_CLEO_OPCODE_0AE7: return 0;
	case COMMAND_CLEO_OPCODE_0AE8: return 0;
	case COMMAND_CLEO_OPCODE_0ACD: return 0;

	// CLEO 2 opcodes (single version)
	case COMMAND_CLEO_IS_BUTTON_PRESSED_ON_PAD: return 0;
	case COMMAND_CLEO_EMULATE_BUTTON_PRESS_ON_PAD: return 0;
	case COMMAND_CLEO_IS_CAMERA_IN_WIDESCREEN_MODE: return 0;
	case COMMAND_CLEO_GET_MODEL_ID_FROM_WEAPON_ID: return 0;
	case COMMAND_CLEO_GET_WEAPON_ID_FROM_MODEL_ID: return 0;
	case COMMAND_CLEO_SET_MEM_OFFSET: return 0;
	case COMMAND_CLEO_GET_CURRENT_WEATHER: return 0;

	case COMMAND_CLEO_SHOW_TEXT_POSITION: {

		CollectParameters(&m_nIp, 3);

		CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].m_fAtX = *(float *)&ScriptParams[0];
		CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].m_fAtY = *(float *)&ScriptParams[1];
		wchar *text = (wchar *)ScriptParams[3];
		uint16 len = CMessages::GetWideStringLength(text);
		for(uint16 i = 0; i < len; i++) CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].m_Text[i] = text[i];
		for(uint16 i = len; i < SCRIPT_TEXT_MAX_LENGTH; i++) CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].m_Text[i] = 0;
		++CTheScripts::NumberOfIntroTextLinesThisFrame;
	}

		return 0;
	case COMMAND_CLEO_SHOW_FORMATTED_TEXT_POSITION: return 0;

	case COMMAND_CLEO_PLAY_ANIMATION: return 0;

	// CLEO 2.1 opcodes
	case COMMAND_CLEO_SET_CLEO_ARRAY: return 0;
	case COMMAND_CLEO_GET_CLEO_ARRAY: return 0;

	case COMMAND_CLEO_GET_CLEO_ARRAY_OFFSET: return 0;
	case COMMAND_CLEO_GET_CLEO_ARRAY_SCRIPT: return 0;
	case COMMAND_CLEO_GET_PLATFORM: return 0;

	// BIT 操作（无后缀）
	case COMMAND_CLEO_BIT_AND: return 0;
	case COMMAND_CLEO_BIT_OR: return 0;
	case COMMAND_CLEO_BIT_XOR: return 0;
	case COMMAND_CLEO_BIT_NOT: return 0;
	case COMMAND_CLEO_BIT_MOD: return 0;
	case COMMAND_CLEO_BIT_SHR: return 0;
	case COMMAND_CLEO_BIT_SHL: return 0;

	default: return 0;
	};

	return -1;
}

// ==================== 编码转换函数 ====================

std::wstring
Utf8ToWide1(const std::string &utf8)
{
	if(utf8.empty()) return L"";

	int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
	wchar_t *wstr = new wchar_t[len];
	MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wstr, len);
	std::wstring result(wstr);
	delete[] wstr;
	return result;
}

#endif // VC_CLEO
