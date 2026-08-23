#include "common.h"

#include "FileMgr.h"
#ifdef MORE_LANGUAGES
#include "Game.h"
#endif
#include "Frontend.h"
#include "Messages.h"
#include "Text.h"
#include "Timer.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/stat.h>
#include <time.h>
#endif

wchar WideErrorString[25];

CText TheText;

//在游戏里面重新选择语言后可能不会重新加载任务文本

char Mission_TableName_backup[64]{0};


CText::CText(void)
{
	encoding = 'e';
	bHasMissionTextOffsets = false;
	bIsMissionTextLoaded = false;
	memset(szMissionTableName, 0, sizeof(szMissionTableName));
	memset(WideErrorString, 0, sizeof(WideErrorString));
	//memset(Mission_TableName_backup, 0, 64);
	//strset(Mission_TableName_backup, 0);
	
}

void
CText::Load(void)
{
	char filename[32];
	size_t offset;
	int file;
	bool tkey_loaded = false, tdat_loaded = false;
	ChunkHeader m_ChunkHeader;

	bIsMissionTextLoaded = false;
	bHasMissionTextOffsets = false;

	Unload();

	CFileMgr::SetDir("TEXT");
	switch(FrontEndMenuManager.m_PrefsLanguage){
	case CMenuManager::LANGUAGE_AMERICAN:
		sprintf(filename, "AMERICAN.GXT");
		break;
	case CMenuManager::LANGUAGE_FRENCH:
		sprintf(filename, "FRENCH.GXT");
		break;
	case CMenuManager::LANGUAGE_GERMAN:
		sprintf(filename, "GERMAN.GXT");
		break;
	case CMenuManager::LANGUAGE_ITALIAN:
		sprintf(filename, "ITALIAN.GXT");
		break;
	case CMenuManager::LANGUAGE_SPANISH:
		sprintf(filename, "SPANISH.GXT");
		break;
#ifdef MORE_LANGUAGES
	case CMenuManager::LANGUAGE_POLISH:
		sprintf(filename, "POLISH.GXT");
		break;
	case CMenuManager::LANGUAGE_RUSSIAN:
		sprintf(filename, "RUSSIAN.GXT");
		break;
	case CMenuManager::LANGUAGE_JAPANESE:
		sprintf(filename, "JAPANESE.GXT");
		break;
	case CMenuManager::LANGUAGE_CHINESE:
		sprintf(filename, "CHINESE.GXT");
		break;
#endif
	}

	file = CFileMgr::OpenFile(filename, "rb");

	offset = 0;
	while (!tkey_loaded || !tdat_loaded) {
		ReadChunkHeader(&m_ChunkHeader, file, &offset);
		if (m_ChunkHeader.size != 0) {
			if (strncmp(m_ChunkHeader.magic, "TABL", 4) == 0) {
				MissionTextOffsets.Load(m_ChunkHeader.size, file, &offset, 0x58000);
				bHasMissionTextOffsets = true;
			} else if (strncmp(m_ChunkHeader.magic, "TKEY", 4) == 0) {
				this->keyArray.Load(m_ChunkHeader.size, file, &offset);
				tkey_loaded = true;
			} else if (strncmp(m_ChunkHeader.magic, "TDAT", 4) == 0) {
				this->data.Load(m_ChunkHeader.size, file, &offset);
				tdat_loaded = true;
			} else {
				CFileMgr::Seek(file, m_ChunkHeader.size, SEEK_CUR);
				offset += m_ChunkHeader.size;
			}
		}
	}

	keyArray.Update(data.chars);
	CFileMgr::CloseFile(file);
	CFileMgr::SetDir("");
}

void
CText::Unload(void)
{
	CMessages::ClearAllMessagesDisplayedByGame();
	keyArray.Unload();
	data.Unload();
	mission_keyArray.Unload();
	mission_data.Unload();
	bIsMissionTextLoaded = false;
	memset(szMissionTableName, 0, sizeof(szMissionTableName));
}

