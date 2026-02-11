LibName := libICA_USV_Test.so

GCC = g++
BuildDIR := ./build

src_files := $(shell find ./Source -name "*.cpp")
header_files := $(shell find -name "*.h")
obj_files := $(src_files:%.cpp=$(BuildDIR)/%.o)

# Add -fPIC to position-independent code for shared lib
CXXFLAGS := -fPIC -I include
LDFLAGS := -shared -lstdc++ -lpthread -lqrencode -lgd

# Build shared library (.so)
$(LibName): $(obj_files)
	$(GCC) $(LDFLAGS) $(obj_files) -o $@

# Compile each .cpp to .o
$(BuildDIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(GCC) -c $< -o $@ $(CXXFLAGS)

# Clean build artifacts
clean:
	rm -rf $(BuildDIR)
	rm -f $(LibName)

#OutFile := ICA_USV_Test
#CFLAGS := -lstdc++ -lpthread -lqrencode -lgd
#$(OutFile) : $(obj_files)
#	$(GCC) $(obj_files) -o $@ $(CFLAGS)
#%.cpp : %.h
#	$(shell	touch $@)
