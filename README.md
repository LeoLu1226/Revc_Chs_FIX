
# Revc的中文化支持和cleo支持

汉化作者：Ova1122（gamefiles 中文文本与贴图来自无名汉化组）

中文的已经好了 gamefiles文件夹里面已经包含无名的文本和贴图了 自己复制到游戏目录  
cleo下次更新会加入  

CLEO已经支持，部分涉及读写内存的我没实现，因为和原版不兼容了，有需要的可以根据cleo的源码自行添加 其他的应该都支持了

## 汉化特性（CHSFont）

* 动态字库：任意中文 + 扩展平面（生僻字 U+30EDE「𰻞」、Emoji）都能显示，永不缺字
* 三种文本渲染模式（[Fonts] `TextRenderer=1/2/3`）：静态 CHINESE.TXD / GDI 动态字库 / DirectWrite（默认，最完善）
* 彩色表情符号（COLR/CPAL）与 emoji 右剪修复
* 可变字重：`NormalWeight` / `SlantWeight` / `RareWeight`（100–900，可自动保存回写 reVC.ini）
* GXT 热重载：修改任意语言文本约 1 秒自动生效（开发调试用）

## 文本渲染模式与字体配置（GDI / DirectWrite）

所有字体相关配置都在游戏目录的 `reVC.ini` 的 `[Fonts]` 段，**改完需重启游戏生效**（启动时读取）。

### 三种渲染模式

| `TextRenderer` | 模式 | 说明 |
|---|---|---|
| `3` | **DirectWrite（默认，推荐）** | 动态字库。任意中文 + 生僻字（𰻞 U+30EDE 等扩展平面）+ 彩色 Emoji（COLR/CPAL）+ 可变字重，全部支持 |
| `2` | **GDI** | GDI `GetGlyphOutlineW` 动态字库，兼容老机器；不支持彩色 Emoji 与可变字重轴 |
| `1` | **TXD（最老）** | 静态 `MODELS\CHINESE.TXD` + `Chinese.dat` 贴图字库（即 gamefiles/chinese_text_file 里那套），无动态能力 |

也兼容旧写法的字符串值（`TXD` / `GDI`），写错或缺失时自动回退到 DirectWrite。

> 以下示例中的 `<…>` 均为占位符，请替换为你**本机已安装**或**有权分发**的字体。本仓库不捆绑任何字体文件；只额外列了 Windows 自带字体作为"开箱可用"的参考。

### DirectWrite 模式（推荐配置示例）

```ini
[Fonts]
TextRenderer=3
NormalFonts=<主字体名>          ; 系统已安装的字体名，或 models 目录下的 .ttf（如微软雅黑填 msyh）
NormalBold=1                    ; 1 = 默认用粗体字重（700）
SlantFontFile=models\<斜体字体>.ttf   ; 斜体（意大利体）字体；可不配，缺省用主字体的伪斜体
SlantBold=0
GlyphHeight=56                  ; 字格高度（像素），越大笔画越细
RareFontFile=<主字体文件>,<补充字体…>  ; 兜底字体链（见下）
```

- **`RareFontFile` 兜底链**：逗号分隔多个字体文件，从左到右依次尝试。主字体**缺字形的任何码位**（生僻字、Emoji、韩文、BMP 缺字）都会按链查找，第一个能画出该字的字体生效——所以"永不缺字"靠的就是它。按需补充 Windows **自带**字体即可：
  - 彩色 Emoji：`C:\Windows\Fonts\seguiemj.ttf`（COLR/CPAL 彩色，Win8.1+ 自带）
  - 韩文：`C:\Windows\Fonts\malgun.ttf`
  - 生僻字（如 𰻞）：`C:\Windows\Fonts\SimsunExtG.ttf`
  - 主字体缺的简中字形：`C:\Windows\Fonts\msyh.ttc`（微软雅黑）
- **可变字重**：`NormalWeight=100~900`（对应 NormalFonts 的粗细，默认 400），`SlantWeight`、`RareWeight` 同理；`NormalBold=1` 时默认字重为 700。这些键会自动写回 `reVC.ini`（不需要手动补）。

### GDI 模式

```ini
[Fonts]
TextRenderer=2
NormalFonts=<主字体名>
SlantFontFile=models\<斜体字体>.ttf
GlyphHeight=56
RareFontFile=<主字体文件>,<补充字体…>
```

GDI 模式也能显示全部码位（超出 BMP 的走 stb_truetype 兜底链），只是没有彩色 Emoji 和字重轴。

### TXD 模式

```ini
[Fonts]
TextRenderer=1
```

需要把 `gamefiles/chinese_text_file/` 下的 `MODELS\CHINESE.TXD` 与 `Chinese.dat` 放进游戏目录对应位置（其余两种模式不需要它们）。

### GXT 热重载（开发调试）

改完 GXT 文本不用重启：游戏运行中保存 `TEXT\xx.GXT`（xx = 当前语言），约 1 秒后自动生效；切换语言会自动跟随新的语言文件；GXT 损坏时自动回滚不崩溃。仅影响文本，不影响 `[Fonts]`（字体配置仍要重启生效）。

