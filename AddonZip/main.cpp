#include <vcl.h>
#include <windows.h>

#pragma hdrstop
#pragma argsused

#include <tchar.h>

#include <stdio.h>

#include <iostream>
#include <memory>

#include <System.Zip.hpp>
#include <IOUtils.hpp>

#include "MCommandLine.hpp"

static const UnicodeString g_s_LITERAL_AUTO_GENERATE = L"AUTO_GENERATE";
static const UnicodeString g_s_LITERAL_ADDON_ZIP_ROOT = L"_ADDON_ZIP_ROOT";

UnicodeString g_s_LITERAL_COPY_PARAMS = L"/MIR /XD .git .svn __pycache* /XF *.log *.md /R:3 /W:3";

UnicodeString GetAddonVersion(const UnicodeString &sAddonFolder) {
	try {
		const UnicodeString sAddonInitPy = TPath::Combine(sAddonFolder, L"__init__.py");
		if (FileExists(sAddonInitPy)) {

			const UnicodeString s_VERSION_PREFIX = L"\"version\":";

			std::unique_ptr<TStringList>AFileListPtr(new TStringList);
			AFileListPtr->LoadFromFile(sAddonInitPy, TEncoding::UTF8);
			bool bBlInfoFound = false;
			for (int i = 0; i < AFileListPtr->Count; i++) {
				const UnicodeString sCurrentLine = AFileListPtr->Strings[i].Trim();
				if (sCurrentLine.Pos("bl_info") == 1) {
					bBlInfoFound = true;
				}
				else if (bBlInfoFound && sCurrentLine.Pos(s_VERSION_PREFIX) == 1) {
					try {
						const int iStart = AFileListPtr->Strings[i].Pos("(");
						const int iEnd = AFileListPtr->Strings[i].Pos(")");
						if (!iStart || !iEnd)
							throw Exception("Wrong version format! Requires: (0, 0, 0)");

						UnicodeString sVersion = AFileListPtr->Strings[i].SubString(iStart + 1, iEnd - iStart - 1).Trim();
						sVersion = StringReplace(sVersion, L" ", L"", TReplaceFlags() << rfReplaceAll);
						sVersion = StringReplace(sVersion, L",", L"_", TReplaceFlags() << rfReplaceAll);

						return sVersion;
					}
					catch (Exception * E) {
						throw Exception(E->Message + " Full version line:" + sCurrentLine);
					}

					break;
				}
			}
		}
		else
			throw Exception("File <" + sAddonInitPy + "> is not found!");
	}
	catch (Exception *E) {
		std::wcerr << L"WARNING> " << E->Message.c_str() << std::endl;
	}

	return L"";
}

void ShowHelp(void) {
	std::wcout << L"Blender Addon Zip by Alex Zhornyak:" << std::endl;
	std::wcout << L"------------------------------" << std::endl;
	std::wcout << L"'addonDir' - path to addon directory" << std::endl;
	std::wcout << L"'addonName' - addon name. Default: name of 'addonDir'" << std::endl;
	std::wcout << L"'tempDir' - temporary directory for creating zip content. Default: User Temp Dir" << std::endl;
	std::wcout << L"'targetZipDir' - path to output zip dir" << std::endl;
	std::wcout << L"'targetZipName' - addon zip literal name or variable: " << g_s_LITERAL_AUTO_GENERATE.c_str()
		<< L". Default: " << g_s_LITERAL_AUTO_GENERATE.c_str() << std::endl;
	std::wcout << L"'copyParams' - params for using in robocopy. Default: " << g_s_LITERAL_COPY_PARAMS.c_str() << std::endl;
	std::wcout << L"------------------------------" << std::endl;
}

