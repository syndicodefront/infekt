#### The rendered mode is slow.
Please try to disable "_Hardware (GPU) acceleration_" in the preferences' Advanced section.

#### How do I open the menu?
Simply press the Alt key on your keyboard. You can also permanently show it using the menu button from iNFekt's toolbar.

#### I get an error message "_Side-by-side configuration is incorrect_" or "_MSVCR110.DLL is missing_" when I try to start the application.

You need to download and install the <a href="http://www.microsoft.com/en-us/download/details.aspx?id=30679">Visual C++ Redistributable for Visual Studio 2012 Update 4</a> from Microsoft.
Alternatively, you can get the iNFekt setup file which will download and install the required files automatically. The DLLs are also shipped with the portable versions.

Earlier Versions: up to and including version 0.8.5 the Visual C++ *2010* runtime was required:
  * <a href="http://www.microsoft.com/download/en/details.aspx?id=8328">Microsoft Visual C++ 2010 SP1 Redistributable Package (x86)</a>
  * <a href="http://www.microsoft.com/download/en/details.aspx?id=13523">Microsoft Visual C++ 2010 SP1 Redistributable Package (x64)</a>

#### Where can I get a Windows 2000 compatible version?
You can use <a href="https://syndicode.org/infekt/downloads/v0.7.9/">iNFekt 0.7.9</a> (32-bit).

#### Where can I get a Windows XP 64-bit compatible version?
You can use <a href="https://syndicode.org/infekt/downloads/v0.8.5/">iNFekt 0.8.5</a> or any 32-bit version.

#### Using the auto update says "_The program can't start because MSVCP110.dll is missing from your computer._" on Windows 7 or 8 64-bit

Please download and install `vcredist_x86.exe` (not x64) from: <a href="http://www.microsoft.com/en-us/download/details.aspx?id=30679">here</a>.
