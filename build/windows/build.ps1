[CmdletBinding()]
param(
  [ValidateSet("Debug", "Release")]
  [string]$Configuration = "Debug",
  [string]$VcpkgRoot = "",
  [switch]$Clean,
  [int]$Jobs = 8
)

$ErrorActionPreference = "Stop"
$RepoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$WorkspaceRoot = Split-Path -Parent $RepoRoot
$WindowsDependenciesRoot = Join-Path $WorkspaceRoot "windows-dependencies"
$CommonScript = Join-Path $WindowsDependenciesRoot "vcpkg-script\dali-build-common.ps1"
if(-not (Test-Path -LiteralPath $CommonScript))
{
  throw "windows-dependencies must be beside dali-adaptor: $WindowsDependenciesRoot"
}
. $CommonScript

$Context = New-DaliBuildContext -WindowsDependenciesRoot $WindowsDependenciesRoot -VcpkgRoot $VcpkgRoot
Initialize-DaliBuildEnvironment -Context $Context
$WindowsDependenciesPackage = Join-Path $Context.SdkRoot "share\dali-windows-dependencies"
$CorePackage = Join-Path $Context.InstallPrefix "share\dali2-core"
Assert-DaliPaths -Paths @(
  (Join-Path $WindowsDependenciesPackage "dali-windows-dependencies-config.cmake"),
  (Join-Path $CorePackage "dali2-core-config.cmake")
) -Description "prerequisite package; install dependencies and build dali-core first"

$TizenHeaders = @(
  (Join-Path $Context.SdkRoot "include\thorvg.h"),
  (Join-Path $Context.SdkRoot "include\thorvg_lottie.h")
)
$TizenDlls = @(
  (Join-Path $Context.SdkRoot "bin\thorvg.dll"),
  (Join-Path $Context.SdkRoot "lib\thorvg.dll")
)
$TizenHeaderCount = @($TizenHeaders | Where-Object { Test-Path -LiteralPath $_ }).Count
$TizenDllCount = @($TizenDlls | Where-Object { Test-Path -LiteralPath $_ }).Count
if($TizenHeaderCount -eq 0 -and $TizenDllCount -eq 0)
{
  $ThorVgSupport = "OFF"
  Write-Host "TizenVG is not installed. Building without CanvasRenderer/Lottie support."
}
elseif($TizenHeaderCount -eq $TizenHeaders.Count -and $TizenDllCount -gt 0)
{
  $ThorVgSupport = "ON"
  Write-Host "TizenVG installation detected. Building with CanvasRenderer/Lottie support."
}
else
{
  throw "The TizenVG installation in WindowsDependenciesSDK is incomplete. Re-run windows-dependencies\install.ps1."
}

$Arguments = (Get-DaliCommonCMakeArguments -Context $Context -Configuration $Configuration) + @(
  "-DENABLE_PKG_CONFIGURE=OFF",
  "-DENABLE_LINK_TEST=OFF",
  "-DINSTALL_CMAKE_MODULES=ON",
  "-DPROFILE_LCASE=windows",
  "-DENABLE_PROFILE=WINDOWS",
  "-DENABLE_GRAPHICS_BACKEND=GLES",
  "-DENABLE_VECTOR_BASED_TEXT_RENDERING=OFF",
  "-Dthorvg_support=$ThorVgSupport",
  "-Ddali-windows-dependencies_DIR=$WindowsDependenciesPackage",
  "-Ddali2-core_DIR=$CorePackage"
)
Invoke-DaliCMakeProject `
  -Name "dali-adaptor" `
  -SourceDirectory (Join-Path $RepoRoot "build\tizen") `
  -BuildDirectory (Join-Path $RepoRoot "_build\windows") `
  -ConfigureArguments $Arguments `
  -Clean:$Clean `
  -Jobs $Jobs

Install-DaliRuntimeScripts -Context $Context
Assert-DaliPaths -Paths @(
  (Join-Path $Context.InstallPrefix "bin\dali2-adaptor.dll"),
  (Join-Path $Context.InstallPrefix "share\dali2-adaptor\dali2-adaptor-config.cmake")
) -Description "dali-adaptor installation output"
Write-Host "`ndali-adaptor installed in $($Context.InstallPrefix)." -ForegroundColor Green