int _tmain(int argc, _TCHAR* argv[]) {
	try {
		UnicodeString sAddonDir = L"";
		UnicodeString sAddonName = L"";
		UnicodeString sTempDir = TPath::GetTempPath();
		UnicodeString sTargetZipDir = L"";
		UnicodeString sTargetZipName = L"";
		UnicodeString sOnBeforeZipCMD = L"";

		std::unique_ptr<TStringList>ACmdListPtr(new TStringList());
		MUtils::ParseCommandLine(ACmdListPtr.get());
		for (int i = 0; i < ACmdListPtr->Count; i++) {
			const UnicodeString sName = ACmdListPtr->Names[i];
			if (SameText(sName, L"addonDir")) {
				sAddonDir = ExcludeTrailingPathDelimiter(ACmdListPtr->ValueFromIndex[i]);
			}
			else if (SameText(sName, L"addonName")) {
				sAddonName = ACmdListPtr->ValueFromIndex[i];
			}
			else if (SameText(sName, L"tempDir")) {
				sTempDir = ACmdListPtr->ValueFromIndex[i];
			}
			else if (SameText(sName, L"targetZipDir")) {
				sTargetZipDir = ACmdListPtr->ValueFromIndex[i];
			}
			else if (SameText(sName, L"targetZipName")) {
				sTargetZipName = ACmdListPtr->ValueFromIndex[i];
			}
			else if (SameText(sName, L"copyParams")) {
				g_s_LITERAL_COPY_PARAMS = ACmdListPtr->ValueFromIndex[i];
			}
			else if (SameText(sName, L"onBeforeZipCMD")) {
				sOnBeforeZipCMD = ACmdListPtr->ValueFromIndex[i];
			}
			else if (sName == L"?" || sName == L"h" || sName == "help") {
				ShowHelp();
				return 0;
			}
		}

		if (!DirectoryExists(sAddonDir))
			throw Exception("Addon dir <" + sAddonDir + "> is not found!");

		if (!DirectoryExists(sTempDir))
			throw Exception("Temporary dir <" + sTempDir + "> is not found!");

		if (sTargetZipDir.IsEmpty())
			throw Exception("Target Zip dir is not defined!");

		if (!DirectoryExists(sTargetZipDir)) {
			ForceDirectories(sTargetZipDir);
		}

		if (!DirectoryExists(sTargetZipDir)) {
			throw Exception("Target Zip dir <" + sTargetZipDir + "> is not found or can not be created!");
		}

		if (sAddonName.IsEmpty()) {
			sAddonName = ExtractFileName(sAddonDir);
		}

		if (sTargetZipName.IsEmpty() || SameText(sTargetZipName, g_s_LITERAL_AUTO_GENERATE)) {
			sTargetZipName = sAddonName + L"_" + GetAddonVersion(sAddonDir) + L".zip";
		}

		const UnicodeString sTempZipRoot = TPath::Combine(sTempDir, g_s_LITERAL_ADDON_ZIP_ROOT);
		const UnicodeString sTempTarget = TPath::Combine(sTempZipRoot, sAddonName);

		try {
			_wsystem(UnicodeString().sprintf(L"robocopy \"%s\" \"%s\" %s", //
				sAddonDir.c_str(), sTempTarget.c_str(), g_s_LITERAL_COPY_PARAMS.c_str() //
				).c_str() //
				);

			if (!sOnBeforeZipCMD.IsEmpty()) {
				_wsystem(UnicodeString().sprintf(L"call \"%s\" \"%s\"", //
					sOnBeforeZipCMD.c_str(), sTempTarget.c_str() //
					).c_str() //
					);
			}

			const UnicodeString sTargetZipFile = TPath::Combine(sTargetZipDir, sTargetZipName);

			TZipFile::ZipDirectoryContents(sTargetZipFile, sTempZipRoot);
			std::wcout << L"Success: " << sTargetZipFile.c_str() << std::endl;
		}
		__finally {
			TDirectory::Delete(sTempZipRoot, true);
		}
	}
	catch (Exception *E) {
		ShowHelp();
		std::wcerr << L"ERROR> " << E->Message.c_str() << std::endl;

		return -1;
	}
	return 0;
}