// === GXT hot-reload (dev tool) ===
// Language-agnostic: every language the game supports (AMERICAN..SPANISH
// plus the MORE_LANGUAGES set) is watched through its own GXT file, and
// switching language in the menu silently switches the watched file.
static uint64 GxtFileTime(const char *path)
{
#if defined(_WIN32)
	WIN32_FILE_ATTRIBUTE_DATA wfd;
	if(GetFileAttributesExA(path, GetFileExInfoStandard, &wfd))
		return ((uint64)wfd.ftLastWriteTime.dwHighDateTime << 32) | wfd.ftLastWriteTime.dwLowDateTime;
	return 0;
#else
	struct stat st;
	if(stat(path, &st) == 0)
		return (uint64)st.st_mtime;
	return 0;
#endif
}

static void GxtFileNameForLanguage(char *out, int lang)
{
	switch(lang) {
	case CMenuManager::LANGUAGE_AMERICAN: strcpy(out, "TEXT/AMERICAN.GXT"); break;
	case CMenuManager::LANGUAGE_FRENCH:   strcpy(out, "TEXT/FRENCH.GXT");   break;
	case CMenuManager::LANGUAGE_GERMAN:   strcpy(out, "TEXT/GERMAN.GXT");   break;
	case CMenuManager::LANGUAGE_ITALIAN:  strcpy(out, "TEXT/ITALIAN.GXT");  break;
	case CMenuManager::LANGUAGE_SPANISH:  strcpy(out, "TEXT/SPANISH.GXT");  break;
#ifdef MORE_LANGUAGES
	case CMenuManager::LANGUAGE_POLISH:   strcpy(out, "TEXT/POLISH.GXT");   break;
	case CMenuManager::LANGUAGE_RUSSIAN:  strcpy(out, "TEXT/RUSSIAN.GXT");  break;
	case CMenuManager::LANGUAGE_JAPANESE: strcpy(out, "TEXT/JAPANESE.GXT"); break;
	case CMenuManager::LANGUAGE_CHINESE:  strcpy(out, "TEXT/CHINESE.GXT");  break;
#endif
	default: out[0] = 0; break;
	}
}

// Transactional reload: deep-copy the live tables, parse the new file,
// roll back if it is missing/broken, and re-attach the mission text that
// was active before the reload.
bool CText::ReloadFromDisk(void)
{
	CKeyEntry *savedKeys = nil;
	CKeyEntry *savedMKeys = nil;
	wchar *savedData = nil;
	wchar *savedMData = nil;
	int savedKeysN = keyArray.numEntries;
	int savedMKeysN = mission_keyArray.numEntries;
	int savedDataN = data.numChars;
	int savedMDataN = mission_data.numChars;
	CMissionTextOffsets savedOffsets = MissionTextOffsets;
	bool savedHasOffsets = bHasMissionTextOffsets;
	bool savedMissionLoaded = bIsMissionTextLoaded;
	char savedMission[sizeof(szMissionTableName)];
	memcpy(savedMission, szMissionTableName, sizeof(szMissionTableName));

	if(savedKeysN > 0) {
		savedKeys = new CKeyEntry[savedKeysN];
		memcpy(savedKeys, keyArray.entries, sizeof(CKeyEntry) * savedKeysN);
	}
	if(savedMKeysN > 0) {
		savedMKeys = new CKeyEntry[savedMKeysN];
		memcpy(savedMKeys, mission_keyArray.entries, sizeof(CKeyEntry) * savedMKeysN);
	}
	if(savedDataN > 0) {
		savedData = new wchar[savedDataN];
		memcpy(savedData, data.chars, sizeof(wchar) * savedDataN);
	}
	if(savedMDataN > 0) {
		savedMData = new wchar[savedMDataN];
		memcpy(savedMData, mission_data.chars, sizeof(wchar) * savedMDataN);
	}

	Load(); // Unload() then parse the fresh file

	if(keyArray.numEntries == 0 || data.numChars == 0) {
		// broken or missing file: free any half-parsed tables and reinstall
		// the backup (ownership of the backup arrays transfers to CText)
		Unload();
		keyArray.entries = savedKeys;           keyArray.numEntries = savedKeysN;
		mission_keyArray.entries = savedMKeys;  mission_keyArray.numEntries = savedMKeysN;
		data.chars = savedData;                 data.numChars = savedDataN;
		mission_data.chars = savedMData;        mission_data.numChars = savedMDataN;
		MissionTextOffsets = savedOffsets;
		bHasMissionTextOffsets = savedHasOffsets;
		bIsMissionTextLoaded = savedMissionLoaded;
		memcpy(szMissionTableName, savedMission, sizeof(szMissionTableName));
		return false;
	}

	// fresh tables are live; re-attach the mission text that was on screen
	CMessages::ClearAllMessagesDisplayedByGame();
	if(savedMissionLoaded && Mission_TableName_backup[0] && bHasMissionTextOffsets)
		LoadMissionText(Mission_TableName_backup);

	delete[] savedKeys;
	delete[] savedMKeys;
	delete[] savedData;
	delete[] savedMData;
	return true;
}

