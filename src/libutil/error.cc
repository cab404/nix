#include "error.hh"

#include <iostream>
#include <optional>

namespace nix
{

std::optional<string> ErrorInfo::programName = std::nullopt;

string showErrPos(const ErrPos &errPos)
{
    if (errPos.column > 0) {
        return (format("(%1%:%2%)") % errPos.lineNumber % errPos.column).str();
    } else {
        return (format("(%1%)") % errPos.lineNumber).str();
    };
}

void printCodeLines(const string &prefix, const ErrorInfo &einfo)
{
    if (einfo.errPos.has_value()) {
        // previous line of code.
        if (einfo.prevLineOfCode.has_value()) {
            std::cout << format("%1% %|2$5d|| %3%")
                      % prefix
                      % (einfo.errPos->lineNumber - 1)
                      % *einfo.prevLineOfCode
                      << std::endl;
        }

        // line of code containing the error.%2$+5d%
        std::cout << format("%1% %|2$5d|| %3%")
                  %    prefix
                  % (einfo.errPos->lineNumber)
                  % einfo.errLineOfCode
                  << std::endl;

        // error arrows for the column range.
        if (einfo.errPos->column > 0) {
            int start = einfo.errPos->column;
            std::string spaces;
            for (int i = 0; i < start; ++i) {
                spaces.append(" ");
            }

            std::string arrows("^");

            std::cout << format("%1%      |%2%" ANSI_RED "%3%" ANSI_NORMAL) % prefix % spaces % arrows << std::endl;
        }



        // next line of code.
        if (einfo.nextLineOfCode.has_value()) {
            std::cout << format("%1% %|2$5d|| %3%")
                      %    prefix
                      % (einfo.errPos->lineNumber + 1)
                      % *einfo.nextLineOfCode
                      << std::endl;
        }

    }

}

void printErrorInfo(const ErrorInfo &einfo)
{
    int errwidth = 80;
    string prefix = "    ";

    string levelString;
    switch (einfo.level) {
    case ErrLevel::elError: {
        levelString = ANSI_RED;
        levelString += "error:";
        levelString += ANSI_NORMAL;
        break;
    }
    case ErrLevel::elWarning: {
        levelString = ANSI_YELLOW;
        levelString += "warning:";
        levelString += ANSI_NORMAL;
        break;
    }
    default: {
        levelString = (format("invalid error level: %1%") % einfo.level).str();
        break;
    }
    }

    int ndl = prefix.length() + levelString.length() + 3 + einfo.name.length() + einfo.programName.value_or("").length();
    int dashwidth = ndl > (errwidth - 3) ? 3 : errwidth - ndl;

    string dashes;
    for (int i = 0; i < dashwidth; ++i)
        dashes.append("-");

    // divider.
    std::cout << format("%1%%2%" ANSI_BLUE " %3% %4% %5% %6%" ANSI_NORMAL)
              % prefix
              % levelString
              % "---"
              % einfo.name
              % dashes
              % einfo.programName.value_or("")
              << std::endl;

    // filename.
    if (einfo.errPos.has_value()) {
        if (einfo.errPos->nixFile != "") {
            string eline = einfo.errLineOfCode != ""
                           ? string(" ") + showErrPos(*einfo.errPos)
                           : "";

            std::cout << format("%1%in file: " ANSI_BLUE "%2%%3%" ANSI_NORMAL)
                      % prefix % einfo.errPos->nixFile % eline << std::endl;
            std::cout << prefix << std::endl;
        } else {
            std::cout << format("%1%from command line argument") % prefix << std::endl;
            std::cout << prefix << std::endl;
        }
    }

    // description
    std::cout << prefix << einfo.description << std::endl;
    std::cout << prefix << std::endl;

    // lines of code.
    if (einfo.errLineOfCode != "") {
        printCodeLines(prefix, einfo);
        std::cout << prefix << std::endl;
    }

    // hint
    if (einfo.hint.has_value()) {
        std::cout << prefix << *einfo.hint << std::endl;
        std::cout << prefix << std::endl;
    }
}

}
