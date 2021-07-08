//---------------------------------------------------------------------------

#ifndef MCommandLineH
#define MCommandLineH
//---------------------------------------------------------------------------

#include <utility>
#include <System.hpp>

namespace MUtils {

	enum TCMDParamStyle {
		cpsLong, cpsShort, cpsWindows
	};

	typedef std::pair<UnicodeString, UnicodeString>pair_str_t;

	void ParseCommandLine(TStringList *APairsList) {
		for (int i = 0; i <= ParamCount(); i++) {

			const UnicodeString sParam = ParamStr(i);

			pair_str_t ps_param_option;
			TCMDParamStyle ACMDParamStyle = TCMDParamStyle::cpsShort;

			const int iLong = sParam.Pos("--");
			const int iShort = sParam.Pos("-");
			const int iWnd = sParam.Pos("/");

			if (iLong == 1 || iShort == 1 || iWnd == 1) {

				if (iLong == 1) {
					ps_param_option.first = StringReplace(sParam, "--", "", TReplaceFlags());
					ACMDParamStyle = TCMDParamStyle::cpsLong;
				}
				else if (iShort == 1) {
					ps_param_option.first = StringReplace(sParam, "-", "", TReplaceFlags());
					ACMDParamStyle = TCMDParamStyle::cpsShort;
				}
				else if (iWnd == 1) {
					ps_param_option.first = StringReplace(sParam, "/", "", TReplaceFlags());
					ACMDParamStyle = TCMDParamStyle::cpsWindows;
				}
				else {
					ps_param_option.first = "";
				}

				if (i == ParamCount()) {
					ps_param_option.second = "";
				}
				else {
					const UnicodeString sSecondParam = ParamStr(i + 1);
					if (sSecondParam.Pos("--") == 1 || sSecondParam.Pos("-")
						== 1 || sSecondParam.Pos("/") == 1) {

						// если параметр вот такой: -TSDebugCfg "-p TSSound.exe"
						if (sSecondParam.LastDelimiter(" \t")) {
							ps_param_option.second = sSecondParam;
							i++;
						}
						else {
							// если вот так: -p -t
							ps_param_option.second = "";
						}
					}
					else {
						ps_param_option.second = sSecondParam;
						i++;
					}
				}

				APairsList->Add(ps_param_option.first + "=" + ps_param_option.second);
			}
			else if (i == 0) {
				// EXE NAME
			}
		}
	}

}

#endif
