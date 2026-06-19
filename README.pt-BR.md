# Media Downloader

English version: [README.md](README.md)

Este projeto é uma interface gráfica em Qt/C++ para ferramentas de linha de comando usadas para baixar mídia online.

O utilitário CLI [yt-dlp](https://github.com/yt-dlp/yt-dlp) é a ferramenta padrão com suporte principal. Outras ferramentas podem ser adicionadas por meio de extensões, e a lista de extensões suportadas é mantida [aqui](https://github.com/wbaamaral/media-downloader/wiki/Extensions).

## Funcionalidades

1. A interface pode ser usada para baixar mídia de qualquer site suportado pelas extensões instaladas.
2. A interface oferece uma lista configurável de opções predefinidas para downloads em diferentes formatos.
3. A interface permite múltiplos downloads concorrentes. Use com cuidado, porque muitos downloads simultâneos podem causar bloqueio pelo site de origem.
4. A interface permite downloads em lote, seja informando links individualmente ou lendo de um arquivo local.
5. A interface permite baixar playlists de sites que oferecem esse recurso, como o YouTube.
6. A interface permite gerenciar links de playlists para acompanhar atividades com mais facilidade.
7. A interface está disponível em vários idiomas.

## Extensões

O Media Downloader é uma interface para [yt-dlp](https://github.com/yt-dlp/yt-dlp), [gallery-dl](https://github.com/mikf/gallery-dl), [you-get](https://github.com/soimort/you-get), [svtplay-dl](https://github.com/spaam/svtplay-dl), [aria2c](https://aria2.github.io/), [wget](https://www.gnu.org/software/wget) e [get-sauce](https://github.com/gan-of-culture/get-sauce).

Para instalar extensões, vá até a aba `Configure`, depois à subaba `Extensions`, clique em `Add An Extension` e escolha a extensão desejada.

## FAQ

A página de perguntas frequentes está [aqui](https://github.com/wbaamaral/media-downloader/wiki/Frequently-Asked-Questions).

## Antes de executar pela primeira vez

Verifique se há acesso à internet na primeira execução do Media Downloader, porque o aplicativo tenta baixar a versão mais recente do `yt-dlp`.

Dependendo das extensões instaladas, o aplicativo também pode acessar a internet para baixar executáveis adicionais.

## Estado atual

O projeto está alinhado com a pre-release `v5.6.1-rc.2` e já inclui:

- melhorias no fluxo de inicialização;
- ajuste de layout e responsividade da interface;
- centralização da janela inicial no monitor ativo;
- persistência do estado da janela entre execuções;
- refinamento da aba `Configure`;
- correções no fluxo de descoberta e uso dos mecanismos de execução.

## Pacotes binários

### Bundle para macOS

Bundle para `arm64` do macOS: [aqui](https://github.com/wbaamaral/media-downloader/releases/download/v5.6.1-rc.2/MediaDownloaderQt6-arm64-v5.6.1-rc.2.dmg).

Bundle para `x86_64` do macOS: [aqui](https://github.com/wbaamaral/media-downloader/releases/download/v5.6.1-rc.2/MediaDownloaderQt6-x86_64-v5.6.1-rc.2.dmg).

Esses bundles não são notarizados e o sistema pode reportá-los como corrompidos. Consulte a documentação do macOS sobre como instalar aplicativos não notarizados, caso queira usar este app nessa plataforma. Este bundle funciona no macOS 14.0 ou superior.

### Instalador para Microsoft Windows

Instalador para Microsoft Windows 32 bits, construído com Qt5 e com requisito mínimo de Windows 7: [aqui](https://github.com/wbaamaral/media-downloader/releases/download/v5.6.1-rc.2/MediaDownloaderQt5-v5.6.1-rc.2.setup.exe).

Instalador para Microsoft Windows 64 bits, construído com Qt6 e com requisito mínimo de Windows 10: [aqui](https://github.com/wbaamaral/media-downloader/releases/download/v5.6.1-rc.2/MediaDownloaderQt6-v5.6.1-rc.2.setup.exe).

### Versão portátil para Microsoft Windows

A versão portátil é autocontida, guarda tudo dentro da pasta da aplicação e não precisa ser instalada antes.

Versão portátil para Microsoft Windows 32 bits, construída com Qt5 e com requisito mínimo de Windows 7: [aqui](https://github.com/wbaamaral/media-downloader/releases/download/v5.6.1-rc.2/MediaDownloaderQt5-v5.6.1-rc.2.zip).

Versão portátil para Microsoft Windows 64 bits, construída com Qt6 e com requisito mínimo de Windows 10: [aqui](https://github.com/wbaamaral/media-downloader/releases/download/v5.6.1-rc.2/MediaDownloaderQt6-v5.6.1-rc.2.zip).

Você também pode instalar a versão portátil no Windows via Scoop:

Adicionar o bucket extras:

```powershell
scoop bucket add extras
```

Instalar o Media Downloader:

```powershell
scoop install media-downloader
```

Versões `git` para Windows e macOS podem ser baixadas [aqui](https://github.com/wbaamaral/media-downloader-git/releases).

### Problemas com antivírus no Windows

De vez em quando, o Windows Defender e outros antivírus podem marcar o aplicativo como vírus/ameaça ou como software potencialmente indesejado. Esses relatórios são falsos positivos e são acompanhados [aqui](https://github.com/wbaamaral/media-downloader/issues/481).

### Flatpak

O Media Downloader está no [Flathub](https://flathub.org/apps/io.github.wbaamaral.media-downloader) para quem prefere usar Flatpak.

### Pacote AUR para Arch Linux

Usuários do Arch Linux podem compilar o projeto a partir do pacote AUR [aqui](https://aur.archlinux.org/packages/media-downloader).

### Pacote para Fedora

O Media Downloader está nos repositórios oficiais do Fedora e pode ser instalado com:

```bash
sudo dnf -y install media-downloader
```

### Pacotes binários para outras distribuições Linux

Pacotes binários mantidos para algumas distribuições Linux estão [aqui](https://software.opensuse.org//download.html?project=home%3Awbaamaral&package=media-downloader).

### Status dos empacotamentos

Uma lista resumida das distribuições que possuem o Media Downloader em seus repositórios, com a versão disponível, é mantida [aqui](https://repology.org/project/media-downloader/badges).

## Como compilar no Linux

1. Clone o repositório e entre nele:

```console
git clone https://github.com/wbaamaral/media-downloader && cd media-downloader
```

2. Torne o script auxiliar executável:

```console
chmod +x build_linux.sh
```

3. Execute o script:

```console
./build_linux.sh
```

Os scripts auxiliares seguem este fluxo:

1. configuram o build com CMake;
2. compilam com `cmake --build`;
3. executam os testes rápidos com `ctest`;
4. opcionalmente iniciam o aplicativo quando `RUN_AFTER_BUILD=1`.

Se quiser executar o processo manualmente:

```console
mkdir build
cd build
cmake .. -DBUILD_TESTING=ON
cmake --build . -j"$(nproc)"
ctest --output-on-failure
```

### Fedora

Usuários do Fedora podem usar o script abaixo para compilar a partir do código-fonte:

```console
./build_fedora.sh
```

### Arch Linux

Usuários do Arch Linux podem usar o script abaixo para compilar a partir do código-fonte:

```console
./build_arch.sh
```

## Capturas de tela

![media-downloader.png](https://raw.githubusercontent.com/wbaamaral/media-downloader/main/images/media-downloader-1.png)

![media-downloader.png](https://raw.githubusercontent.com/wbaamaral/media-downloader/main/images/media-downloader-2.png)

![media-downloader.png](https://raw.githubusercontent.com/wbaamaral/media-downloader/main/images/media-downloader-3.png)

![media-downloader.png](https://raw.githubusercontent.com/wbaamaral/media-downloader/main/images/media-downloader-4.png)

![media-downloader.png](https://raw.githubusercontent.com/wbaamaral/media-downloader/main/images/media-downloader-5.png)

![media-downloader.png](https://raw.githubusercontent.com/wbaamaral/media-downloader/main/images/media-downloader-6.png)

![media-downloader.png](https://raw.githubusercontent.com/wbaamaral/media-downloader/main/images/media-downloader-7.png)

## Aviso

Este programa foi criado para ser usado de forma que não viole quaisquer leis aplicáveis aos usuários.

## Licença

Este programa é software livre: você pode redistribuí-lo e/ou modificá-lo sob os termos da GNU General Public License, conforme publicada pela Free Software Foundation, seja a versão 2 da licença ou, a seu critério, qualquer versão posterior.

Este programa é distribuído na esperança de ser útil, mas SEM QUALQUER GARANTIA; sem mesmo a garantia implícita de COMERCIABILIDADE ou ADEQUAÇÃO A UM DETERMINADO FIM. Consulte a GNU General Public License para mais detalhes.
