      #pragma once
#include "Font.h"
#include "PedType.h"
#include "Text.h"
#include "Sprite2d.h"
#include <vector>
#include <memory>
//#include <CustomScript.h>

class CEntity;
class CBuilding;
class CPhysical;
class CVehicle;
class CPed;
class CObject;
class CPlayerInfo;

class CRunningScript;

extern int32 ScriptParams[32];

void FlushLog();
#define script_assert(_Expression) FlushLog(); assert(_Expression);

#define PICKUP_PLACEMENT_OFFSET (0.5f)
#define PED_FIND_Z_OFFSET (5.0f)
#define COP_PED_FIND_Z_OFFSET (10.0f)

#define UPSIDEDOWN_UP_THRESHOLD (-0.97f)
#define UPSIDEDOWN_MOVE_SPEED_THRESHOLD (0.01f)
#define UPSIDEDOWN_TURN_SPEED_THRESHOLD (0.02f)
#define UPSIDEDOWN_TIMER_THRESHOLD (1000)

#define SPHERE_MARKER_R (252)
#define SPHERE_MARKER_G (138)
#define SPHERE_MARKER_B (242)
#define SPHERE_MARKER_A (228)
#define SPHERE_MARKER_PULSE_PERIOD 2048
#define SPHERE_MARKER_PULSE_FRACTION 0.1f

#ifdef USE_PRECISE_MEASUREMENT_CONVERTION
#define MILES_IN_METER (0.000621371192f)
#define METERS_IN_FOOT (0.3048f)
#define FEET_IN_METER (3.28084f)
#else
#define MILES_IN_METER (1 / 1670.f)
#define METERS_IN_FOOT (0.3f)
#define FEET_IN_METER (3.33f)
#endif

#define KEY_LENGTH_IN_SCRIPT (8)

//#define GTA_SCRIPT_COLLECTIVE

struct intro_script_rectangle 
{
	bool m_bIsUsed;
	bool m_bBeforeFade;
	int16 m_nTextureId;
	CRect m_sRect;
	CRGBA m_sColor;
	
	intro_script_rectangle() { }
	~intro_script_rectangle() { }
};

VALIDATE_SIZE(intro_script_rectangle, 0x18);

enum {
	SCRIPT_TEXT_MAX_LENGTH = 100
};

struct intro_text_line 
{
	float m_fScaleX;
	float m_fScaleY;
	CRGBA m_sColor;
	bool m_bJustify;
	bool m_bCentered;
	bool m_bBackground;
	bool m_bBackgroundOnly;
	float m_fWrapX;
	float m_fCenterSize;
	CRGBA m_sBackgroundColor;
	bool m_bTextProportional;
	bool m_bTextBeforeFade;
	bool m_bRightJustify;
	int32 m_nFont;
	float m_fAtX;
	float m_fAtY;
	wchar m_Text[SCRIPT_TEXT_MAX_LENGTH];

	intro_text_line() { }
	~intro_text_line() { }
	
	void Reset()
	{
		m_fScaleX = 0.48f;
		m_fScaleY = 1.12f;
		m_sColor = CRGBA(225, 225, 225, 255);
		m_bJustify = false;
		m_bRightJustify = false;
		m_bCentered = false;
		m_bBackground = false;
		m_bBackgroundOnly = false;
		m_fWrapX = 182.0f;
		m_fCenterSize = DEFAULT_SCREEN_WIDTH;
		m_sBackgroundColor = CRGBA(128, 128, 128, 128);
		m_bTextProportional = true;
		m_bTextBeforeFade = false;
		m_nFont = FONT_STANDARD;
		m_fAtX = 0.0f;
		m_fAtY = 0.0f;
		memset(&m_Text, 0, sizeof(m_Text));
	}
};

VALIDATE_SIZE(intro_text_line, 0x414);

struct script_sphere_struct
{
	bool m_bInUse;
	uint16 m_Index;
	uint32 m_Id;
	CVector m_vecCenter;
	float m_fRadius;
	
	script_sphere_struct() { }
};

struct CStoredLine
{
	CVector vecInf;
	CVector vecSup;
	uint32 color1;
	uint32 color2;
};