// Called once per frame. Polls the current language's GXT mtime every
// 250ms; a change arms a 500ms-delayed reload so a mid-write editor save
// does not get parsed half-way. The first tick only caches the mtime.
void TextHotReloadTick(void)
{
	static char sTrackedPath[64] = "";
	static uint64 sLastWrite = 0;
	static uint32 sNextCheck = 0;
	static uint32 sReloadAt = 0;
	static bool sPending = false;

	char path[64];
	GxtFileNameForLanguage(path, FrontEndMenuManager.m_PrefsLanguage);
	if(path[0] == 0)
		return;

	uint32 now = CTimer::GetTimeInMilliseconds();
	if(strcmp(path, sTrackedPath) != 0) {
		// first frame or the language switched in-game: re-anchor tracking
		// on the new file without reloading it
		strcpy(sTrackedPath, path);
		sLastWrite = GxtFileTime(path);
		sPending = false;
		return;
	}
	if(now < sNextCheck)
		return;
	sNextCheck = now + 250;
	if(!sPending) {
		uint64 w = GxtFileTime(path);
		if(w != sLastWrite) {
			sLastWrite = w;
			sPending = true;
			sReloadAt = now + 500;
		}
	} else if(now >= sReloadAt) {
		sPending = false;
		TheText.ReloadFromDisk();
	}
}
// ================================

wchar*
CText::Get(const char *key)
{
	uint8 result = false;
#if defined (FIX_BUGS) || defined(FIX_BUGS_64)
	wchar *outstr = keyArray.Search(key, data.chars, &result);
#else
	wchar *outstr = keyArray.Search(key, &result);
#endif

	if (!result && bHasMissionTextOffsets && bIsMissionTextLoaded)
#if defined (FIX_BUGS) || defined(FIX_BUGS_64)
		outstr = mission_keyArray.Search(key, mission_data.chars, &result);
#else
		outstr = mission_keyArray.Search(key, &result);
#endif
	return outstr;
}

wchar UpperCaseTable[128] = {
	128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138,
	139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
	150, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137,
	138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148,
	149, 173, 173, 175, 176, 177, 178, 179, 180, 181, 182,
	183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193,
	194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204,
	205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215,
	216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226,
	227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237,
	238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248,
	249, 250, 251, 252, 253, 254, 255
};

wchar FrenchUpperCaseTable[128] = {
	128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138,
	139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
	150, 65, 65, 65, 65, 132, 133, 69, 69, 69, 69, 73, 73,
	73, 73, 79, 79, 79, 79, 85, 85, 85, 85, 173, 173, 175,
	176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186,
	187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197,
	198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208,
	209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219,
	220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230,
	231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241,
	242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252,
	253, 254, 255
};

wchar
CText::GetUpperCase(wchar c)
{
	switch (encoding)
	{
	case 'e':
		if (c >= 'a' && c <= 'z')
			return c - 32;
		break;
	case 'f':
		if (c >= 'a' && c <= 'z')
			return c - 32;

		if (c >= 128 && c <= 255)
			return FrenchUpperCaseTable[c-128];
		break;
	case 'g':
	case 'i':
	case 's':
		if (c >= 'a' && c <= 'z')
			return c - 32;

		if (c >= 128 && c <= 255)
			return UpperCaseTable[c-128];
		break;
	default:
		break;
	}
	return c;
}

