set(CMAKE_CXX_FLAGS -std=c++11)
set(LLVM_LINK_COMPONENTS asmparser bitreader support mc option)
set(LLVM_USED_LIBS 
clangFrontend 
clangSerialization 
clangDriver 
clangTooling 
clangParse 
clangSema 
clangAnalysis 
clangRewriteFrontend 
clangRewriteCore 
clangEdit 
clangAST 
clangLex 
clangIndex
clangStaticAnalyzerFrontend
clangBasic)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -I/usr/include/mysql -lmysqlclient")

add_clang_executable(cblock
  CBlocks.cpp
	QueryBlock.cpp
  )

target_link_libraries(cblock
  clangTooling
  clangBasic
  clangASTMatchers
  clangStaticAnalyzerFrontend
  clangFrontend
  clangAST
  clangDriver
  clangAnalysis
  )