enum {
	CLEANUP_UNUSED = 0,
	CLEANUP_CAR,
	CLEANUP_CHAR,
	CLEANUP_OBJECT
};

struct cleanup_entity_struct
{
	uint8 type;
	int32 id;
};

enum {
	MAX_CLEANUP = 50,
	MAX_UPSIDEDOWN_CAR_CHECKS = 6,
	MAX_STUCK_CAR_CHECKS = 16
};

class CMissionCleanup
{
public:
	cleanup_entity_struct m_sEntities[MAX_CLEANUP];
	uint8 m_nCount;

	CMissionCleanup();

	void Init();
	cleanup_entity_struct* FindFree();
	void AddEntityToList(int32, uint8);
	void RemoveEntityFromList(int32, uint8);
	void Process();
	void CheckIfCollisionHasLoadedForMissionObjects();
};

struct upsidedown_car_data
{
	int32 m_nVehicleIndex;
	uint32 m_nUpsideDownTimer;
};

class CUpsideDownCarCheck
{
	upsidedown_car_data m_sCars[MAX_UPSIDEDOWN_CAR_CHECKS];

public:
	void Init();
	bool IsCarUpsideDown(int32);
	bool IsCarUpsideDown(CVehicle*);
	void UpdateTimers();
	bool AreAnyCarsUpsideDown();
	void AddCarToCheck(int32);
	void RemoveCarFromCheck(int32);
	bool HasCarBeenUpsideDownForAWhile(int32);
};

struct stuck_car_data
{
	int32 m_nVehicleIndex;
	CVector m_vecPos;
	int32 m_nLastCheck;
	float m_fRadius;
	uint32 m_nStuckTime;
	bool m_bStuck;

	stuck_car_data() { }
	void Reset();
};

class CStuckCarCheck
{
	stuck_car_data m_sCars[MAX_STUCK_CAR_CHECKS];

public:
	void Init();
	void Process();
	void AddCarToCheck(int32, float, uint32);
	void RemoveCarFromCheck(int32);
	bool HasCarBeenStuckForAWhile(int32);
};

enum {
	ARGUMENT_END = 0,
	ARGUMENT_INT32,
	
	ARGUMENT_GLOBALVAR,
	ARGUMENT_LOCALVAR,
	ARGUMENT_INT8,
	ARGUMENT_INT16,
	ARGUMENT_FLOAT,
#ifdef VC_CLEO
	PARAM_TYPE_STRING = 14
#endif // VC_CLEO
	

};

struct tCollectiveData
{
	int32 colIndex;
	int32 pedIndex;
};

enum {
	USED_OBJECT_NAME_LENGTH = 24
};

struct tUsedObject
{
	char name[USED_OBJECT_NAME_LENGTH];
	int32 index;
};

struct tBuildingSwap
{
	CBuilding* m_pBuilding;
	int32 m_nNewModel;
	int32 m_nOldModel;
};


enum {
	MAX_STACK_DEPTH = 6,
	NUM_LOCAL_VARS = 16,
	NUM_TIMERS = 2
};
#ifdef VC_CLEO
enum eScriptType : unsigned short { SCRIPT_TYPE_DEFAULT = 0, SCRIPT_TYPE_CUSTOM = 1 };
#endif // VC_CLEO

class CRunningScript
{
	enum {
		ANDOR_NONE = 0,
		ANDS_1 = 1,
		ANDS_2,
		ANDS_3,
		ANDS_4,
		ANDS_5,
		ANDS_6,
		ANDS_7,
		ANDS_8,
		ORS_1 = 21,
		ORS_2,
		ORS_3,
		ORS_4,
		ORS_5,
		ORS_6,
		ORS_7,
		ORS_8
	};

public:
	CRunningScript *next; // 下一个脚本
	CRunningScript *prev;   // 上一个脚本
	char m_abScriptName[8]; // 脚本名称
	uint32 m_nIp;// 指令指针
	uint32 m_anStack[MAX_STACK_DEPTH];//栈
	uint16 m_nStackPointer;//栈指针
	int32 m_anLocalVariables[NUM_LOCAL_VARS + NUM_TIMERS];//本地变量
	bool m_bIsActive;//是否激活
	bool m_bCondResult;//条件结果
	bool m_bIsMissionScript;//是否任务脚本
	bool m_bSkipWakeTime;//跳过唤醒时间
	uint32 m_nWakeTime;//唤醒时间
	uint16 m_nAndOrState;//与或状态 bool?
	bool m_bNotFlag;//空标记
	bool m_bDeatharrestEnabled;//死亡逮捕已启用
	bool m_bDeatharrestExecuted;//死亡逮捕已执行
	bool m_bMissionFlag;//任务标记
#ifdef VC_CLEO
	eScriptType sctype = SCRIPT_TYPE_DEFAULT;
	uint32 base_nIp; // 指令指针
	char m_bScriptFileName[64];
#endif // VC_CLEO


public:

