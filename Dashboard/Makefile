CXX = D:/bin/mingw64_1420/bin/g++
CXXFLAGS = -ID:/bin/SFML-3.0.2-windows-gcc-14.2.0-mingw-64-bit/SFML-3.0.2/include
LDFLAGS  = -LD:/bin/SFML-3.0.2-windows-gcc-14.2.0-mingw-64-bit/SFML-3.0.2/lib \
           -lsfml-graphics -lsfml-window -lsfml-system

SRC = $(FILE)
TARGET = $(basename $(FILE)).exe

all:
	$(CXX) $(SRC) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)

clean:
	del *.exe
