#include <iostream>
#include <map>
#include <cmath>
#include "to_words.hpp"

std::map<int, UnitType> types = {{0, UnitType::PERCENT}, {1, UnitType::DEGREE}, {2, UnitType::VOLT}, {3, UnitType::AMPERE}, {4, UnitType::WATT}, {5, UnitType::NONE} };

//генератор случайных чисел с ограничением диапазона [_min, _max], включая границы
unsigned long bounded_rand(unsigned long _min, unsigned long _max) {
    unsigned long _ref = _max + 2 -_min;
    for (unsigned long x, r;;)
        if ((x = rand()) && (r = x % _ref))
            return r+_min-1;
}

//вызывает перевод в текст для случайных числе из переданного диапазона и случайных величин
void oneCycle(unsigned range) {
    for(int i=0;i<50;i++) {
      double x = double(bounded_rand(0, range)) / (1 + bounded_rand(0, 4));
      x = std::round(x * 10) / 10; //оставляем только десятые, библиотека их все равно бы округлила
      printf("Число: %.1f\n", x);  //для std::cout для больших чисел нужно форматирование
      std::cout << NumberToWords::convert(x, types[bounded_rand(0, 5)]) << std::endl;
    }
    std::cout<<"Конец цикла\n"<<std::endl;
}


int main()
{
    srand(time(NULL));  //рандомизируем начало
    oneCycle(1000000);  //сначала большие числа посмотрим
    oneCycle(200);      //теперь маленькие, порядок не важен
    std::cout<<"Ручные примеры:"<<std::endl;
    std::cout << NumberToWords::convert(999999.9, UnitType::NONE) << std::endl;
    std::cout << NumberToWords::convert(-999999.9, UnitType::NONE) << std::endl;
    std::cout << NumberToWords::convert(2000000, UnitType::NONE) << std::endl;
    std::cout << NumberToWords::convert(1027.2, UnitType::NONE) << std::endl;
    std::cout << NumberToWords::convert(133000, UnitType::NONE) << std::endl;
    std::cout << NumberToWords::convert(3000.3, UnitType::NONE) << std::endl;
    std::cout << NumberToWords::convert(90842.0, UnitType::NONE) << std::endl;
    return 0;
}