	void SetIP(uint32 ip) { m_nIp = ip; }


	//void SetIP(uint32 ip) { m_nIp = ip; }
	CRunningScript* GetNext() const { return next; }

	void Save(uint8 *&buf); // 保存脚本状态
	void Load(uint8 *&buf); // 加载脚本状态

	// 更新计时器
	void UpdateTimers(float timeStep) {
		m_anLocalVariables[NUM_LOCAL_VARS] += timeStep;
		m_anLocalVariables[NUM_LOCAL_VARS + 1] += timeStep;
	}

	//初始化
	void Init();
	// 处理脚本
	void Process();

	// 从列表中移除脚本
	void RemoveScriptFromList(CRunningScript**);
	// 添加脚本到列表
	void AddScriptToList(CRunningScript**);

	static const uint32 nSaveStructSize; // 保存结构大小
	// 收集参数
	void CollectParameters(uint32 *, int16); 
	// 收集下一个参数而不增加PC
	int32 CollectNextParameterWithoutIncreasingPC(uint32); 
	 // 获取脚本变量的指针
	int32 *GetPointerToScriptVariable(uint32 *, int16);   
	// 存储参数
	void StoreParameters(uint32 *, int16);                 

	// 处理一个命令
	int8 ProcessOneCommand(); 
	// 执行死亡逮捕检查
	void DoDeatharrestCheck(); 
	 // 更新比较标志
	void UpdateCompareFlag(bool);
	// 获取pad状态
	int16 GetPadState(uint16, uint16); 

	int8 ProcessCommands0To99(int32); // 处理命令0到99
	int8 ProcessCommands100To199(int32);
	int8 ProcessCommands200To299(int32);
	int8 ProcessCommands300To399(int32);
	int8 ProcessCommands400To499(int32);
	int8 ProcessCommands500To599(int32);
	int8 ProcessCommands600To699(int32);
	int8 ProcessCommands700To799(int32);
	int8 ProcessCommands800To899(int32);
	int8 ProcessCommands900To999(int32);
	int8 ProcessCommands1000To1099(int32);
	int8 ProcessCommands1100To1199(int32);
	int8 ProcessCommands1200To1299(int32);
	int8 ProcessCommands1300To1399(int32);
	int8 ProcessCommands1400To1499(int32);
	//cleo
	int8 ProcessCleoScripts(int32);

