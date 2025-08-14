using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace ZDSRAPI_CSharp
{
    class ZDSRAPI
    {
#if WIN64
        const string dllPath = "ZDSRAPI_x64.dll";
#else
        const string dllPath = "ZDSRAPI.dll";
#endif
        /// <summary>
        /// 初始化语音接口
        /// </summary>
        /// <param name="channelType">0 读屏通道 1 独立通道</param>
        /// <param name="channelName">独立通道名称, type为0时忽略,传入NULL; type为1时:独立通道名称, NULL或空字符串时,将使用默认名称"API"</param>
        /// <param name="bKeyDownInterrupt">true 按键打断朗读 false 按键不打断朗读</param>
        /// <returns>0:成功; 1:版本不匹配;</returns>
        [DllImport(dllPath)]
        internal static extern int InitTTS(int channelType, [MarshalAs(UnmanagedType.LPWStr)] string channelName, [MarshalAs(UnmanagedType.Bool)]bool bKeyDownInterrupt);

        /// <summary>
        /// 朗读文本
        /// </summary>
        /// <param name="text">要朗读的文本,Unicode</param>
        /// <param name="bInterrupt">true:打断朗读, false:等待空闲时朗读</param>
        /// <returns>0:成功; 1:版本不匹配; 2:ZDSR没有运行</returns>
        [DllImport(dllPath)]
        internal static extern int Speak([MarshalAs(UnmanagedType.LPWStr)] string text, [MarshalAs(UnmanagedType.Bool)]bool bInterrupt);

        /// <summary>
        /// 获取状态
        /// </summary>
        /// <returns>1:版本不匹配; 2:ZDSR没有运行或没有授权; 3:正在朗读; 4:没有朗读</returns>
        [DllImport(dllPath)]
        internal static extern int GetSpeakState();

        /// <summary>
        /// 停止朗读
        /// </summary>
        [DllImport(dllPath)]
        internal static extern void StopSpeak();
    }
}