void
CText::UpperCase(wchar *s)
{
	while(*s){
		*s = GetUpperCase(*s);
		s++;
	}
}

void
CText::GetNameOfLoadedMissionText(char *outName)
{
	strcpy(outName, szMissionTableName);
}

void
CText::ReadChunkHeader(ChunkHeader *buf, int32 file, size_t *offset)
{
#ifdef THIS_IS_STUPID
	char *_buf = (char*)buf;
	for (int i = 0; i < sizeof(ChunkHeader); i++) {
		CFileMgr::Read(file, &_buf[i], 1);
		(*offset)++;
	}
#else
	// original code loops 8 times to read 1 byte with CFileMgr::Read, that's retarded
	CFileMgr::Read(file, (char*)buf, sizeof(ChunkHeader));
	*offset += sizeof(ChunkHeader);
#endif
}


void
CText::LoadMissionText(char *MissionTableName)
{
	char filename[32];
	CMessages::ClearAllMessagesDisplayedByGame();
	//处理一个疑似bug 重新加载语言不会重新加载任务文本

	strcpy(Mission_TableName_backup, MissionTableName);
	
	mission_keyArray.Unload();
	mission_data.Unload();

	bool search_result = false;
	int missionTableId = 0;

	for (missionTableId = 0; missionTableId < MissionTextOffsets.size; missionTableId++) {
		if (strncmp(MissionTextOffsets.data[missionTableId].szMissionName, MissionTableName, strlen(MissionTextOffsets.data[missionTableId].szMissionName)) == 0) {
			search_result = true;
			break;
		}
	}

	if (!search_result) {
		printf("CText::LoadMissionText - couldn't find %s", MissionTableName);
		return;
	}

	CFileMgr::SetDir("TEXT");
	switch (FrontEndMenuManager.m_PrefsLanguage) {
	case CMenuManager::LANGUAGE_AMERICAN:
		sprintf(filename, "AMERICAN.GXT");
		break;
	case CMenuManager::LANGUAGE_FRENCH:
		sprintf(filename, "FRENCH.GXT");
		break;
	case CMenuManager::LANGUAGE_GERMAN:
		sprintf(filename, "GERMAN.GXT");
		break;
	case CMenuManager::LANGUAGE_ITALIAN:
		sprintf(filename, "ITALIAN.GXT");
		break;
	case CMenuManager::LANGUAGE_SPANISH:
		sprintf(filename, "SPANISH.GXT");
		break;
#ifdef MORE_LANGUAGES
	case CMenuManager::LANGUAGE_POLISH:
		sprintf(filename, "POLISH.GXT");
		break;
	case CMenuManager::LANGUAGE_RUSSIAN:
		sprintf(filename, "RUSSIAN.GXT");
		break;
	case CMenuManager::LANGUAGE_JAPANESE:
		sprintf(filename, "JAPANESE.GXT");
		break;
	case CMenuManager::LANGUAGE_CHINESE: 
		sprintf(filename, "CHINESE.GXT"); 
		break;
#endif
	}
	CTimer::Suspend();
	int file = CFileMgr::OpenFile(filename, "rb");
	CFileMgr::Seek(file, MissionTextOffsets.data[missionTableId].offset, SEEK_SET);

	char TableCheck[8];
	CFileMgr::Read(file, TableCheck, 8);
	if (strncmp(TableCheck, MissionTableName, 8) != 0)
		printf("CText::LoadMissionText - expected to find %s in the text file", MissionTableName);

	bool tkey_loaded = false, tdat_loaded = false;
	ChunkHeader m_ChunkHeader;
	while (!tkey_loaded || !tdat_loaded) {
		size_t bytes_read = 0;
		ReadChunkHeader(&m_ChunkHeader, file, &bytes_read);
		if (m_ChunkHeader.size != 0) {
			if (strncmp(m_ChunkHeader.magic, "TKEY", 4) == 0) {
				size_t bytes_read = 0;
				mission_keyArray.Load(m_ChunkHeader.size, file, &bytes_read);
				tkey_loaded = true;
			} else if (strncmp(m_ChunkHeader.magic, "TDAT", 4) == 0) {
				size_t bytes_read = 0;
				mission_data.Load(m_ChunkHeader.size, file, &bytes_read);
				tdat_loaded = true;
			} else
				CFileMgr::Seek(file, m_ChunkHeader.size, SEEK_CUR);
		}
	}

	mission_keyArray.Update(mission_data.chars);
	CFileMgr::CloseFile(file);
	CTimer::Resume();
	CFileMgr::SetDir("");
	strcpy(szMissionTableName, MissionTableName);
	bIsMissionTextLoaded = true;
}