	// 定位玩家
	void LocatePlayerCommand(int32, uint32 *); 
	// 定位玩家角色
	void LocatePlayerCharCommand(int32, uint32 *); 
	// 定位玩家车辆
	void LocatePlayerCarCommand(int32, uint32 *);  
	 // 定位角色
	void LocateCharCommand(int32, uint32 *);      
	// 定位角色角色
	void LocateCharCharCommand(int32, uint32 *);   
	// 定位角色车辆
	void LocateCharCarCommand(int32, uint32 *);    
	// 定位角色对象
	void LocateCharObjectCommand(int32, uint32 *); 
	// 定位车辆
	void LocateCarCommand(int32, uint32 *);        
	// 定位狙击子弹
	void LocateSniperBulletCommand(int32, uint32 *); 
	// 玩家在区域检查命令
	void PlayerInAreaCheckCommand(int32, uint32 *);  
	// 玩家在角区域检查命令
	void PlayerInAngledAreaCheckCommand(int32, uint32 *); 
	 // 角色在区域检查命令
	void CharInAreaCheckCommand(int32, uint32 *);   
	// 车辆在区域检查命令
	void CarInAreaCheckCommand(int32, uint32 *);     
	 // 定位对象
	void LocateObjectCommand(int32, uint32 *);     
	  // 对象在区域检查命令
	void ObjectInAreaCheckCommand(int32, uint32 *);     

#ifdef GTA_SCRIPT_COLLECTIVE
	void LocateCollectiveCommand(int32, uint32*);
	void LocateCollectiveCharCommand(int32, uint32*);
	void LocateCollectiveCarCommand(int32, uint32*);
	void LocateCollectivePlayerCommand(int32, uint32*);
	void CollectiveInAreaCheckCommand(int32, uint32*);
#endif

#ifdef MISSION_REPLAY
	// 是否允许任务重播
	bool CanAllowMissionReplay();
#endif

#ifdef USE_ADVANCED_SCRIPT_DEBUG_OUTPUT
	int CollectParameterForDebug(char* buf, bool& var);
	void GetStoredParameterForDebug(char* buf);
	void LogOnStartProcessing();
	void LogBeforeProcessingCommand(int32 command);
	void LogAfterProcessingCommand(int32 command);

	static char commandInfo[];
	static uint32 storedIp;

#endif

	// 限制圆上的角度
	float LimitAngleOnCircle(float angle) { return angle < 0.0f ? angle + 360.0f : angle; }

	// 检查是否是有效的随机车辆
	bool ThisIsAValidRandomCop(uint32 mi, int cop, int swat, int fbi, int army, int miami);
	// 检查是否是有效的随机角色
	bool ThisIsAValidRandomPed(uint32 pedtype, int civ, int gang, int criminal);

	// 检查损坏的武器类型
	bool CheckDamagedWeaponType(int32 actual, int32 type);
	
};


enum {
	VAR_LOCAL = 1,
	VAR_GLOBAL = 2,
};

enum {
#ifdef PS2
	SIZE_MAIN_SCRIPT = 205512,
#else

	SIZE_MAIN_SCRIPT = 225512, // 原版

#endif

	SIZE_MISSION_SCRIPT = 35000, // 大任务包测试35000->400000
                                // VC_BMP
	
	SIZE_SCRIPT_SPACE = SIZE_MAIN_SCRIPT + SIZE_MISSION_SCRIPT
};


enum {

	MAX_NUM_SCRIPTS = 128,


	MAX_NUM_INTRO_TEXT_LINES = 48,
	MAX_NUM_INTRO_RECTANGLES = 16,
	MAX_NUM_SCRIPT_SRPITES = 16,
	MAX_NUM_SCRIPT_SPHERES = 16,
	MAX_NUM_USED_OBJECTS = 220,
	MAX_NUM_MISSION_SCRIPTS = 120,
	
	MAX_NUM_BUILDING_SWAPS = 25,
	MAX_NUM_INVISIBILITY_SETTINGS = 20,
	MAX_NUM_STORED_LINES = 1024
};

