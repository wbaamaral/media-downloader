#Requires -Version 5.1
# ==============================================================================
# Nome:        build_windows.ps1
# Descrição:   Build unificado + empacotamento (zip/Inno Setup) para Windows
# Autor:       wba-skill-sysadm
# Data:        2026-07-27
# Uso:         .\build_windows.ps1 [-QtVersion 5|6|auto] [-Clean] [-Package] [-Run] [-DryRun]
# Dependências: PowerShell 5.1+, CMake, MinGW (Qt), Git
# ==============================================================================

[CmdletBinding()]
param(
    [ValidateSet("5","6","auto")]
    [string]$QtVersion = "auto",

    [ValidateSet("x86","x64")]
    [string]$Arch = "x64",

    [switch]$Clean,
    [switch]$Package,
    [switch]$Run,
    [switch]$DryRun
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$PSDefaultParameterValues['Out-File:Encoding'] = 'utf8'

# ── Helpers visuais ───────────────────────────────────────────────────────────
function Write-Title {
    param([string]$m)
    Write-Host ''
    Write-Host ('=' * 60) -ForegroundColor Cyan
    Write-Host "  $m" -ForegroundColor Cyan
    Write-Host ('=' * 60) -ForegroundColor Cyan
}
function Write-Info  { param([string]$m) Write-Host "[INFO]  $m" -ForegroundColor White }
function Write-Ok    { param([string]$m) Write-Host "[OK]    $m" -ForegroundColor Green }
function Write-Fail  { param([string]$m) Write-Host "[FALHA] $m" -ForegroundColor Red }
function Write-Warn  { param([string]$m) Write-Host "[AVISO] $m" -ForegroundColor Yellow }

# ── Logging ───────────────────────────────────────────────────────────────────
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$LogDir    = Join-Path $ScriptDir "logs"
if (-not (Test-Path $LogDir)) { New-Item -ItemType Directory -Path $LogDir -Force | Out-Null }

$LogFile = Join-Path $LogDir ("build-{0}.log" -f (Get-Date -Format 'yyyyMMdd-HHmmss'))

function Write-Log {
    param([string]$Message)
    $ts = Get-Date -Format 'yyyy-MM-dd HH:mm:ss'
    $entry = "[$ts] $Message"
    Add-Content -Path $LogFile -Value $entry -Encoding UTF8
}

# Wrapper para executáveis externos
function Invoke-Ext {
    param(
        [string]$FilePath,
        [string[]]$Arguments = @(),
        [string]$WorkingDirectory = "."
    )

    $cmd = Get-Command $FilePath -ErrorAction SilentlyContinue
    if (-not $cmd) {
        Write-Fail "Executável não encontrado: $FilePath"
        return [PSCustomObject]@{ ExitCode = 127; Output = "Não encontrado: $FilePath" }
    }

    if ($DryRun) {
        Write-Host "[DRY]   $FilePath $($Arguments -join ' ')" -ForegroundColor Yellow
        return [PSCustomObject]@{ ExitCode = 0; Output = "(dry-run)" }
    }

    Write-Info "Executando: $FilePath $($Arguments -join ' ')"
    Write-Log "EXEC: $FilePath $($Arguments -join ' ')"

    try {
        $pinfo = New-Object System.Diagnostics.ProcessStartInfo
        $pinfo.FileName = $FilePath
        # Quote arguments that contain spaces
        $quotedArgs = $Arguments | ForEach-Object {
            if ($_ -match '\s') { "`"$_`"" } else { $_ }
        }
        $pinfo.Arguments = $quotedArgs -join ' '
        $pinfo.RedirectStandardOutput = $true
        $pinfo.RedirectStandardError = $true
        $pinfo.UseShellExecute = $false
        $pinfo.CreateNoWindow = $true

        $proc = [System.Diagnostics.Process]::Start($pinfo)

        $stdoutTask = $proc.StandardOutput.ReadToEndAsync()
        $stderrTask = $proc.StandardError.ReadToEndAsync()
        $proc.WaitForExit()
        $stdout = $stdoutTask.Result
        $stderr = $stderrTask.Result
        $exitCode = $proc.ExitCode

        if ($stderr) {
            Write-Log "Stderr: $stderr"
        }
        if ($exitCode -ne 0) {
            Write-Fail "Falhou (exit $exitCode): $FilePath"
            Write-Log "FALHA (exit $exitCode): $stdout"
        }
        return [PSCustomObject]@{ ExitCode = $exitCode; Output = $stdout.Trim() }
    } catch {
        Write-Fail "Excecao: $($_.Exception.Message)"
        Write-Log "EXCECAO: $($_.Exception.Message)"
        return [PSCustomObject]@{ ExitCode = 1; Output = $_.Exception.Message }
    }
}

function Assert-Tool {
    param([string]$Name, [string]$Description)
    $cmd = Get-Command $Name -ErrorAction SilentlyContinue
    if ($cmd) {
        Write-Ok "${Description}: $($cmd.Source)"
        return $true
    } else {
        Write-Fail "$Description NÃO encontrado ($Name)"
        return $false
    }
}

# ── Detecção de Qt ────────────────────────────────────────────────────────────
function Find-Qt {
    param(
        [string]$Version = "auto",
        [string]$TargetArch = "x64"
    )

    $searchPaths = @(
        "C:\Qt",
        "$env:USERPROFILE\Qt",
        "$env:LOCALAPPDATA\Qt",
        "C:\Program Files\Qt",
        "C:\Program Files (x86)\Qt"
    )

    $results = New-Object System.Collections.ArrayList

    foreach ($base in $searchPaths) {
        if (-not (Test-Path $base)) { continue }

        $versions = @(Get-ChildItem -Path $base -Directory -ErrorAction SilentlyContinue |
            Where-Object { $_.Name -match '^[56]\.\d+\.\d+$' })

        foreach ($v in $versions) {
            $major = $v.Name.Split('.')[0]
            if ($Version -ne "auto" -and $major -ne $Version) { continue }

            $subDirs = @(Get-ChildItem -Path $v.FullName -Directory -ErrorAction SilentlyContinue)

            foreach ($sub in $subDirs) {
                $subName = $sub.Name.ToLower()
                $isX64 = $subName.Contains('64') -and (-not $subName.Contains('32'))
                $isX86 = $subName.Contains('32') -or $subName.Contains('86')
                $isMinGW = $subName.Contains('mingw')
                $isMSVC = $subName.Contains('msvc')

                if (($TargetArch -eq "x64" -and $isX64) -or ($TargetArch -eq "x86" -and $isX86)) {
                    if ($isMinGW -or $isMSVC) {
                        $compiler = if ($isMinGW) { 'MinGW' } else { 'MSVC' }
                        $results.Add([PSCustomObject]@{
                            Path     = $sub.FullName
                            Version  = $v.Name
                            Compiler = $compiler
                            Arch     = $TargetArch
                        }) | Out-Null
                    }
                }
            }
        }
    }

    if ($results.Count -gt 0) {
        # Qt6 > Qt5, MinGW > MSVC
        $sorted = $results | Sort-Object @{Expression={$_.Version.Split('.')[0]}; Ascending=$false},
                                          @{Expression={if($_.Compiler -eq 'MinGW'){0}else{1}}; Ascending=$true}
        return @($sorted)[0]
    }
    return $null
}

# ── Detecção de MinGW ─────────────────────────────────────────────────────────
function Find-MinGW {
    param([string]$QtPath)

    # Procurar MinGW junto ao Qt
    if ($QtPath) {
        $qtBase = Split-Path -Parent $QtPath  # C:\Qt\5.15.2
        $toolsDir = Join-Path (Split-Path -Parent $qtBase) "Tools"

        if (Test-Path $toolsDir) {
            $mingwDirs = Get-ChildItem -Path $toolsDir -Directory -ErrorAction SilentlyContinue |
                Where-Object { $_.Name -match 'mingw' } |
                Sort-Object Name -Descending

            foreach ($m in $mingwDirs) {
                $gcc = Join-Path $m.FullName "bin\g++.exe"
                if (Test-Path $gcc) {
                    return $m.FullName
                }
            }
        }
    }

    # Fallback: PATH global
    $gpp = Get-Command g++.exe -ErrorAction SilentlyContinue
    if ($gpp) { return (Split-Path -Parent $gpp.Source) }

    return $null
}

# ── Detecção de Inno Setup ────────────────────────────────────────────────────
function Find-InnoSetup {
    $candidates = @(
        "C:\Program Files (x86)\Inno Setup 6",
        "C:\Program Files\Inno Setup 6",
        "C:\Program Files (x86)\Inno Setup 5",
        "C:\Program Files\Inno Setup 5"
    )
    foreach ($p in $candidates) {
        $iscc = Join-Path $p "iscc.exe"
        if (Test-Path $iscc) { return $p }
    }
    return $null
}

# ── Detecção de 7-Zip ─────────────────────────────────────────────────────────
function Find-7Zip {
    $candidates = @(
        "C:\Program Files\7-Zip",
        "C:\Program Files (x86)\7-Zip",
        "C:\projects\7-Zip",
        "$env:USERPROFILE\7-Zip"
    )
    foreach ($p in $candidates) {
        $sz = Join-Path $p "7z.exe"
        if (Test-Path $sz) { return $p }
    }
    # Fallback: PATH global
    $cmd = Get-Command 7z.exe -ErrorAction SilentlyContinue
    if ($cmd) { return (Split-Path -Parent $cmd.Source) }
    return $null
}

# ── Leitura de versão do build ────────────────────────────────────────────────
function Get-BuildVersion {
    param([string]$BuildDir)

    $versionFile = Join-Path $BuildDir "VERSION_INFO"
    if (Test-Path $versionFile) {
        return (Get-Content $versionFile -Raw).Trim()
    }

    $versionH = Join-Path $BuildDir "version.h"
    if (Test-Path $versionH) {
        $content = Get-Content $versionH -Raw
        if ($content -match 'VERSION\s+"([^"]+)"') {
            return $Matches[1].Replace('.git_tag', '')
        }
    }

    # Fallback: ler do CMakeLists.txt
    $cmakeLists = Join-Path $ScriptDir "CMakeLists.txt"
    if (Test-Path $cmakeLists) {
        $cmakeContent = Get-Content $cmakeLists -Raw
        if ($cmakeContent -match 'set\(\s*PGR_VERSION\s+"([^"]+)"') {
            return $Matches[1]
        }
    }

    return "0.0.0"
}

# ══════════════════════════════════════════════════════════════════════════════
# FLUXO PRINCIPAL
# ══════════════════════════════════════════════════════════════════════════════

if ($DryRun) {
    Write-Warn "MODO DRY-RUN: nenhuma alteração será aplicada."
}

Write-Log "=== INÍCIO DO BUILD ==="
Write-Log "Parâmetros: QtVersion=$QtVersion Arch=$Arch Clean=$Clean Package=$Package DryRun=$DryRun"

# ── 1. Detectar ferramentas ───────────────────────────────────────────────────
Write-Title "Detectando ferramentas"

$depsOk = $true
$depsOk = (Assert-Tool "cmake.exe" "CMake") -and $depsOk
$depsOk = (Assert-Tool "git.exe" "Git") -and $depsOk

# Detectar Qt
$qtInfo = Find-Qt -Version $QtVersion -TargetArch $Arch
if (-not $qtInfo) {
    Write-Fail "Qt não encontrado. Instale via Qt Online Installer ou defina `$env:QT_PATH"
    exit 1
}
Write-Ok "Qt $($qtInfo.Version) ($($qtInfo.Compiler), $($qtInfo.Arch)): $($qtInfo.Path)"

# Detectar MinGW
$mingwPath = Find-MinGW -QtPath $qtInfo.Path
if ($mingwPath) {
    Write-Ok "MinGW: $mingwPath"
} else {
    Write-Warn "MinGW não encontrado automaticamente — verifique se está no PATH"
}

# Detectar Inno Setup
$innoPath = Find-InnoSetup
if ($innoPath) {
    Write-Ok "Inno Setup: $innoPath"
} else {
    Write-Warn "Inno Setup não encontrado — instalador (.iss) não será gerado"
}

# Detectar 7-Zip
$sevenZipPath = Find-7Zip
if ($sevenZipPath) {
    Write-Ok "7-Zip: $sevenZipPath"
} else {
    Write-Warn "7-Zip não encontrado — pacote .zip não será gerado"
}

if (-not $depsOk) {
    Write-Fail "Dependências críticas faltando. Abortando."
    exit 1
}

# ── 2. Configurar PATH ────────────────────────────────────────────────────────
Write-Title "Configurando PATH"

$pathAdditions = @()
$pathAdditions += Join-Path $qtInfo.Path "bin"
if ($mingwPath) { $pathAdditions += Join-Path $mingwPath "bin" }
if ($innoPath)  { $pathAdditions += $innoPath }
if ($sevenZipPath) { $pathAdditions += $sevenZipPath }

foreach ($p in $pathAdditions) {
    if ($p -and (Test-Path $p) -and $env:Path -notlike "*$p*") {
        $env:Path = "$p;$env:Path"
        Write-Info "PATH +: $p"
    }
}

# ── 3. Variáveis de build ────────────────────────────────────────────────────
$SrcDir  = $ScriptDir
$BuildDir = Join-Path $SrcDir "BUILD"
$DistDir = Join-Path $SrcDir "dist"
if (-not (Test-Path $DistDir)) { New-Item -ItemType Directory -Path $DistDir -Force | Out-Null }

# ── 4. Limpar build (se solicitado) ──────────────────────────────────────────
if ($Clean) {
    Write-Title "Limpando BUILD/"
    if (Test-Path $BuildDir) {
        if ($DryRun) {
            Write-Host "[DRY]   Remove-Item -Path '$BuildDir' -Recurse -Force" -ForegroundColor Yellow
        } else {
            Remove-Item -Path $BuildDir -Recurse -Force
            Write-Ok "BUILD/ removido"
        }
    } else {
        Write-Info "BUILD/ não existe, nada a limpar"
    }
}

# ── 5. Configurar CMake ──────────────────────────────────────────────────────
Write-Title "Configurando CMake"

$cmakeBuildDir = Join-Path $BuildDir "MediaDownloader"
if (-not (Test-Path $cmakeBuildDir)) {
    New-Item -ItemType Directory -Path $cmakeBuildDir -Force | Out-Null
}

$cmakeArgs = @(
    "-DCMAKE_VERBOSE_MAKEFILE=FALSE"
    "-DCMAKE_BUILD_TYPE:STRING=Release"
    "-DCMAKE_PREFIX_PATH=$($qtInfo.Path)"
    "-DCMAKE_C_COMPILER=$mingwPath\bin\gcc.exe"
    "-DCMAKE_CXX_COMPILER=$mingwPath\bin\g++.exe"
    "-G", "MinGW Makefiles"
    "-S", $SrcDir
    "-B", $cmakeBuildDir
)

$result = Invoke-Ext -FilePath "cmake.exe" -Arguments $cmakeArgs
if ($result.ExitCode -ne 0) {
    Write-Fail "CMake configure falhou"
    exit 1
}
Write-Ok "CMake configure concluído"

# ── 6. Ler versão ────────────────────────────────────────────────────────────
$VERSION = Get-BuildVersion -BuildDir $cmakeBuildDir
Write-Info "Versão: $VERSION"
Write-Log "Versão detectada: $VERSION"

# Reconfigurar com LIBRARIES_LOCATION correto
$portableDir = "MediaDownloader-$VERSION"
$cmakeArgs2 = @(
    "-DCMAKE_VERBOSE_MAKEFILE=FALSE"
    "-DCMAKE_BUILD_TYPE:STRING=Release"
    "-DCMAKE_PREFIX_PATH=$($qtInfo.Path)"
    "-DLIBRARIES_LOCATION=$BuildDir\$portableDir"
    "-DOUTPUT_PATH=$BuildDir"
    "-DSOURCE_PATH=$SrcDir"
    "-DCMAKE_C_COMPILER=$mingwPath\bin\gcc.exe"
    "-DCMAKE_CXX_COMPILER=$mingwPath\bin\g++.exe"
    "-G", "MinGW Makefiles"
    "-S", $SrcDir
    "-B", $cmakeBuildDir
)

$result = Invoke-Ext -FilePath "cmake.exe" -Arguments $cmakeArgs2
if ($result.ExitCode -ne 0) {
    Write-Fail "CMake reconfigure falhou"
    exit 1
}

# ── 7. Build ─────────────────────────────────────────────────────────────────
Write-Title "Compilando"

$result = Invoke-Ext -FilePath "cmake.exe" -Arguments @("--build", $cmakeBuildDir)
if ($result.ExitCode -ne 0) {
    Write-Fail "Build falhou"
    exit 1
}
Write-Ok "Build concluído"

# ── 8. Empacotamento ─────────────────────────────────────────────────────────
if ($Package) {
    Write-Title "Empacotamento"

    $portablePath = Join-Path $BuildDir $portableDir

    # Limpar staging anterior
    if (Test-Path $portablePath) {
        if ($DryRun) {
            Write-Host "[DRY]   Remove-Item -Path '$portablePath' -Recurse -Force" -ForegroundColor Yellow
        } else {
            Remove-Item -Path $portablePath -Recurse -Force
        }
    }

    if (-not $DryRun) {
        New-Item -ItemType Directory -Path $portablePath -Force | Out-Null
    }

    # 8a. Copiar binário
    $exePath = Join-Path $cmakeBuildDir "media-downloader.exe"
    if (Test-Path $exePath) {
        if ($DryRun) {
            Write-Host "[DRY]   Copy-Item '$exePath' -> '$portablePath'" -ForegroundColor Yellow
        } else {
            Copy-Item -Path $exePath -Destination $portablePath
        }
        Write-Ok "Binário copiado"
    } else {
        Write-Fail "media-downloader.exe não encontrado em $cmakeBuildDir"
    }

    # 8b. Copiar traduções
    $translationsSrc = Join-Path $SrcDir "translations"
    if (Test-Path $translationsSrc) {
        if ($DryRun) {
            Write-Host "[DRY]   Copy-Item '$translationsSrc' -> '$portablePath\translations'" -ForegroundColor Yellow
        } else {
            Copy-Item -Path $translationsSrc -Destination $portablePath -Recurse
        }
        Write-Ok "Traduções copiadas"
    }

    # 8c. windeployqt (coleta DLLs do Qt)
    $windeployqt = Join-Path $qtInfo.Path "bin\windeployqt.exe"
    if (Test-Path $windeployqt) {
        $deployArgs = @(
            (Join-Path $portablePath "media-downloader.exe")
            "--libdir", $portablePath
            "--plugindir", $portablePath
        )
        $result = Invoke-Ext -FilePath $windeployqt -Arguments $deployArgs
        if ($result.ExitCode -eq 0) {
            Write-Ok "windeployqt concluído"
        } else {
            Write-Warn "windeployqt retornou exit $($result.ExitCode)"
        }
    } else {
        Write-Warn "windeployqt não encontrado — DLLs do Qt não foram coletadas"
    }

    # 8d. OpenSSL DLLs (Qt 5.15 precisa de OpenSSL 1.1.x para TLS)
    $sslDlls = @("libssl-1_1-x64.dll", "libcrypto-1_1-x64.dll")
    $needSsl = $false
    foreach ($dll in $sslDlls) {
        if (-not (Test-Path (Join-Path $portablePath $dll))) {
            $needSsl = $true
            break
        }
    }
    if ($needSsl) {
        Write-Info "Procurando OpenSSL 1.1.x DLLs..."
        $sslSources = @(
            "C:\Program Files\OpenSSL-Win64"
            "C:\Program Files\OpenSSL"
            "C:\Program Files (x86)\OpenSSL-Win64"
            "C:\Program Files (x86)\OpenSSL"
            "$SrcDir\openssl"
        )
        $sslCopied = $false
        foreach ($srcDir in $sslSources) {
            if (-not (Test-Path $srcDir)) { continue }
            $allFound = $true
            foreach ($dll in $sslDlls) {
                if (-not (Test-Path (Join-Path $srcDir $dll))) {
                    $allFound = $false
                    break
                }
            }
            if ($allFound) {
                foreach ($dll in $sslDlls) {
                    $srcFile = Join-Path $srcDir $dll
                    Copy-Item -Path $srcFile -Destination $portablePath
                    Write-Ok "OpenSSL: $dll (de $srcDir)"
                }
                $sslCopied = $true
                break
            }
        }
        if (-not $sslCopied) {
            Write-Warn "OpenSSL 1.1.x DLLs não encontradas — TLS pode não funcionar"
            Write-Warn "Copie manualmente libssl-1_1-x64.dll e libcrypto-1_1-x64.dll para o diretorio de instalação"
        }
    } else {
        Write-Ok "OpenSSL DLLs já presentes"
    }

    # 8e. bsdtar (necessário para extrair zips: auto-update, deno, etc.)
    $bsdtarDst = Join-Path $portablePath "bsdtar.exe"
    if (-not (Test-Path $bsdtarDst)) {
        $bsdtarSources = @(
            "C:\Windows\System32\tar.exe"
            "C:\Program Files\Git\usr\bin\bsdtar.exe"
            "C:\Program Files (x86)\Git\usr\bin\bsdtar.exe"
        )
        $bsdtarCopied = $false
        foreach ($src in $bsdtarSources) {
            if (Test-Path $src) {
                Copy-Item -Path $src -Destination $bsdtarDst
                Write-Ok "bsdtar: copiado de $src"
                $bsdtarCopied = $true
                break
            }
        }
        if (-not $bsdtarCopied) {
            Write-Warn "bsdtar não encontrado — auto-update pode não funcionar"
        }
    } else {
        Write-Ok "bsdtar já presente"
    }

    # 8e-2. Launcher batch (adiciona dir ao PATH para bsdtar/yt-dlp)
    $launcherBat = Join-Path $portablePath "media-downloader.bat"
    $launcherLines = @(
        "@echo off"
        "set PATH=%~dp0;%PATH%"
        "start `"`" `%~dp0media-downloader.exe`""
    )
    if (-not $DryRun) {
        $launcherLines | Out-File -FilePath $launcherBat -Encoding ASCII
        Write-Ok "Launcher: media-downloader.bat"
    }

    # 8f. Zip (7-Zip)
    if ($sevenZipPath) {
        $zipName = "media-downloader-$VERSION-win-$Arch.zip"
        $zipPath = Join-Path $DistDir $zipName

        if (Test-Path $zipPath) {
            if ($DryRun) {
                Write-Host "[DRY]   Remove-Item '$zipPath'" -ForegroundColor Yellow
            } else {
                Remove-Item -Path $zipPath -Force
            }
        }

        $szExe = Join-Path $sevenZipPath "7z.exe"
        $result = Invoke-Ext -FilePath $szExe -Arguments @("a", $zipPath, "$portablePath\*")
        if ($result.ExitCode -eq 0) {
            Write-Ok "Zip: $zipPath"
        } else {
            Write-Fail "7-Zip falhou"
        }
    }

    # 8g. Inno Setup (instalador)
    if ($innoPath) {
        $issFile = Join-Path $cmakeBuildDir "media-downloader_windows_installer_Qt$($qtInfo.Version.Split('.')[0]).iss"
        if (Test-Path $issFile) {
            # Criar pasta 3rdParty com placeholder (dependências opcionais)
            $thirdPartyDir = Join-Path $portablePath "3rdParty"
            if (-not (Test-Path $thirdPartyDir)) {
                New-Item -ItemType Directory -Path $thirdPartyDir -Force | Out-Null
            }
            $placeholder = Join-Path $thirdPartyDir ".placeholder"
            if (-not (Test-Path $placeholder)) {
                Set-Content -Path $placeholder -Value "# Optional third-party tools (yt-dlp, gallery-dl, wget, etc.)" -Encoding UTF8
            }
            $isccExe = Join-Path $innoPath "iscc.exe"
            $result = Invoke-Ext -FilePath $isccExe -Arguments @("/O`"$DistDir`"", $issFile)
            if ($result.ExitCode -eq 0) {
                Write-Ok "Instalador Inno Setup gerado"
            } else {
                Write-Warn "Inno Setup retornou exit $($result.ExitCode)"
            }
        } else {
            Write-Warn "Arquivo .iss não encontrado: $issFile"
        }
    }

    # 8f. Informações de versão
    if (-not $DryRun) {
        $versionTxt = Join-Path $portablePath "version_info.txt"
        Set-Content -Path $versionTxt -Value $VERSION -Encoding UTF8

        $localDir = Join-Path $portablePath "local"
        if (-not (Test-Path $localDir)) {
            New-Item -ItemType Directory -Path $localDir -Force | Out-Null
        }
    }
}

# ── 9. Executar ──────────────────────────────────────────────────────────────
if ($Run) {
    Write-Title "Executando media-downloader"
    $exePath = Join-Path $cmakeBuildDir "media-downloader.exe"
    if (Test-Path $exePath) {
        if ($DryRun) {
            Write-Host "[DRY]   Start-Process '$exePath'" -ForegroundColor Yellow
        } else {
            Start-Process -FilePath $exePath
            Write-Ok "Aplicação iniciada"
        }
    } else {
        Write-Fail "Executável não encontrado: $exePath"
    }
}

# ── 10. Sucesso ──────────────────────────────────────────────────────────────
Write-Title "BUILD CONCLUÍDO"
Write-Ok "Binário: $cmakeBuildDir\media-downloader.exe"
if ($Package) {
    Write-Ok "Pacotes em: $DistDir\"
}
Write-Ok "Log: $LogFile"
if ($DryRun) {
    Write-Warn "Dry-run: reexecute sem -DryRun para aplicar."
}

Write-Log "=== FIM DO BUILD ==="