const char *
CText::GetMissiontTableName(void)
{
	
	return Mission_TableName_backup;
}

#ifdef VC_CLEO
#include <algorithm>
#include <cwchar>
#include <cctype>
bool
CText::AddKeyValue(const char *key, const wchar *value)
{
	if(!key || !value) return false;

	// 规范化 key（最多 7 字节），并转为大写以与 GXT 约定保持一致

	char keybuf[8];
	memset(keybuf, 0, sizeof(keybuf));
	strncpy(keybuf, key, 7);
	keybuf[7] = '\0';
	for(int i = 0; i < 7 && keybuf[i]; ++i) keybuf[i] = (char)toupper((unsigned char)keybuf[i]);

	// 计算 value 长度（wchar 单元，不含终止符）

	size_t valLen = std::wcslen((wchar_t *)value);

	// 记录老数据基址与大小（用于非 FIX_BUGS 分支更新指针）

	wchar *oldChars = data.chars;
	int oldNumChars = data.numChars;

	// 分配并复制新的 chars 缓冲区（包含终止符）

	wchar *newChars = new wchar[oldNumChars + (int)valLen + 1];
	if(oldChars) { std::memcpy(newChars, oldChars, oldNumChars * sizeof(wchar)); }
std::memcpy(newChars + oldNumChars, value, (valLen + 1) * sizeof(wchar)); // 含终止符
	data.chars = newChars;
	data.numChars = oldNumChars + (int)valLen + 1;

	// 构造新条目

	CKeyEntry newEntry;
	memset(&newEntry, 0, sizeof(newEntry));
#if defined(FIX_BUGS) || defined(FIX_BUGS_64)
	// Search 使用 (uint8*)data + valueOffset —— valueOffset 必须是字节偏移

	newEntry.valueOffset = (uint32)(oldNumChars * sizeof(wchar));
#else
	// 非 FIX_BUGS 分支，entries 中存的是指针（所以直接设为新缓冲区对应指针）

	newEntry.value = &data.chars[oldNumChars];
#endif
	std::memset(newEntry.key, 0, sizeof(newEntry.key));
	std::strncpy(newEntry.key, keybuf, 7);

	// 在 keyArray.entries 中按字典序插入 newEntry（保持排序）

	int oldN = keyArray.numEntries;
	CKeyEntry *oldEntries = keyArray.entries;
	CKeyEntry *newEntries = new CKeyEntry[oldN + 1];

	// 找到插入位置（按 strcmp，已将 key 转为大写）

	int pos = 0;
	while(pos < oldN && std::strncmp(oldEntries[pos].key, newEntry.key, 8) < 0) ++pos;

	// 复制前半部分
	for(int i = 0; i < pos; ++i) newEntries[i] = oldEntries[i];

	// 插入新条目

	newEntries[pos] = newEntry;

	// 复制后半部分
	for(int i = pos; i < oldN; ++i) newEntries[i + 1] = oldEntries[i];

	// 如果是非 FIX_BUGS（entries 存指针），需要修正所有条目的 value 指针指向新 chars：

#if !defined(FIX_BUGS) && !defined(FIX_BUGS_64)
	if(oldEntries) {
		for(int i = 0; i < oldN + 1; ++i) {
			// 只有当原来条目的 value 指针不为 nullptr 时才修正（newEntry 已正确）

			if(i == pos) continue; // 新插入的已经正确
			wchar *oldPtr = nullptr;
			// 原来来自 oldEntries：但我们已把 oldEntries 内容复制到 newEntries

// 先从 oldEntries[i] 获取原始指针（注意越界）
			// 当 i < pos 使用 oldEntries[i]，当 i > pos 使用 oldEntries[i-1]
			int srcIndex = (i < pos) ? i : (i - 1);

			oldPtr = oldEntries[srcIndex].value;
			if(oldPtr) {
				// 计算在旧缓冲区的字单元偏移（以字节差再除以 sizeof(wchar)）

				ptrdiff_t byteOff = (uint8 *)oldPtr - (uint8 *)oldChars;
// 边界检查（防御性）
				if(byteOff >= 0 && byteOff % (intptr_t)sizeof(wchar) == 0) {
					wchar *newPtr = (wchar *)((uint8 *)data.chars + byteOff);
					newEntries[i].value = newPtr;
				} else {
					// 不可信的指针，置为 nullptr 避免访问

					newEntries[i].value = nullptr;
				}
			}
		}
	}
#endif

	// 释放旧 entries、旧 chars

	if(oldEntries) delete[] oldEntries;
	if(oldChars) delete[] oldChars; // oldChars 已被复制到 newChars


	keyArray.entries = newEntries;
	keyArray.numEntries = oldN + 1;

	return true;
}
#endif // VC_CLEO