class CTheScripts
{
public:
#ifndef VC_CLEO
	static uint8 ScriptSpace[SIZE_SCRIPT_SPACE]; // 脚本空间
	#else
	static std::vector<uint8> ScriptSpace; // CLEO脚本列表
#endif // VC_CLEO

	
	static CRunningScript ScriptsArray[MAX_NUM_SCRIPTS]; // 脚本数组
	static intro_text_line IntroTextLines[MAX_NUM_INTRO_TEXT_LINES]; // 介绍文本行
	static intro_script_rectangle IntroRectangles[MAX_NUM_INTRO_RECTANGLES]; // 介绍脚本矩形
	static CSprite2d ScriptSprites[MAX_NUM_SCRIPT_SRPITES];                  // 脚本2d精灵
	static script_sphere_struct ScriptSphereArray[MAX_NUM_SCRIPT_SPHERES];   // 脚本球体数组
	static tUsedObject UsedObjectArray[MAX_NUM_USED_OBJECTS];                // 使用的对象数组
	static int32 MultiScriptArray[MAX_NUM_MISSION_SCRIPTS];                  // 多脚本数组
	static tBuildingSwap BuildingSwapArray[MAX_NUM_BUILDING_SWAPS];          // 建筑交换数组
	static CEntity *InvisibilitySettingArray[MAX_NUM_INVISIBILITY_SETTINGS]; // 隐形设置数组
	static CStoredLine aStoredLines[MAX_NUM_STORED_LINES];                   // 存储的线条数组
	static bool DbgFlag;                                                     // 调试标志
	static uint32 OnAMissionFlag;                                            // 正在执行任务标记
	static CMissionCleanup MissionCleanUp;                                   // 任务清理对象
	static CStuckCarCheck StuckCars;                                         // 车辆卡住检查对象
	static CUpsideDownCarCheck UpsideDownCars;                               // 车辆翻车检查对象
	static int32 StoreVehicleIndex;                                          // 存储的车辆索引
	static bool StoreVehicleWasRandom;                                       // 存储的车辆是否为随机的
	static CRunningScript *pIdleScripts;                                     // 空闲脚本指针
	static CRunningScript *pActiveScripts;                                   // 活动脚本指针
	static int32 NextFreeCollectiveIndex;                                    // 下一个可用的集合索引
	static int32 LastRandomPedId;                                            // 最后一个随机行人ID
	static uint16 NumberOfUsedObjects;                                       // 使用中的对象数量
	static bool bAlreadyRunningAMissionScript;                               // 是否已经在运行一个任务脚本
	static bool bUsingAMultiScriptFile;                                      // 是否使用多脚本文件
	static uint16 NumberOfMissionScripts;                                    // 任务脚本数量
	static uint32 LargestMissionScriptSize;                                  // 最大的任务脚本大小
	static uint32 MainScriptSize;                                            // 主脚本大小
	static uint8 FailCurrentMission;                                         // 失败当前任务
	static uint16 NumScriptDebugLines;                                       // 脚本调试行数
	static uint16 NumberOfIntroRectanglesThisFrame;                          // 本帧介绍矩形的数量
	static uint16 NumberOfIntroTextLinesThisFrame;                           // 本帧介绍文本行数
	static uint8 UseTextCommands;                                            // 使用文本命令
	static uint16 CommandsExecuted;                                          // 已执行的命令数
	static uint16 ScriptsUpdated;                                            // 已更新的脚本数
	static uint32 LastMissionPassedTime;                                     // 上一个任务完成时间
	static uint16 NumberOfExclusiveMissionScripts;                           // 独占任务脚本数量
#if (defined GTA_PC && !defined GTAVC_JP_PATCH || defined GTA_XBOX || defined SUPPORT_XBOX_SCRIPT || defined GTA_MOBILE || defined SUPPORT_MOBILE_SCRIPT)
#define CARDS_IN_SUIT (13)
#define NUM_SUITS (4)
#define MAX_DECKS (6)
#define CARDS_IN_DECK (CARDS_IN_SUIT * NUM_SUITS)
#define CARDS_IN_STACK (CARDS_IN_DECK * MAX_DECKS)
	static int16 CardStack[CARDS_IN_STACK];
	static int16 CardStackPosition;
#endif
	static bool bPlayerIsInTheStatium; // 玩家是否在体育场
	static uint8 RiotIntensity;        // 暴动强度
	static bool bPlayerHasMetDebbieHarry; // 玩家是否见过黛比·哈里

	static void Init();
	static void Process(); // 处理脚本

	static void LoadCustomScripts();
	static void LoadCustomScriptsFxt();

	 // 启动测试脚本
	static CRunningScript *StartTestScript();
	// 玩家是否在任务中
	static bool IsPlayerOnAMission();       
	// 清除任务实体的空间
	static void ClearSpaceForMissionEntity(const CVector &, CEntity *); 

	// 撤销建筑交换
	static void UndoBuildingSwaps(); 
	// 撤销实体隐形设置
	static void UndoEntityInvisibilitySettings(); 

	// 脚本调试线条
	static void ScriptDebugLine3D(float x1, float y1, float z1, float x2, float y2, float z2, uint32 col, uint32 col2);
	// 渲染脚本调试线条
	static void RenderTheScriptDebugLines(); 

	 // 保存所有脚本
	static void SaveAllScripts(uint8 *, uint32 *);
	// 加载所有脚本
	static void LoadAllScripts(uint8 *, uint32);  

