# Компилятор и флаги
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pipe -march=native -MMD -MP -Wpmf-conversions -fexceptions
LDFLAGS  = -lpthread -lglib-2.0
# Директории
SRCDIR = ../../src
#INCDIR = -I$(SRCDIR)
INCDIR = -I$(SRCDIR) -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include
OBJDIR = obj
TARGET = 4in1_ex

# Исходные файлы
SOURCES = main.cpp $(SRCDIR)/mlogger.cpp $(SRCDIR)/mtimer.cpp $(SRCDIR)/iniobject.cpp $(SRCDIR)/to_words.cpp
# Объектные файлы
OBJECTS = $(OBJDIR)/main.o $(OBJDIR)/mlogger.o $(OBJDIR)/mtimer.o $(OBJDIR)/iniobject.o $(OBJDIR)/to_words.o
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
	rm -rf *.log
	@echo "🧹 Очищено"

# Установка (опционально, в /usr/local/bin)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/
	@echo "📦 Установлено в /usr/local/bin/"
