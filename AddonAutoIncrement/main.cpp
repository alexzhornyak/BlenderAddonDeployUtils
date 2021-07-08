//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <tchar.h>
//---------------------------------------------------------------------------

#include <memory>
#include <iostream>
#include <StrUtils.hpp>
#include <MCommandLine.hpp>

#pragma argsused

void ShowHelp(void) {
	std::wcout << L"Blender Addon AutoIncrementer by Alex Zhornyak:" << std::endl;
	std::wcout << L"------------------------------" << std::endl;
	std::wcout << L"'path' - path to '__init__.py'" << std::endl;
	std::wcout << L"'index' - version (0,0,0) index to increment: 0 or 1 or 2 etc.  Default: last" << std::endl;
	std::wcout << L"'encoding' - file encoding (UTF-8, SYSTEM). Default: UTF-8" << std::endl;
}

int _tmain(int argc, _TCHAR* argv[]) {
	try {

		UnicodeString sPath = L"";
		int iIndex = -1;
		TEncoding *AEncoding = TEncoding::UTF8;

		std::unique_ptr<TStringList>ACmdListPtr(new TStringList());
		MUtils::ParseCommandLine(ACmdListPtr.get());
		for (int i = 0; i < ACmdListPtr->Count; i++) {
			const UnicodeString sName = ACmdListPtr->Names[i].LowerCase();
			if (sName == L"path") {
				sPath = ACmdListPtr->ValueFromIndex[i];
			}
			else if (sName == L"index") {
				iIndex = ACmdListPtr->ValueFromIndex[i].ToInt();
			}
			else if (sName == L"encoding") {
				if (SameText(ACmdListPtr->ValueFromIndex[i], L"SYSTEM")) {
					AEncoding = TEncoding::Default;
				}
			}
			else if (sName == L"?" || sName == L"h" || sName == "help") {
				ShowHelp();
				return 0;
			}
		}

		if (FileExists(sPath)) {

			const UnicodeString s_VERSION_PREFIX = L"\"version\":";

			std::unique_ptr<TStringList>AFileListPtr(new TStringList);
			AFileListPtr->LoadFromFile(sPath, AEncoding);
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

						std::unique_ptr<TStringList>AVersionListPtr(new TStringList);
						AVersionListPtr->StrictDelimiter = true;
						AVersionListPtr->Delimiter = L',';
						AVersionListPtr->DelimitedText = AFileListPtr->Strings[i].SubString(iStart + 1, iEnd - iStart - 1);

						if (AVersionListPtr->Count == 0)
							throw Exception("Requires at least major version number!");

						int iVersion = 0;
						const int iLastIndex = AVersionListPtr->Count - 1;

						if (iIndex == -1) {
							iIndex = iLastIndex;
						}
						else {

							if (iIndex < 0 || iIndex >= AVersionListPtr->Count)
								throw Exception(UnicodeString().sprintf(L"Version index:[%d] is out of range:[1...%d]", //
									iIndex, AVersionListPtr->Count));
						}

						if (TryStrToInt(AVersionListPtr->Strings[iIndex], iVersion)) {

							int iWhiteSpace = 0;
							for (int k = 1; k <= AVersionListPtr->Strings[iIndex].Length(); k++) {
								if (AVersionListPtr->Strings[iIndex][k] == L' ') {
									iWhiteSpace++;
								}
								else
									break;
							}

							AVersionListPtr->Strings[iIndex] = DupeString(L" ", iWhiteSpace) + UnicodeString(++iVersion);
						}
						else
							throw Exception(UnicodeString().sprintf(L"Can not convert version:[%d] number:[%s] to integer!", //
								iIndex, AVersionListPtr->Strings[iIndex].c_str()));

						AFileListPtr->Strings[i] = AFileListPtr->Strings[i].SubString(1, iStart) + AVersionListPtr->CommaText + "),";
					}
					catch(Exception * E) {
						throw Exception(E->Message + " Full version line:" + sCurrentLine);
					}

					break;
				}
			}
			AFileListPtr->SaveToFile(sPath, AEncoding);
		}
		else
			throw Exception("File <" + sPath + "> is not found!");
	}
	catch(Exception * E) {
		ShowHelp();
		std::wcerr << L"ERROR>" << E->Message.c_str() << std::endl;

		return -1;
	}

	return 0;
}
//---------------------------------------------------------------------------