	static bool IsDebugOn() { return DbgFlag; };
	static void InvertDebugFlag() { DbgFlag = !DbgFlag; }

	// 获取脚本变量的指针
	static int32* GetPointerToScriptVariable(int32 offset) { assert(offset >= 8 && offset < CTheScripts::GetSizeOfVariableSpace()); return (int32*)&ScriptSpace[offset]; }

	// 从脚本中读取4字节数据
	static int32 Read4BytesFromScript(uint32* pIp) {
		int32 retval = ScriptSpace[*pIp + 3] << 24 | ScriptSpace[*pIp + 2] << 16 | ScriptSpace[*pIp + 1] << 8 | ScriptSpace[*pIp];
		*pIp += 4;
		return retval;
	}
	//从脚本中读取2字节数据
	static int16 Read2BytesFromScript(uint32* pIp) {
		int16 retval = ScriptSpace[*pIp + 1] << 8 | ScriptSpace[*pIp];
		*pIp += 2;
		return retval;
	}
	// 从脚本中读取1字节数据
	static int8 Read1ByteFromScript(uint32* pIp) {
		int8 retval = ScriptSpace[*pIp];
		*pIp += 1;
		return retval;
	}
	// 从脚本中读取浮点数
	static float ReadFloatFromScript(uint32* pIp) {
		return Read2BytesFromScript(pIp) / 16.0f;
	}
	// 从脚本中读取文本标签
	static void ReadTextLabelFromScript(uint32* pIp, char* buf) {
		strncpy(buf, (const char*)&CTheScripts::ScriptSpace[*pIp], KEY_LENGTH_IN_SCRIPT);
	}
	// 从脚本中通过键获取文本
	static wchar* GetTextByKeyFromScript(uint32* pIp) {
		wchar* text = TheText.Get((const char*)&CTheScripts::ScriptSpace[*pIp]);
		*pIp += KEY_LENGTH_IN_SCRIPT;
		return text;
	}
	// 获取变量空间的大小
	static int32 GetSizeOfVariableSpace()
	{
		uint32 tmp = 3;
		return Read4BytesFromScript(&tmp);
	}

	// 启动新脚本
	static CRunningScript* StartNewScript(uint32);

	// 停止脚本
	static void CleanUpThisVehicle(CVehicle*);
	// 停止这个角色
	static void CleanUpThisPed(CPed *);
	// 停止这个对象
	static void CleanUpThisObject(CObject *); 

	// 角色是否停止
	static bool IsPedStopped(CPed *); 
	 // 玩家是否停止
	static bool IsPlayerStopped(CPlayerInfo *);
	// 车辆是否停止
	static bool IsVehicleStopped(CVehicle *);  

	// 打印列表大小
	static void PrintListSizes(); 
	// 从脚本中读取对象名称
	static void ReadObjectNamesFromScript(); 
	 // 更新对象索引
	static void UpdateObjectIndices();      
	// 从脚本中读取多脚本文件偏移
	static void ReadMultiScriptFileOffsetsFromScript(); 
	// 绘制脚本球体
	static void DrawScriptSpheres();                    
	// 高亮显示重要区域
	static void HighlightImportantArea(uint32, float, float, float, float, float);
	// 高亮显示重要的角区域
	static void HighlightImportantAngledArea(uint32, float, float, float, float, float, float, float, float, float);
	// 绘制调试方块
	static void DrawDebugSquare(float, float, float, float);
	// 绘制调试角方块
	static void DrawDebugAngledSquare(float, float, float, float, float, float, float, float);
	// 绘制调试立方体
	static void DrawDebugCube(float, float, float, float, float, float);
	// 绘制调试角立方体
	static void DrawDebugAngledCube(float, float, float, float, float, float, float, float, float, float);

	// 添加到隐形交换数组
	static void AddToInvisibilitySwapArray(CEntity*, bool);
	// 添加到建筑交换数组
	static void AddToBuildingSwapArray(CBuilding*, int32, int32);