void
CKeyArray::Load(size_t length, int file, size_t* offset)
{
	char *rawbytes;

	// You can make numEntries size_t if you want to exceed 32-bit boundaries, everything else should be ready.
	numEntries = (int)(length / sizeof(CKeyEntry));
	entries = new CKeyEntry[numEntries];
	rawbytes = (char*)entries;

#ifdef THIS_IS_STUPID
	for (uint32 i = 0; i < length; i++) {
		CFileMgr::Read(file, &rawbytes[i], 1);
		(*offset)++;
	}
#else
	CFileMgr::Read(file, rawbytes, length);
	*offset += length;
#endif
}

void
CKeyArray::Unload(void)
{
	delete[] entries;
	entries = nil;
	numEntries = 0;
}

void
CKeyArray::Update(wchar *chars)
{
#if !defined(FIX_BUGS) && !defined(FIX_BUGS_64)
	int i;
	for(i = 0; i < numEntries; i++)
		entries[i].value = (wchar*)((uint8*)chars + (uintptr)entries[i].value);
#endif
}

CKeyEntry*
CKeyArray::BinarySearch(const char *key, CKeyEntry *entries, int16 low, int16 high)
{
	int mid;
	int diff;

	if(low > high)
		return nil;

	mid = (low + high)/2;
	diff = strcmp(key, entries[mid].key);
	if(diff == 0)
		return &entries[mid];
	if(diff < 0)
		return BinarySearch(key, entries, low, mid-1);
	if(diff > 0)
		return BinarySearch(key, entries, mid+1, high);
	return nil;
}

wchar*
#if defined (FIX_BUGS) || defined(FIX_BUGS_64)
CKeyArray::Search(const char *key, wchar *data, uint8 *result)
#else
CKeyArray::Search(const char *key, uint8 *result)
#endif
{
	CKeyEntry *found;
	char errstr[25];
	int i;

#if defined (FIX_BUGS) || defined(FIX_BUGS_64)
	found = BinarySearch(key, entries, 0, numEntries-1);
	if (found) {
		*result = true;
		return (wchar*)((uint8*)data + found->valueOffset);
	}
#else
	found = BinarySearch(key, entries, 0, numEntries-1);
	if (found) {
		*result = true;
		return found->value;
	}
#endif
	*result = false;
#ifdef MASTER
	sprintf(errstr, "");
#else
	sprintf(errstr, "%s missing", key);
#endif // MASTER
	for(i = 0; i < 25; i++)
		WideErrorString[i] = errstr[i];
	return WideErrorString;
}

