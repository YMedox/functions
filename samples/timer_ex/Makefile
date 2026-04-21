# Компилятор и флаги
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pipe -march=native -MMD -MP -Wpmf-conversions
LDFLAGS  = 
# Директории
SRCDIR = ../../src
INCDIR = $(SRCDIR)
OBJDIR = obj
TARGET = timer_ex

# Исходные файлы
SOURCES = main.cpp $(SRCDIR)/mtimer.cpp
# Объектные файлы
OBJECTS = $(OBJDIR)/main.o $(OBJDIR)/mtimer.o
# Зависимости (автогенерация)
DEPS = $(OBJECTS:.o=.d)

# Цели по умолчанию
.PHONY: all clean run

all: $(TARGET)

# Линковка исполняемого файла
$(TARGET): $(OBJECTS) | $(OBJDIR)
	$(CXX) -o $@ $^ $(LDFLAGS)
	@echo "✅ Собрано: $(TARGET)"
	
# Компиляция объектных файлов
# $(@D) — это директория цели 
# $(@F) — это имя файла цели 
$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

# Создание директории obj
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Включение зависимостей
-include $(DEPS)

# Запуск
run: $(TARGET)
	./$(TARGET)

# Очистка
clean:
	rm -rf $(OBJDIR) $(TARGET)
	@echo "🧹 Очищено"

# Установка (опционально, в /usr/local/bin)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/
	@echo "📦 Установлено в /usr/local/bin/"
