# Headless local APK build for Windows — no Android Studio required.
# Downloads: cmdline-tools (~130MB) + platform + build-tools + NDK r26 + CMake + Gradle 8.7 (~2GB total).
# Needs: 64-bit Windows, ~6GB free, internet. Run once; re-runs reuse the SDK.
# NOTE: your system Java is 24 (too new for AGP 8.5); this script fetches Temurin JDK 17.

param(
  [string]$SdkRoot = "$env:LOCALAPPDATA\PlatinumFast\Sdk",
  [string]$WorkRoot = "$env:LOCALAPPDATA\PlatinumFast\tools"
)

$ErrorActionPreference = 'Stop'
New-Item -ItemType Directory -Force -Path $SdkRoot, $WorkRoot | Out-Null

function Get-File([string]$Url, [string]$Out) {
  if (Test-Path -LiteralPath $Out) { Write-Host "cached: $Out"; return }
  Write-Host "downloading $Url ..."
  Invoke-WebRequest -Uri $Url -OutFile $Out
}

# 1. JDK 17 (Temurin)
$jdkZip = "$WorkRoot\jdk17.zip"
Get-File 'https://api.adoptium.net/v3/binary/latest/17/ga/windows/x64/jdk/hotspot/normal/eclipse' $jdkZip
$jdkDir = "$WorkRoot\jdk-17"
if (-not (Test-Path -LiteralPath $jdkDir)) {
  Expand-Archive -LiteralPath $jdkZip -DestinationPath "$WorkRoot\jdk-tmp" -Force
  Get-ChildItem "$WorkRoot\jdk-tmp" -Directory | Select-Object -First 1 |
    Move-Item -Destination $jdkDir
}
$env:JAVA_HOME = $jdkDir
$env:PATH = "$jdkDir\bin;$env:PATH"

# 2. Android cmdline-tools
$cmdZip = "$WorkRoot\cmdtools.zip"
Get-File 'https://dl.google.com/android/repository/commandlinetools-win-11076708_latest.zip' $cmdZip
$cmdDir = "$SdkRoot\cmdline-tools\latest"
if (-not (Test-Path -LiteralPath "$cmdDir\bin\sdkmanager.bat")) {
  Expand-Archive -LiteralPath $cmdZip -DestinationPath "$WorkRoot\cmd-tmp" -Force
  New-Item -ItemType Directory -Force -Path $cmdDir | Out-Null
  Copy-Item "$WorkRoot\cmd-tmp\cmdline-tools\*" $cmdDir -Recurse -Force
}
$env:ANDROID_HOME = $SdkRoot
$env:ANDROID_SDK_ROOT = $SdkRoot
$env:PATH = "$cmdDir\bin;$SdkRoot\platform-tools;$env:PATH"

# 3. SDK packages (accept licenses non-interactively)
$sdkman = "$cmdDir\bin\sdkmanager.bat"
"platform-tools", "platforms;android-36", "build-tools;36.0.0",
"ndk;26.3.11579264", "cmake;3.22.1" | ForEach-Object {
  & $sdkman --install $_ 2>&1 | Select-Object -Last 2
}
(1..20 | ForEach-Object { 'y' }) | & $sdkman --licenses | Out-Null

# 4. Gradle 8.7 (no wrapper needed)
$gradleZip = "$WorkRoot\gradle-8.7-bin.zip"
Get-File 'https://services.gradle.org/distributions/gradle-8.7-bin.zip' $gradleZip
$gradleDir = "$WorkRoot\gradle-8.7"
if (-not (Test-Path -LiteralPath "$gradleDir\bin\gradle.bat")) {
  Expand-Archive -LiteralPath $gradleZip -DestinationPath $WorkRoot -Force
}

# 5. Build from the project root (parent of tools/)
$proj = Split-Path -Parent $PSScriptRoot
Set-Location -LiteralPath $proj
Write-Host "Building with SDK=$SdkRoot ..."
& "$gradleDir\bin\gradle.bat" :app:assembleDebug --no-daemon
Write-Host "APK: $proj\app\build\outputs\apk\debug\app-debug.apk"
