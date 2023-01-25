# BaoYi-fake-nvda

> A fake `nvdaControllerClient.dll`, but take BaoYi as its backend. 

让 `nvdaControllerClient.dll` 对接保益读屏


## 背景

NVDA 是一款开源的读屏软件，很多国外的软件、游戏会对接它。
通过 `nvdaControllerClient.dll` 这个接口向 NVDA 输出文本，然后 NVDA 朗读出来。
但国内还有其他的读屏软件，就想借用 nvda 的接口，对接到国内的争渡、保益读屏。
所以写了一个 nvdaDll 对接保益读屏的插件，也就是这个项目。