	// 获取实际的脚本球体索引
	static int32 GetActualScriptSphereIndex(int32 index); 
	// 添加脚本球体
	static int32 AddScriptSphere(int32 id, CVector pos, float radius); 
	 // 获取新的唯一脚本球体索引
	static int32 GetNewUniqueScriptSphereIndex(int32 index);          
	// 移除脚本球体
	static void RemoveScriptSphere(int32 index);     
	 // 移除脚本纹理字典
	static void RemoveScriptTextureDictionary();                      
public:
	 // 移除这个角色
	static void RemoveThisPed(CPed *pPed);

	// 获取上一个任务完成时间
	static uint32& GetLastMissionPassedTime() { return LastMissionPassedTime; }
#ifdef MISSION_SWITCHER
	// 切换到任务
	static void SwitchToMission(int32 mission);
#endif

#ifdef GTA_SCRIPT_COLLECTIVE
	static void AdvanceCollectiveIndex()
	{
		if (NextFreeCollectiveIndex == INT32_MAX)
			NextFreeCollectiveIndex = 0;
		else
			NextFreeCollectiveIndex++;
	}

	static int AddPedsInVehicleToCollective(int);
	static int AddPedsInAreaToCollective(float, float, float, float);
	static int FindFreeSlotInCollectiveArray();
	static void SetObjectiveForAllPedsInCollective(int, eObjective, int16, int16);
	static void SetObjectiveForAllPedsInCollective(int, eObjective, CVector, float);
	static void SetObjectiveForAllPedsInCollective(int, eObjective, CVector);
	static void SetObjectiveForAllPedsInCollective(int, eObjective, void*);
	static void SetObjectiveForAllPedsInCollective(int, eObjective);
#endif

#ifdef USE_MISSION_REPLAY_OVERRIDE_FOR_NON_MOBILE_SCRIPT
	static bool MissionSupportsMissionReplay(int index)
	{
		return index >= 3 && index <= 35 || index >= 51 && index <= 65 || index >= 67 && index <= 74 || index >= 83 && index <= 87;
	}
#endif

#ifdef USE_DEBUG_SCRIPT_LOADER
	static int ScriptToLoad;
	static int OpenScript();
#endif

#ifdef USE_ADVANCED_SCRIPT_DEBUG_OUTPUT
	static void LogAfterScriptInitializing();
	static void LogBeforeScriptProcessing();
	static void LogAfterScriptProcessing();
#endif


};

#ifdef MISSION_REPLAY
extern int AllowMissionReplay; // 是否允许任务重播
extern uint32 WaitForMissionActivate; // 等待任务激活
extern uint32 WaitForSave;            // 等待保存
extern uint32 MissionStartTime;       // 任务开始时间
extern int missionRetryScriptIndex;   // 任务重试脚本索引
extern bool doingMissionRetry;        // 是否正在进行任务重试
extern bool gbTryingPorn4Again;       // 是否正在尝试再次进行色情4任务
extern int IsInAmmunation;            // 是否在军火商
extern int MissionSkipLevel;          // 任务跳过等级

#ifdef USE_MISSION_REPLAY_OVERRIDE_FOR_NON_MOBILE_SCRIPT
extern bool UsingMobileScript; // 是否使用移动脚本
extern bool AlreadySavedGame;  // 是否已经保存游戏
#endif

// 添加额外的死亡延迟
 uint32 AddExtraDeathDelay(); 
// 重试任务
 void RetryMission(int, int unk = 0); 

enum {
	MISSION_RETRY_TYPE_SUGGEST_TO_PLAYER = 0,
	MISSION_RETRY_TYPE_1,
	MISSION_RETRY_TYPE_BEGIN_RESTARTING
};

enum {
	MISSION_RETRY_STAGE_NORMAL = 0,
	MISSION_RETRY_STAGE_WAIT_FOR_SCRIPT_TO_TERMINATE,
	MISSION_RETRY_STAGE_START_PROCESSING,
	MISSION_RETRY_STAGE_WAIT_FOR_DELAY,
	MISSION_RETRY_STAGE_WAIT_FOR_MENU,
	MISSION_RETRY_STAGE_WAIT_FOR_USER,
	MISSION_RETRY_STAGE_START_RESTARTING,
	MISSION_RETRY_STAGE_WAIT_FOR_TIMER_AFTER_RESTART,
};
#endif
