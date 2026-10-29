[CmdletBinding()]
param(
  [ValidateSet("Debug", "Release")]
  [string]$Configuration = "Debug",
  [string[]]$Modules = @(),
  [string]$VcpkgRoot = "",
  [switch]$Clean,
  [int]$Jobs = 8
)

$ErrorActionPreference = "Stop"
$TestRoot = $PSScriptRoot
$RepoRoot = Split-Path -Parent $TestRoot
$WorkspaceRoot = Split-Path -Parent $RepoRoot
$WindowsDependenciesRoot = Join-Path $WorkspaceRoot "windows-dependencies"
$CommonScript = Join-Path $WindowsDependenciesRoot "vcpkg-script\dali-build-common.ps1"
$SupportedModules = @("dali-adaptor", "dali-adaptor-windows")

if($Modules.Count -gt 0)
{
  $UnsupportedModules = @($Modules | Where-Object { $SupportedModules -notcontains $_ })
  if($UnsupportedModules.Count -gt 0)
  {
    throw "Unsupported Windows test module: $($UnsupportedModules -join ', '). Supported modules: $($SupportedModules -join ', ')"
  }
}

if(-not (Test-Path -LiteralPath $CommonScript))
{
  throw "windows-dependencies must be beside dali-adaptor: $WindowsDependenciesRoot"
}
. $CommonScript

$Context = New-DaliBuildContext `
  -WindowsDependenciesRoot $WindowsDependenciesRoot `
  -VcpkgRoot $VcpkgRoot
Initialize-DaliBuildEnvironment -Context $Context

$CorePackage = Join-Path $Context.InstallPrefix "share\dali2-core"
$AdaptorPackage = Join-Path $Context.InstallPrefix "share\dali2-adaptor"
Assert-DaliPaths -Paths @(
  (Join-Path $CorePackage "dali2-core-config.cmake"),
  (Join-Path $AdaptorPackage "dali2-adaptor-config.cmake")
) -Description "DALi package; build dali-core and dali-adaptor first"

$Arguments = (Get-DaliCommonCMakeArguments -Context $Context -Configuration $Configuration) + @(
  "-Ddali2-core_DIR=$CorePackage",
  "-Ddali2-adaptor_DIR=$AdaptorPackage"
)

Invoke-DaliCMakeProject `
  -Name "dali-adaptor common UTC on Windows" `
  -SourceDirectory (Join-Path $TestRoot "windows-common") `
  -BuildDirectory (Join-Path $TestRoot "_build\windows") `
  -ConfigureArguments $Arguments `
  -Clean:$Clean `
  -Jobs $Jobs

Install-DaliRuntimeScripts -Context $Context
$TestExecutable = Join-Path $Context.InstallPrefix "bin\tct-dali-adaptor-core.exe"
Assert-DaliPaths -Paths @($TestExecutable) -Description "Windows test executable"

Write-Host "`nDALi adaptor common UTCs installed for Windows." -ForegroundColor Green
Write-Host "Run from the workspace root: .\dali-adaptor\automated-tests\execute.ps1"
