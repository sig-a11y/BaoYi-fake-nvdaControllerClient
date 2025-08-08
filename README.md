# BaoYi-fake-nvda

> A fake `nvdaControllerClient.dll`, but take BaoYi as its backend. 

让 `nvdaControllerClient.dll` 对接保益读屏


## 背景

NVDA 是一款开源的读屏软件，很多国外的软件、游戏会对接它。
软件通过 `nvdaControllerClient.dll` 这个接口向 NVDA 输出文本，然后 NVDA 朗读出来。
但国内还有其他的读屏软件，就想借用 nvda 的接口，对接到国内的争渡、保益读屏。
所以写了一个 nvdaDll 对接保益读屏的插件，也就是这个项目。

## 功能

已经实现 nvdaDll 导出的所有接口，目前处于优化功能的阶段。
支持通过配置文件 `nvdaCfg.ini` 进行一些设置，修改配置文件需要重启程序。

### 实现的接口
[`nvdll_impl.h`](nvdll_impl.h)：
- `testIfRunning_impl`: 检查读屏器是否在运行
- `speakText_impl`: 向读屏器输出文本
- `cancelSpeech_impl`: 静音当前输出
- `brailleMessage_impl`: 【暂不支持/空实现】输出盲文

> 备注：头文件中的函数名和实际导出的函数名并不一致。
> 导出函数名通过 [`dll.def`](dll.def) 指定。

### 可用的配置
默认配置：[`nvdaCfg.ini`](nvdaCfg.ini)
*[APP]*
- `USE_SLAVE=1`: 独立通道：0使用读屏通道；1使用独立通道
- `USE_APPEND=1`: 排队朗读：0打断朗读；1排队朗读
- `ALLOW_BREAK=2`: 使用打断键：独立通道=1时，有效。  
    0永不打断，按下打断键也不打断朗读；
    1仅按下打断键打断；
    2按下任意按键可以打断朗读

> 备注：`ALLOW_BREAK=2` 为本项目新添加的功能。保益截至 v1.5.3 版本尚不支持按任意键打断。


## 版权
> All rights reserved
> 保留所有权利

- `baoyi-dll/` 文件夹下的除 `.md` 文件外，所有文件都归属保益读屏所有。
- `deps/` 文件夹为第三方依赖，各自有独立的版权协议
    - `loguru`: [PD](deps/loguru/loguru.hpp)
    - `simpleini`: [MIT](deps/simpleini/LICENCE.txt)
    - `spdlog`: [MIT](deps/spdlog/LICENSE)
- 其余文件为本项目所属文件
