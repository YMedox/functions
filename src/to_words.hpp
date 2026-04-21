#ifndef NUMBER_TO_WORDS_HPP
#define NUMBER_TO_WORDS_HPP

#include <string>
#include <unordered_map>
#include <vector>

enum class UnitType {
    PERCENT,
    DEGREE,
    VOLT,
    AMPERE,
    WATT,
    NONE
};

class NumberToWords {
public:
    static std::string convert(double number, UnitType unit = UnitType::NONE);

private:
    static std::string getOnes(int number, bool isFractional);
    static std::string getThousandsEnding(int ending);
    static std::string numberToWords(int number, bool isFractional);
    static std::string decimalPartToWords(int decimalPart);
    static std::string getUnitSuffix(UnitType unit, int baseNumber, bool isFractional);

    static const std::unordered_map<UnitType, std::string> unitNames;
    static const std::unordered_map<UnitType, std::vector<std::string>> unitSuffixes;
};

#endif