void
CData::Load(size_t length, int file, size_t * offset)
{
	char *rawbytes;

	// You can make numChars size_t if you want to exceed 32-bit boundaries, everything else should be ready.
	numChars = (int)(length / sizeof(wchar));
	chars = new wchar[numChars];
	rawbytes = (char*)chars;

#ifdef THIS_IS_STUPID
	for(uint32 i = 0; i < length; i++){
		CFileMgr::Read(file, &rawbytes[i], 1);
		(*offset)++;
	}
#else
	CFileMgr::Read(file, rawbytes, length);
	*offset += length;
#endif
}

void
CData::Unload(void)
{
	delete[] chars;
	chars = nil;
	numChars = 0;
}

void
CMissionTextOffsets::Load(size_t table_size, int file, size_t *offset, int)
{
#ifdef THIS_IS_STUPID
	size_t num_of_entries = table_size / sizeof(CMissionTextOffsets::Entry);
	for (size_t mi = 0; mi < num_of_entries; mi++) {
		for (uint32 i = 0; i < sizeof(data[mi].szMissionName); i++) {
			CFileMgr::Read(file, &data[i].szMissionName[i], 1);
			(*offset)++;
		}
		char* _buf = (char*)&data[mi].offset;
		for (uint32 i = 0; i < sizeof(data[mi].offset); i++) {
			CFileMgr::Read(file, &_buf[i], 1);
			(*offset)++;
		}
	}
	size = (uint16)num_of_entries;
#else
	// not exact VC code but smaller and better :P

	// You can make this size_t if you want to exceed 32-bit boundaries, everything else should be ready.
	size = (uint16) (table_size / sizeof(CMissionTextOffsets::Entry));
	CFileMgr::Read(file, (char*)data, sizeof(CMissionTextOffsets::Entry) * size);
	*offset += sizeof(CMissionTextOffsets::Entry) * size;
#endif
}

char*
UnicodeToAscii(wchar *src)
{
	static char aStr[256];
	int len;
	for(len = 0; *src != '\0' && len < 256-1; len++, src++)
#ifdef MORE_LANGUAGES
		if(*src < 128 || ((CGame::russianGame || CGame::japaneseGame) && *src < 256))
#else
		if(*src < 128)
#endif
			aStr[len] = *src;
		// convert to CP1252
		else if(*src <= 131)
			aStr[len] = *src + 64;
		else if (*src <= 141)
			aStr[len] = *src + 66;
		else if (*src <= 145)
			aStr[len] = *src + 68;
		else if (*src <= 149)
			aStr[len] = *src + 71;
		else if (*src <= 154)
			aStr[len] = *src + 73;
		else if (*src <= 164)
			aStr[len] = *src + 75;
		else if (*src <= 168)
			aStr[len] = *src + 77;
		else if (*src <= 204)
			aStr[len] = *src + 80;
		else switch (*src) {
		case 205: aStr[len] = 209; break;
		case 206: aStr[len] = 241; break;
		case 207: aStr[len] = 191; break;
		default: aStr[len] = '#'; break;
		}
	aStr[len] = '\0';
	return aStr;
}

char*
UnicodeToAsciiForSaveLoad(wchar *src)
{
	static char aStr[256];
	int len;
	for(len = 0; *src != '\0' && len < 256; len++, src++)
		if(*src < 256)
			aStr[len] = *src;
		else
			aStr[len] = '#';
	aStr[len] = '\0';
	return aStr;
}

char*
UnicodeToAsciiForMemoryCard(wchar *src)
{
	static char aStr[256];
	int len;
	for(len = 0; *src != '\0' && len < 256; len++, src++)
		if(*src < 256)
			aStr[len] = *src;
		else
			aStr[len] = '#';
	aStr[len] = '\0';
	return aStr;
}

void
TextCopy(wchar *dst, const wchar *src)
{
	while((*dst++ = *src++) != '\0');
}
