# PowerShell 脚本：自动打包发布 nvda2Boy
# 功能：复制文件到指定目录，创建带时间戳的文件夹并压缩为zip

param(
    [string]$BaseDir = "nvda2Boy",
    [switch]$Force = $false
)

# 设置错误处理
$ErrorActionPreference = "Stop"

# 获取脚本所在目录
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ScriptDir

Write-Host "=== nvda2Boy 自动打包发布脚本 ===" -ForegroundColor Green
Write-Host "工作目录: $ScriptDir" -ForegroundColor Yellow

# 生成时间戳 (格式: 20250808+2136)
$DateTime = Get-Date -Format "yyyyMMdd+HHmm"
$TargetDir = "$BaseDir-$DateTime"

Write-Host "目标目录: $TargetDir" -ForegroundColor Yellow

try {
    # 1. 创建目标目录结构
    Write-Host "`n[1/6] 创建目录结构..." -ForegroundColor Cyan
    
    $x64Dir = "$BaseDir\x64"
    $x86Dir = "$BaseDir\x86"
    
    if (!(Test-Path $x64Dir)) {
        New-Item -ItemType Directory -Path $x64Dir -Force | Out-Null
        Write-Host "  创建目录: $x64Dir" -ForegroundColor Gray
    }
    
    if (!(Test-Path $x86Dir)) {
        New-Item -ItemType Directory -Path $x86Dir -Force | Out-Null
        Write-Host "  创建目录: $x86Dir" -ForegroundColor Gray
    }

    # 2. 复制 x64 文件
    Write-Host "`n[2/6] 复制 x64 文件..." -ForegroundColor Cyan
    
    $x64Files = @{
        "nvdaCfg.ini" = "nvdaCfg.ini"
        "VS2022\x64\Release\BoyCtrl-x64.dll" = "BoyCtrl-x64.dll"
        "VS2022\x64\Release\nvdaControllerClient64.dll" = "nvdaControllerClient64.dll"
        "VS2022\x64\Release\NvdaTest.exe" = "NvdaTest.exe"
    }
    
    foreach ($file in $x64Files.GetEnumerator()) {
        $sourcePath = $file.Key
        $targetPath = "$x64Dir\$($file.Value)"
        
        if (Test-Path $sourcePath) {
            Copy-Item -Path $sourcePath -Destination $targetPath -Force
            Write-Host "  ✓ $sourcePath -> $targetPath" -ForegroundColor Green
        } else {
            Write-Warning "  ✗ 源文件不存在: $sourcePath"
        }
    }

    # 3. 复制 x86 文件
    Write-Host "`n[3/6] 复制 x86 文件..." -ForegroundColor Cyan
    
    $x86Files = @{
        "nvdaCfg.ini" = "nvdaCfg.ini"
        "VS2022\Release\BoyCtrl.dll" = "BoyCtrl.dll"
        "VS2022\Release\nvdaControllerClient.dll" = "nvdaControllerClient.dll"
        "VS2022\Release\nvdaControllerClient32.dll" = "nvdaControllerClient32.dll"
        "VS2022\Release\NvdaTest.exe" = "NvdaTest.exe"
    }
    
    foreach ($file in $x86Files.GetEnumerator()) {
        $sourcePath = $file.Key
        $targetPath = "$x86Dir\$($file.Value)"
        
        if (Test-Path $sourcePath) {
            Copy-Item -Path $sourcePath -Destination $targetPath -Force
            Write-Host "  ✓ $sourcePath -> $targetPath" -ForegroundColor Green
        } else {
            Write-Warning "  ✗ 源文件不存在: $sourcePath"
        }
    }

    # 4. 复制整个 nvda2Boy 文件夹到带时间戳的目录
    Write-Host "`n[4/6] 创建带时间戳的发布目录..." -ForegroundColor Cyan
    
    if (Test-Path $TargetDir) {
        if ($Force) {
            Remove-Item -Path $TargetDir -Recurse -Force
            Write-Host "  已删除现有目录: $TargetDir" -ForegroundColor Yellow
        } else {
            throw "目标目录已存在: $TargetDir (使用 -Force 参数强制覆盖)"
        }
    }
    
    Copy-Item -Path $BaseDir -Destination $TargetDir -Recurse -Force
    Write-Host "  ✓ 复制 $BaseDir -> $TargetDir" -ForegroundColor Green

    # 5. 压缩为 ZIP 文件
    Write-Host "`n[5/6] 压缩为 ZIP 文件..." -ForegroundColor Cyan
    
    $ZipPath = "$TargetDir.zip"
    
    if (Test-Path $ZipPath) {
        if ($Force) {
            Remove-Item -Path $ZipPath -Force
            Write-Host "  已删除现有ZIP文件: $ZipPath" -ForegroundColor Yellow
        } else {
            throw "ZIP文件已存在: $ZipPath (使用 -Force 参数强制覆盖)"
        }
    }
    
    # 使用 .NET 压缩
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    [System.IO.Compression.ZipFile]::CreateFromDirectory($TargetDir, $ZipPath)
    
    Write-Host "  ✓ 创建ZIP文件: $ZipPath" -ForegroundColor Green

    # 6. 显示结果信息
    Write-Host "`n[6/6] 打包完成!" -ForegroundColor Cyan
    
    $ZipSize = [math]::Round((Get-Item $ZipPath).Length / 1MB, 2)
    Write-Host "  ZIP文件大小: $ZipSize MB" -ForegroundColor Gray
    Write-Host "  ZIP文件路径: $(Resolve-Path $ZipPath)" -ForegroundColor Gray

    Write-Host "`n=== 打包发布完成! ===" -ForegroundColor Green
    Write-Host "发布文件: $ZipPath" -ForegroundColor Yellow

} catch {
    Write-Host "`n❌ 错误: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host "脚本执行失败!" -ForegroundColor Red
    exit 1
}

# # 暂停以便查看结果
# if ($Host.Name -eq "ConsoleHost") {
#     Write-Host "`n按任意键退出..." -ForegroundColor Gray
#     $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
# }