### 调试日志

启动时会在游戏 exe 旁生成 `chsfont.log`，记录字体加载/渲染回退过程；字显示不出来时先看它。字体文件版权归各自作者所有，本仓库不包含任何字体文件。




<img src="https://github.com/GTAmodding/re3/blob/miami/logo.png?raw=true" alt="reVC logo" width="200">

[![Build Status](https://img.shields.io/endpoint.svg?url=https%3A%2F%2Factions-badge.atrox.dev%2FGTAmodding%2Fre3%2Fbadge%3Fref%3Dmiami&style=flat)](https://actions-badge.atrox.dev/GTAmodding/re3/goto?ref=miami)
<a href="https://discord.gg/ERYg58ttcE"><img src="https://img.shields.io/badge/discord-join-7289DA.svg?logo=discord&longCache=true&style=flat" /></a>

## Intro

In this repository you'll find the fully reversed source code for GTA III ([master](https://github.com/GTAmodding/re3/tree/master/) branch) and GTA VC ([miami](https://github.com/GTAmodding/re3/tree/miami/) branch).

It has been tested and works on Windows, Linux and FreeBSD, on x86, amd64, arm and arm64.\
Rendering is handled either by original RenderWare (D3D8)
or the reimplementation [librw](https://github.com/aap/librw) (D3D9, OpenGL 2.1 or above, OpenGL ES 2.0 or above).\
Audio is done with MSS (using dlls from original GTA) or OpenAL.

We cannot build for PS2 or Xbox yet. If you're interested in doing so, get in touch with us.

## How can I try it?

- reVC requires game assets to work, so you **must** own [a copy of GTA Vice City](https://store.steampowered.com/app/12110/Grand_Theft_Auto_Vice_City/).
- Build reVC or download the latest build:
  - [Windows D3D9 MSS 32bit](https://nightly.link/GTAmodding/re3/workflows/reVC_msvc_x86/miami/reVC_Release_win-x86-librw_d3d9-mss.zip)
  - [Windows D3D9 64bit](https://nightly.link/GTAmodding/re3/workflows/reVC_msvc_amd64/miami/reVC_Release_win-amd64-librw_d3d9-oal.zip)
  - [Windows OpenGL 64bit](https://nightly.link/GTAmodding/re3/workflows/reVC_msvc_amd64/miami/reVC_Release_win-amd64-librw_gl3_glfw-oal.zip)
  - [Linux 64bit](https://nightly.link/GTAmodding/re3/workflows/build-cmake-conan/miami/ubuntu-latest-gl3.zip)
  - [MacOS 64bit](https://nightly.link/GTAmodding/re3/workflows/build-cmake-conan/miami/macos-latest-gl3.zip)
- Extract the downloaded zip over your GTA VC directory and run reVC. The zip includes the gamefiles and in case of OpenAL the required dlls.

## Screenshots

![screen_ 1613087332](https://user-images.githubusercontent.com/1521437/107714111-f84f3200-6ccc-11eb-902e-d757481d579a.png)
![screen_ 1613086852](https://user-images.githubusercontent.com/1521437/107714115-fa18f580-6ccc-11eb-9de5-eb4cd04865d3.png)
![screen_ 1613086989](https://user-images.githubusercontent.com/1521437/107714103-f38a7e00-6ccc-11eb-88a3-c8c2033c51d6.png)
![screen_ 1613087193](https://user-images.githubusercontent.com/1521437/107714106-f4bbab00-6ccc-11eb-96a9-13821d9b9684.png)

## Improvements

We have implemented a number of changes and improvements to the original game.
They can be configured in `core/config.h`.
Some of them can be toggled at runtime, some cannot.

* Fixed a lot of smaller and bigger bugs
* User files (saves and settings) stored in GTA root directory
* Settings stored in reVC.ini file instead of gta_vc.set
* Debug menu to do and change various things (Ctrl-M to open)
* Debug camera (Ctrl-B to toggle)
* Rotatable camera
* XInput controller support (Windows)
* No loading screens between islands ("map memory usage" in menu)
* Rendering
  * Widescreen support (properly scaled HUD, Menu and FOV)
  * PS2 MatFX (vehicle reflections)
  * PS2 alpha test (better rendering of transparency)
  * Xbox vehicle rendering
  * Xbox world lightmap rendering (needs Xbox map)
  * Xbox ped rim light
  * Xbox screen rain droplets
  * More customizable colourfilter
* Menu
  * More options
  * Controller configuration menu
  * ...
* Can load DFFs and TXDs from other platforms, possibly with a performance penalty
* ...

## To-Do

The following things would be nice to have/do:

* Fix physics for high FPS
* Improve performance on lower end devices, especially the OpenGL layer on the Raspberry Pi (if you have experience with this, please get in touch)
* [PS2 port](https://github.com/GTAmodding/re3/wiki/PS2-port)
* Xbox port (not quite as important)
* reverse remaining unused/debug functions
* compare CodeWarrior build with original binary for more accurate code (very tedious)

## Modding

Asset modifications (models, texture, handling, script, ...) should work the same way as with original GTA for the most part.

Mods that make changes to the code (dll/asi, CLEO, limit adjusters) will *not* work.
Some things these mods do are already implemented in re3 (much of SkyGFX, GInput, SilentPatch, Widescreen fix),
others can easily be achieved (increasing limis, see `config.h`),
others will simply have to be rewritten and integrated into the code directly.
Sorry for the inconvenience.

## Building from Source  

When using premake, you may want to point GTA_VC_RE_DIR environment variable to GTA Vice City root folder if you want the executable to be moved there via post-build script.

Clone the repository with `git clone --recursive -b miami https://github.com/GTAmodding/re3.git reVC`. Then `cd reVC` into the cloned repository.

<details><summary>Linux Premake</summary>

For Linux using premake, proceed: [Building on Linux](https://github.com/GTAmodding/re3/wiki/Building-on-Linux)

</details>

<details><summary>Linux Conan</summary>

Install python and conan, and then run build.
```
conan export vendor/librw librw/master@
mkdir build
cd build
conan install .. reVC/master@ -if build -o reVC:audio=openal -o librw:platform=gl3 -o librw:gl3_gfxlib=glfw --build missing -s reVC:build_type=RelWithDebInfo -s librw:build_type=RelWithDebInfo
conan build .. -if build -bf build -pf package
```
</details>

<details><summary>FreeBSD</summary>

For FreeBSD using premake, proceed: [Building on FreeBSD](https://github.com/GTAmodding/re3/wiki/Building-on-FreeBSD)

</details>

<details><summary>Windows</summary>

Assuming you have Visual Studio 2015/2017/2019:
- Run one of the `premake-vsXXXX.cmd` variants on root folder.
- Open build/reVC.sln with Visual Studio and compile the solution.
    
Microsoft recently discontinued its downloads of the DX9 SDK. You can download an archived version here: https://archive.org/details/dxsdk_jun10

**If you choose OpenAL on Windows** You must read [Running OpenAL build on Windows](https://github.com/GTAmodding/re3/wiki/Running-OpenAL-build-on-Windows).
</details>

> :information_source: premake has an `--lto` option if you want the project to be compiled with Link Time Optimization.

> :information_source: There are various settings in [config.h](https://github.com/GTAmodding/re3/tree/miami/src/core/config.h), you may want to take a look there.

> :information_source: reVC uses completely homebrew RenderWare-replacement rendering engine; [librw](https://github.com/aap/librw/). librw comes as submodule of re3, but you also can use LIBRW enviorenment variable to specify path to your own librw.

If you feel the need, you can also use CodeWarrior 7 to compile reVC using the supplied codewarrior/reVC.mcp project - this requires the original RW34 libraries, and the DX8 SDK. The build is unstable compared to the MSVC builds though, and is mostly meant to serve as a reference.

## Contributing
As long as it's not linux/cross-platform skeleton/compatibility layer, all of the code on the repo that's not behind a preprocessor condition(like FIX_BUGS) are **completely** reversed code from original binaries.  

We **don't** accept custom codes, as long as it's not wrapped via preprocessor conditions, or it's linux/cross-platform skeleton/compatibility layer.

We accept only these kinds of PRs;

- A new feature that exists in at least one of the GTAs (if it wasn't in III/VC then it doesn't have to be decompilation)  
- Game, UI or UX bug fixes (if it's a fix to original code, it should be behind FIX_BUGS)
- Platform-specific and/or unused code that's not been reversed yet
- Makes reversed code more understandable/accurate, as in "which code would produce this assembly".
- A new cross-platform skeleton/compatibility layer, or improvements to them
- Translation fixes, for languages original game supported
- Code that increase maintainability  

We have a [Coding Style](https://github.com/GTAmodding/re3/blob/master/CODING_STYLE.md) document that isn't followed or enforced very well.

Do not use features from C++11 or later.


## History

re3 was started sometime in the spring of 2018,
initially as a way to test reversed collision and physics code
inside the game.
This was done by replacing single functions of the game
with their reversed counterparts using a dll.

After a bit of work the project lay dormant for about a year
and was picked up again and pushed to github in May 2019.
At the time I (aap) had reversed around 10k lines of code and estimated
the final game to have around 200-250k.
Others quickly joined the effort (Fire_Head, shfil, erorcun and Nick007J
in time order, and Serge a bit later) and we made very quick progress
throughout the summer of 2019
after which the pace slowed down a bit.

Due to everyone staying home during the start of the Corona pandemic
everybody had a lot of time to work on re3 again and
we finally got a standalone exe in April 2020 (around 180k lines by then).

After the initial excitement and fixing and polishing the code further,
reVC was started in early May 2020 by starting from re3 code,
not by starting from scratch replacing functions with a dll.
After a few months of mostly steady progress we considered reVC
finished in December.

Since then we have started reLCS, which is currently work in progress.


## License

We don't feel like we're in a position to give this code a license.\
The code should only be used for educational, documentation and modding purposes.\
We do not encourage piracy or commercial use.\
Please keep derivate work open source and give proper credit.
