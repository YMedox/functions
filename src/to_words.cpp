/*---------------------------------------------------------------------------------------------------
Библиотека преобразования числовой величины в пропись текстом на русском языке.
Код примерно на 70% сгенерирован Алисой. Исправлены ошибки. Увеличен диапазон счета.
Округляет дробную часть до десятых.
Считает только от -999999.9 до 999999.9.
Основной вызов NumberToWords::convert(value, UnitType::ххх)

Автор - Ярослав Медокс
---------------------------------------------------------------------------------------------------*/
#include "to_words.hpp"
#include <cmath>
#include <sstream>

const std::unordered_map<UnitType, std::vector<std::string>> NumberToWords::unitSuffixes = {
    {UnitType::PERCENT, {"процент", "процента", "процентов"}},
    {UnitType::DEGREE,  {"градус", "градуса", "градусов"}},
    {UnitType::VOLT,    {"вольт", "вольта", "вольт"}},
    {UnitType::AMPERE,  {"ампер", "ампера", "ампер"}},
    {UnitType::WATT,    {"ватт", "ватта", "ватт"}},
    {UnitType::NONE,    {"", "", ""}}
};

std::string NumberToWords::convert(double number, UnitType unit) {
    if (number >= 1000000) return "БОЛЬШЕ МАКСИМУМА";

    bool isNegative = number < 0;
    if (isNegative) {
        number = -number;
    }

    int integerPart = static_cast<int>(number);
    double fractionalPartDouble = number - integerPart;
    int fractionalPart = static_cast<int>(std::round(fractionalPartDouble * 10)); // одна цифра после запятой
    bool isFractional = fractionalPart > 0;

    std::string result;

    if (integerPart == 0 && fractionalPart == 0) {
        result = "ноль";
    } else {
        if (integerPart > 0) {
            result = numberToWords(integerPart, isFractional);
        }
        if (isFractional) {
            if (!result.empty()) {
                result += " цел";
                if(integerPart%100==11) { result += "ых "; }
                else                    { result += integerPart%10==1&&integerPart!=11?"ая ":"ых "; }
            }
            result += (decimalPartToWords(fractionalPart) + (fractionalPart%10==1?" десятая":" десятых"));
        }
    }

    if (isNegative) {
        result = "минус " + result;
    }

    if (unit != UnitType::NONE) {
        int baseNumber = isFractional ? fractionalPart : integerPart;
        std::string unitSuffix = getUnitSuffix(unit, baseNumber, isFractional);
        result += " " + unitSuffix;
    }

    return result;
}

std::string NumberToWords::getOnes(int number, bool isFractional) {
    static const char* ones[] = {
        "три", "четырe", "пять",
        "шесть", "семь", "восемь", "девять", "десять",
        "одиннадцать", "двенадцать", "тринадцать", "четырнадцать", "пятнадцать",
        "шестнадцать", "семнадцать", "восемнадцать", "девятнадцать"
    };
    if(number < 3) {
      static const char* ones_fra[] = {
          "", "однa", "двe"
      };
      static const char* ones_nfra[] = {
          "", "один", "два"
      };
      return std::string(isFractional?ones_fra[number]:ones_nfra[number]);
    }
    return std::string(ones[number-3]);
}

std::string NumberToWords::getThousandsEnding(int ending) {
  std::string start = " тысяч";
  switch(ending%10) { //нужны единицы от числа тысяч
     case 1:
       return start + "a";
     case 2:
     case 3:
     case 4:
       return start + "и";
     default:
       return start;
  }
}

std::string NumberToWords::numberToWords(int number, bool isFractional) {
    if (number == 0)       return "ноль";
    static const char* hundreds[] = {
        "", "сто", "двести", "триста", "четыреста",
        "пятьсот", "шестьсот", "семьсот", "восемьсот", "девятьсот"
    };
    if (number > 1000) {
       std::string ret = numberToWords(number / 1000, true);
       ret += getThousandsEnding(number / 1000) +
          (number % 1000 != 0 ? " " + numberToWords(number % 1000, isFractional) : "");
       return ret;
    }
    if (number < 20) {
        return getOnes(number, isFractional);
    }
    if (number < 100) {
        static const char* tens[] = {
            "", "", "двадцать", "тридцать", "сорок",
            "пятьдесят", "шестьдесят", "семьдесят", "восемьдесят", "девяносто"
        };
        return std::string(tens[number / 10]) +
               (number % 10 != 0 ? " " + getOnes(number % 10, isFractional) : "");
    }
    return std::string(hundreds[number / 100]) +
           (number % 100 != 0 ? " " + numberToWords(number % 100, isFractional) : "");
}

std::string NumberToWords::decimalPartToWords(int decimalPart) {
    return numberToWords(decimalPart, true);
}

std::string NumberToWords::getUnitSuffix(UnitType unit, int number, bool isFractional) {
    const auto& suffixes = unitSuffixes.at(unit);

    if (isFractional) {
        return suffixes[1];
    }

    // Для целых чисел — стандартная логика
    int lastDigit = number % 10;
    int lastTwoDigits = number % 100;

    if (lastTwoDigits >= 11 && lastTwoDigits <= 14) {
        return suffixes[2];
    }
    if (lastDigit == 1) {
        return suffixes[0];
    }
    if (lastDigit >= 2 && lastDigit <= 4) {
        return suffixes[1];
    }
    return suffixes[2];
}

