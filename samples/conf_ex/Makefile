# Компилятор и флаги
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pipe -march=native -MMD -MP -fexceptions
LDFLAGS  = -lglib-2.0
# Директории
SRCDIR = ../../src
INCDIR = -I$(SRCDIR) -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include
OBJDIR = obj
TARGET = conf_ex

# Исходные файлы
SOURCES = main.cpp $(SRCDIR)/iniobject.cpp
# Объектные файлы
OBJECTS = $(OBJDIR)/main.o $(OBJDIR)/iniobject.o
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
	$(CXX) $(CXXFLAGS) $(INCDIR) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCDIR) -c $< -o $@

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
	rm -rf $(TARGET).conf
	@echo "🧹 Очищено"

# Установка (опционально, в /usr/local/bin)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/
	@echo "📦 Установлено в /usr/local/bin/"
